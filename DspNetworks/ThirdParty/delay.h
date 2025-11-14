/*
    Copyright 2025 iamlamprey

    This file is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This file is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with This file. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <JuceHeader.h>

enum class GlobalCablesDelay
{
    tempo = 0,
    pitch = 1,
    nam = 2
};    

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;
using cable_manager_t = routing::global_cable_cpp_manager<SN_GLOBAL_CABLE(110245659),
                                                          SN_GLOBAL_CABLE(106677056),
                                                          SN_GLOBAL_CABLE(108826)>;
                                                    
template <int NV> struct delay: public data::base, public cable_manager_t
{
    SNEX_NODE(delay);        
    
    struct MetadataClass
    {
        SN_NODE_ID("delay");
    };
    
    static constexpr bool isModNode() { return false; };
    static constexpr bool isPolyphonic() { return NV > 1; };
    static constexpr bool hasTail() { return true; };
    static constexpr bool isSuspendedOnSilence() { return false; };
    static constexpr int getFixChannelAmount() { return 2; };
    static constexpr int NumTables = 0;
    static constexpr int NumSliderPacks = 0;
    static constexpr int NumAudioFiles = 0;
    static constexpr int NumFilters = 0;
    static constexpr int NumDisplayBuffers = 0;

    int handleModulation(double& value) { return 0; }
    template <typename T> void processFrame(T& data) {}
    void handleHiseEvent(HiseEvent& e) {}
    void setExternalData(const ExternalData& data, int index) {}

    delay()
    {        
        delayLines.resize(2);
        delayIndices.assign(2, 0);

        this->registerDataCallback<GlobalCablesDelay::tempo>([this](const var& data)
        {
            // set global tempo here
            currentBPM = data;
        });
    }
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate = static_cast<float>(specs.sampleRate);
        numChannels = static_cast<int>(specs.numChannels);
        numChannels = juce::jlimit(1, 2, numChannels); // capped to stereo

        const float maxDelayTime = 4.0f;
        const int maxDelayInSamples = static_cast<int>(maxDelayTime * sampleRate);
        
        delayLines.resize(numChannels);
        delayIndices.resize(numChannels);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            delayLines[ch].assign(maxDelayInSamples, 0.0f);
            delayIndices[ch] = 0;
        }
        
        lfoPhaseL = 0.0f;
        lfoPhaseR = juce::MathConstants<float>::pi * 0.5f;
                
        updateDampingFilter();
        updateInputFilter();
        
        const size_t fadeSamples = static_cast<size_t>(0.01f * sampleRate);
        for (auto& rb : reverseBlocks)
        {
            const size_t maxReverseSize = static_cast<size_t>(sampleRate * 4.0f);
            rb.dataA.assign(maxReverseSize, 0.0f);
            rb.dataB.assign(maxReverseSize, 0.0f);
            rb.playIsA = false; 
            rb.writePos = 0;
            rb.playPos = 0;
            rb.playing = false;
            rb.fadeSamples = fadeSamples;
            rb.fadingIn = false;
            rb.fadingOut = false;
            rb.fadePos = 0;
            rb.recFilled = false;
            rb.currentBlockSize = 0;
        }
        
        const size_t glitchBufferSize = static_cast<size_t>(sampleRate * 2.0f);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            glitchBuffer[ch].assign(glitchBufferSize, 0.0f);
            glitchWritePos[ch] = 0;
            
            for (auto& grain : glitchGrains[ch])
            {
                grain.data.assign(static_cast<size_t>(sampleRate * 0.5f), 0.0f);
                grain.totalFadeSamples = static_cast<size_t>(0.005f * sampleRate);
                grain.size = 0;
                grain.playPos = 0;
                grain.active = false;
            }
        }

        reset();
    }       

    void reset() 
    {
        const int chans = juce::jmin<int>(numChannels, static_cast<int>(delayLines.size()));
        for (int ch = 0; ch < chans; ++ch)
        {
            std::fill(delayLines[ch].begin(), delayLines[ch].end(), 0.0f);
            delayIndices[ch] = 0;
        }
        
        for (int ch = 0; ch < 2; ++ch)
        {
            dampingFilterState[ch].reset();
            inputFilterState[ch].reset();
        }

        lfoPhaseL = 0.0f;
        lfoPhaseR = juce::MathConstants<float>::pi * 0.5f;
        
        // reverse
        for (auto& rb : reverseBlocks) 
        {
            rb.writePos = 0;
            rb.playPos = 0;
            rb.playing = false;
            rb.fadingIn = false;
            rb.fadingOut = false;
            rb.fadePos = 0;
            rb.playIsA = false;
            rb.recFilled = false;
            rb.currentBlockSize = 0;
            std::fill(rb.dataA.begin(), rb.dataA.end(), 0.0f);
            std::fill(rb.dataB.begin(), rb.dataB.end(), 0.0f);
        }
        
        // glitch
        for (int ch = 0; ch < 2; ++ch)
        {
            glitchWritePos[ch] = 0;
            glitchStutterCounter[ch] = 0;
            glitchStutterLength[ch] = 0;
            lastReverseSample[ch] = 0.0f;
            
            for (auto& grain : glitchGrains[ch])
            {
                grain.active = false;
                grain.playPos = 0;
                grain.size = 0;
                std::fill(grain.data.begin(), grain.data.end(), 0.0f);
            }
        }

        glitchRandomPhase = 0.0f;
        glitchMode = 0;
    }
    
    template <typename T> void process(T& data)
    {
        const int numSamples = data.getNumSamples();
        const float mixValue = juce::jlimit(0.0f, 1.0f, mix);
        const float delayTimeValue = juce::jlimit(0.01f, 4.0f, delayTime);
        const float feedbackValue = juce::jlimit(0.0f, 0.995f, feedback);
        const float dampingValue = juce::jlimit(200.0f, 8000.0f, damping);
        const float modulationValue = juce::jlimit(0.0f, 1.0f, modulation);
        const float chorusRateValue = juce::jlimit(0.1f, 2.0f, chorusRate);
        const float stereoWidthValue = juce::jlimit(0.0f, 1.0f, stereoWidth);
        const bool tempoSyncEnabled = tempoSync;
        const int currentDelayMode = static_cast<int>(delayMode);

        updateDampingFilter();
        ignoreUnused(dampingValue);

        for (int s = 0; s < numSamples; ++s)
        {
            float actualDelayTime = delayTimeValue;

            if (tempoSyncEnabled && currentBPM > 0.0f)
            {                
                const int idx = juce::jlimit(0, 18, (int)std::round(delayTimeSynced));
                float beats = getBeatsForSyncedIndex(idx);
                actualDelayTime = (60.0f / currentBPM) * beats;
            }

            float delayTimeL = actualDelayTime;
            float delayTimeR = actualDelayTime * (1.0f + stereoWidthValue * 0.15f);

            lfoPhaseL += (2.0f * juce::MathConstants<float>::pi * chorusRateValue) / sampleRate;
            lfoPhaseR += (2.0f * juce::MathConstants<float>::pi * chorusRateValue) / sampleRate;
            if (lfoPhaseL >= 2.0f * juce::MathConstants<float>::pi)
                lfoPhaseL -= 2.0f * juce::MathConstants<float>::pi;
            if (lfoPhaseR >= 2.0f * juce::MathConstants<float>::pi)
                lfoPhaseR -= 2.0f * juce::MathConstants<float>::pi;
            const float lfoL = std::sin(lfoPhaseL);
            const float lfoR = std::sin(lfoPhaseR);

            int channelIndex = 0;
            for (auto ch : data)
            {
                if (channelIndex >= numChannels) break;
                if (channelIndex >= (int)delayLines.size()) break; // safety check

                dyn<float> channelData = data.toChannelData(ch);
                float input = channelData[s];
                float drySignal = input;
                
                float filteredInput = inputFilterState[channelIndex].process(input);

                const float channelDelayTime = (channelIndex == 0) ? delayTimeL : delayTimeR;
                const float lfoValue = (channelIndex == 0) ? lfoL : lfoR;
                
                const float chorusModulation = modulationValue * 0.005f * lfoValue;
                float modulatedDelayTime = juce::jlimit(0.01f, 4.0f, channelDelayTime + chorusModulation);

                float output = 0.0f;


                if (currentDelayMode == 0) { output = processForwardDelay(filteredInput, channelIndex, modulatedDelayTime, feedbackValue); } // forward
                else if (currentDelayMode == 1) { output = processReverseDelay(filteredInput, channelIndex, modulatedDelayTime, feedbackValue); } // reverse
                else if (currentDelayMode == 2) { output = processGlitchDelay(filteredInput, channelIndex, modulatedDelayTime, feedbackValue, modulationValue, s); } // glitch

                channelData[s] = drySignal * (1.0f - mixValue) + output * mixValue;
                channelIndex++;
            }
        }
    }
    
    template <int P> void setParameter(double v)
    {
        switch (P)
        {
        case 0: mix = static_cast<float>(v); break;
        case 1: delayTime = static_cast<float>(v); break;                 
        case 2: feedback = static_cast<float>(v); break;
        case 3: damping = static_cast<float>(v); break;
        case 4: modulation = static_cast<float>(v); break;
        case 5: stereoWidth = static_cast<float>(v); break;
        case 6: tempoSync = v > 0.5f; break;
        case 7: delayMode = static_cast<float>(v); reset(); break;
        case 8: delayTimeSynced = static_cast<float>(v); break;           
        case 9: 
        {
            int iv = (int)std::round(v);
            glitchModeParamValue = juce::jlimit(0, 1, iv);
            break;
        }
        }
    }
    
    void createParameters(ParameterDataList& data)
    {       
        {
            parameter::data mix_param("Mix", { 0.0, 1.0 });
            registerCallback<0>(mix_param);
            mix_param.setDefaultValue(0.5);
            data.add(std::move(mix_param));
        }
        {            
            parameter::data time_param("DelayTime", { 0.01, 4.0 });
            registerCallback<1>(time_param);
            time_param.setDefaultValue(0.5);
            data.add(std::move(time_param));
        }
        {            
            parameter::data time_sync_param("DelayTimeSynced", { 0.0, 18.0 });
            registerCallback<8>(time_sync_param);
            time_sync_param.setDefaultValue(5.0); // default to quarter
            data.add(std::move(time_sync_param));
        }
        {
            parameter::data feedback_param("Feedback", { 0.0, 0.98 });
            registerCallback<2>(feedback_param);
            feedback_param.setDefaultValue(0.7);
            data.add(std::move(feedback_param));
        }
        {
            parameter::data damping_param("Damping", { 200.0, 8000.0 });
            registerCallback<3>(damping_param);
            damping_param.setDefaultValue(2000.0);
            data.add(std::move(damping_param));
        }
        {
            parameter::data modulation_param("Modulation", { 0.0, 1.0 });
            registerCallback<4>(modulation_param);
            modulation_param.setDefaultValue(0.4);
            data.add(std::move(modulation_param));
        }
        {
            parameter::data width_param("StereoWidth", { 0.0, 1.0 });
            registerCallback<5>(width_param);
            width_param.setDefaultValue(0.3);
            data.add(std::move(width_param));
        }
        {
            parameter::data sync_param("TempoSync", { 0.0, 1.0 });
            registerCallback<6>(sync_param);
            sync_param.setDefaultValue(0.0);
            data.add(std::move(sync_param));
        }
        {
            parameter::data mode_param("DelayMode", { 0.0, 2.0 });
            registerCallback<7>(mode_param);
            mode_param.setDefaultValue(0.0);
            data.add(std::move(mode_param));
        }
        {            
            parameter::data glitch_mode_param("GlitchMode", { 0.0, 1.0 });
            registerCallback<9>(glitch_mode_param);
            glitch_mode_param.setDefaultValue(0.0);
            data.add(std::move(glitch_mode_param));
        }
    }

private:
    
    float sampleRate = 44100.0f;
    int numChannels = 2;
    float currentBPM = 120.0f;

    float mix = 0.5f;
    float delayTime = 0.5f;         
    float delayTimeSynced = 5.0f;   
    float feedback = 0.7f;
    float damping = 2000.0f;
    float modulation = 0.4f;
    float chorusRate = 0.6f;
    float stereoWidth = 0.3f;
    bool tempoSync = false;
    float delayMode = 0.0f;
    int glitchModeParamValue = 0;   // 0 = granular, 1 = stretch
    
    std::vector<std::vector<float>> delayLines; // sized in ctor/prepare
    std::vector<int> delayIndices;              // sized in ctor/prepare
    
    float lfoPhaseL = 0.0f;
    float lfoPhaseR = 0.0f;
    
    struct BiquadState
    {
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
        
        void reset()
        {
            x1 = x2 = y1 = y2 = 0.0f;
        }
        
        float process(float input)
        {
            float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            
            x2 = x1;
            x1 = input;
            y2 = y1;
            y1 = output;
            
            return output;
        }
    };

    BiquadState dampingFilterState[2];
    BiquadState inputFilterState[2];
    
    struct ReverseBlock
    {
        // more clanker nonsense
        std::vector<float> dataA;
        std::vector<float> dataB;

        bool     playIsA = false;      // true: play A, record B. false: play B, record A.
        size_t   writePos = 0;         // position in the recording buffer
        bool     recFilled = false;    // whether we've filled a full block in the current recording buffer

        bool     playing = false;      // whether we are currently playing a block
        size_t   playPos = 0;          // read position in the playing buffer (counts down)
        size_t   currentBlockSize = 0; // size of the current block being played

        bool     fadingIn = false;
        bool     fadingOut = false;
        size_t   fadeSamples = 0;
        size_t   fadePos = 0;
    };

    std::array<ReverseBlock, 2> reverseBlocks;
    float lastReverseSample[2] = { 0.0f, 0.0f };
    
    struct GlitchGrain
    {
        std::vector<float> data;
        size_t size = 0;
        size_t playPos = 0;
        float pitch = 1.0f;
        float gain = 1.0f;
        bool active = false;
        bool reverse = false;
        size_t totalFadeSamples = 0;
    };

    std::array<GlitchGrain, 4> glitchGrains[2];
    std::vector<float> glitchBuffer[2];
    int glitchWritePos[2] = { 0, 0 };
    int glitchStutterCounter[2] = { 0, 0 }; 
    int glitchStutterLength[2] = { 0, 0 };  
    float glitchRandomPhase = 0.0f;
    int glitchMode = 0;
    
    float getBeatsForSyncedIndex(int idx) const
    {
        if (idx <= 0) return 4.0f; 

        static constexpr int denoms[6] = { 2, 4, 8, 16, 32, 64 };
        const int i = idx - 1;
        const int group = i / 3;          
        const int posInGroup = i % 3;     

        const float baseBeats = 4.0f / (float)denoms[group];
        const float mult = (posInGroup == 0) ? 1.5f    // dotted
                          : (posInGroup == 1) ? 1.0f   // straight
                          : (2.0f / 3.0f);            // triplet

        return baseBeats * mult;
    }

    void updateDampingFilter()
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * damping / sampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float alpha = sinOmega / (2.0f * 0.707f);
        
        float b0 = (1.0f - cosOmega) / 2.0f;
        float b1 = 1.0f - cosOmega;
        float b2 = (1.0f - cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;
        
        for (int ch = 0; ch < 2; ++ch)
        {
            dampingFilterState[ch].b0 = b0 / a0;
            dampingFilterState[ch].b1 = b1 / a0;
            dampingFilterState[ch].b2 = b2 / a0;
            dampingFilterState[ch].a1 = a1 / a0;
            dampingFilterState[ch].a2 = a2 / a0;
        }
    }

    void updateInputFilter()
    {
        float freq = 80.0f;
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float alpha = sinOmega / (2.0f * 0.707f);
        
        float b0 = (1.0f + cosOmega) / 2.0f;
        float b1 = -(1.0f + cosOmega);
        float b2 = (1.0f + cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;
        
        for (int ch = 0; ch < 2; ++ch)
        {
            inputFilterState[ch].b0 = b0 / a0;
            inputFilterState[ch].b1 = b1 / a0;
            inputFilterState[ch].b2 = b2 / a0;
            inputFilterState[ch].a1 = a1 / a0;
            inputFilterState[ch].a2 = a2 / a0;
        }
    }

    float processForwardDelay(float input, int channel, float delayTimeSec, float feedbackValue)
    {
        auto& line = delayLines[channel];
        if (line.empty()) return input;

        const float maxIdx = static_cast<float>(line.size() - 1);
        float delaySamples = juce::jlimit(1.0f, maxIdx - 1.0f, delayTimeSec * sampleRate);
        
        float readIdx = static_cast<float>(delayIndices[channel]) - delaySamples;
        while (readIdx < 0.0f) readIdx += static_cast<float>(line.size());
        while (readIdx >= static_cast<float>(line.size())) readIdx -= static_cast<float>(line.size());

        int i0 = static_cast<int>(std::floor(readIdx));
        int i1 = (i0 + 1) % (int)line.size();
        float frac = readIdx - static_cast<float>(i0);
        
        float delayedSample = line[i0] + frac * (line[i1] - line[i0]);        
        float filteredFeedback = dampingFilterState[channel].process(delayedSample);
        float feedbackSignal = input + (filteredFeedback * feedbackValue);

        line[delayIndices[channel]] = feedbackSignal;
        delayIndices[channel] = (delayIndices[channel] + 1) % (int)line.size();

        return delayedSample * 0.8f;
    }

    float processReverseDelay(float input, int channel, float delayTimeSec, float feedbackValue)
    {
        auto& rb = reverseBlocks[channel];
        
        const size_t maxReverseSize = rb.dataA.size();
        if (maxReverseSize == 0) return input;

        size_t blockSize = static_cast<size_t>(delayTimeSec * sampleRate);
        blockSize = juce::jlimit<size_t>(1, maxReverseSize, blockSize);

        auto& playBuf = rb.playIsA ? rb.dataA : rb.dataB;
        auto& recBuf  = rb.playIsA ? rb.dataB : rb.dataA;

        float output = 0.0f;

        if (!rb.playing)
        {            
            float recordSample = input + feedbackValue * lastReverseSample[channel];

            if (rb.writePos >= recBuf.size()) rb.writePos = 0; // safety wrap
            recBuf[rb.writePos++] = recordSample;

            if (rb.writePos >= blockSize)
            {                
                rb.playing = true;
                rb.playIsA = !rb.playIsA; 
                rb.currentBlockSize = rb.writePos; 
                rb.playPos = rb.currentBlockSize;
                rb.fadingIn = true;
                rb.fadePos = 0;

                // prepare next recording                
                rb.writePos = 0;
                rb.recFilled = false;
            }

            // no output until we switch to play
            lastReverseSample[channel] = 0.0f;
            return 0.0f;
        }

        // read backwards from buffer
        if (rb.playPos > 0)
        {
            size_t readIndex = rb.playPos - 1;
            if (readIndex >= playBuf.size()) readIndex = 0; // safety
            output = playBuf[readIndex];
            rb.playPos--;

            // fade in start
            if (rb.fadingIn)
            {
                float gain = (rb.fadeSamples > 0) ? (static_cast<float>(rb.fadePos) / rb.fadeSamples) : 1.0f;
                output *= juce::jlimit(0.0f, 1.0f, gain);
                if (++rb.fadePos >= rb.fadeSamples)
                    rb.fadingIn = false;
            }

            // fade out end
            if (rb.playPos < rb.fadeSamples && rb.fadeSamples > 0)
            {
                float gain = static_cast<float>(rb.playPos) / rb.fadeSamples;
                output *= juce::jlimit(0.0f, 1.0f, gain);
            }
        }

        float filteredOut = dampingFilterState[channel].process(output);

        if (rb.writePos < blockSize)
        {
            recBuf[rb.writePos++] = input + feedbackValue * filteredOut;
            if (rb.writePos >= blockSize)
                rb.recFilled = true;
        }
        else
        {
            rb.writePos = 0;
            if (blockSize > 0)
            {
                recBuf[rb.writePos++] = input + feedbackValue * filteredOut;
                rb.recFilled = (blockSize == 1) ? true : rb.recFilled;
            }
        }

        if (rb.playPos == 0)
        {
            // if next block has enough samples, swap immediately
            size_t nextSize = rb.recFilled ? blockSize : rb.writePos;

            if (nextSize > 0)
            {
                rb.playIsA = !rb.playIsA;    // switch roles
                rb.currentBlockSize = juce::jmin(nextSize, blockSize);
                rb.playPos = rb.currentBlockSize;
                rb.fadingIn = true;
                rb.fadePos = 0;

                // reset recorder
                rb.writePos = 0;
                rb.recFilled = false;
            }
            else
            {                
                rb.playing = false;
                rb.writePos = 0;
                rb.recFilled = false;
            }
        }
        
        lastReverseSample[channel] = filteredOut;
        
        return filteredOut;
    }

    float processGlitchDelay(float input, int channel, float delayTimeSec, float feedbackValue, float glitchIntensity, int sampleIndex)
    {
        auto& buffer = glitchBuffer[channel];
        auto& grains = glitchGrains[channel];

        if (buffer.empty()) return input;

        float chunkSize = juce::jlimit(0.01f, 0.5f, delayTimeSec);
        
        ignoreUnused(sampleIndex, glitchIntensity);
        glitchMode = juce::jlimit(0, 1, glitchModeParamValue);
        
        float output = 0.0f;
        switch (glitchMode)
        {
        case 0: // grain
            output = processGranularGlitch(channel, chunkSize, buffer, grains);
            break;
        case 1: // stretch
            output = processPitchGlitch(channel, chunkSize, buffer, grains);
            break;
        default: // if somehow param gets poisoned
            output = 0.0f;
            break;
        }
        
        const int writePos = glitchWritePos[channel];
        float fbSample = dampingFilterState[channel].process(output); // tone-shape feedback
        buffer[glitchWritePos[channel]] = input + feedbackValue * fbSample;
        glitchWritePos[channel] = (glitchWritePos[channel] + 1) % (int)buffer.size();

        return output;
    }

    float processGranularGlitch(int channel, float chunkSize, std::vector<float>& buffer, std::array<GlitchGrain, 4>& grains)
    {
        float glitchIntensity = 1.0f;        
        if (juce::Random::getSystemRandom().nextFloat() < glitchIntensity * 0.1f) // random grains
        {
            for (auto& grain : grains)
            {
                if (!grain.active)
                {
                    size_t desired = static_cast<size_t>(chunkSize * sampleRate * (0.2f + juce::Random::getSystemRandom().nextFloat() * 0.8f));
                    desired = juce::jlimit<size_t>(1, grain.data.size(), desired);
                    desired = juce::jmin<size_t>(desired, buffer.size());

                    grain.size = desired;
                    grain.playPos = 0;
                    grain.active = true;
                    grain.reverse = juce::Random::getSystemRandom().nextBool();
                    grain.pitch = 0.5f + juce::Random::getSystemRandom().nextFloat() * 2.0f;
                    grain.gain = 0.3f + juce::Random::getSystemRandom().nextFloat() * 0.7f;

                    int maxStart = (int)buffer.size() - (int)grain.size;
                    int capturePos = (maxStart > 0) ? juce::Random::getSystemRandom().nextInt(maxStart) : 0;

                    for (size_t i = 0; i < grain.size; ++i)
                    {
                        grain.data[i] = buffer[(capturePos + (int)i) % (int)buffer.size()];
                    }
                    break;
                }
            }
        }

        float output = 0.0f;
        for (auto& grain : grains)
        {
            if (grain.active && grain.playPos < grain.size)
            {
                size_t pos = grain.reverse ? (grain.size - 1 - grain.playPos) : grain.playPos;
                float sample = grain.data[pos] * grain.gain;

                float env = 1.0f;
                if (grain.playPos < grain.totalFadeSamples)
                    env = static_cast<float>(grain.playPos) / grain.totalFadeSamples;
                else if (grain.playPos > grain.size - grain.totalFadeSamples)
                    env = static_cast<float>(grain.size - grain.playPos) / grain.totalFadeSamples;

                output += sample * juce::jlimit(0.0f, 1.0f, env) * 0.25f;
                grain.playPos += static_cast<size_t>(juce::jmax(1.0f, grain.pitch));
            }
            else if (grain.active)
            {
                grain.active = false;
            }
        }
        return output;
    }

    float processPitchGlitch(int channel, float chunkSize,
                            std::vector<float>& buffer, std::array<GlitchGrain, 4>& grains)
    {
        if (glitchStutterCounter[channel] <= 0)
        {
            auto& grain = grains[0];
            size_t desired = static_cast<size_t>(chunkSize * sampleRate);
            desired = juce::jlimit<size_t>(1, grain.data.size(), desired);
            desired = juce::jmin<size_t>(desired, buffer.size());

            glitchStutterLength[channel] = (int)desired;
            glitchStutterCounter[channel] = (int)desired;

            grain.size = desired;
            grain.playPos = 0;
            grain.active = true;
            grain.pitch = 0.25f + juce::Random::getSystemRandom().nextFloat() * 3.0f;

            int readPos = (int)glitchWritePos[channel] - (int)grain.size;
            if (readPos < 0) readPos += (int)buffer.size();

            for (size_t i = 0; i < grain.size; ++i)
            {
                grain.data[i] = buffer[(readPos + (int)i) % (int)buffer.size()];
            }
        }

        float output = 0.0f;
        if (grains[0].active)
        {
            size_t pos = static_cast<size_t>(grains[0].playPos);
            if (pos < grains[0].size)
            {                
                size_t pos1 = pos % grains[0].size;
                size_t pos2 = (pos + 1) % grains[0].size;
                float frac = grains[0].playPos - (float)pos;
                output = grains[0].data[pos1] * (1.0f - frac) + grains[0].data[pos2] * frac;
            }

            grains[0].playPos += grains[0].pitch;
            if (grains[0].playPos >= grains[0].size) { grains[0].playPos = 0;  } // loop the grain            
        }

        glitchStutterCounter[channel]--;
        return output;
    }
};
}