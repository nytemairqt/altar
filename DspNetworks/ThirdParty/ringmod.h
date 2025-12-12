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

enum class GlobalCablesRingmod
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

template <int NV> struct ringmod: public data::base, public cable_manager_t
{
    SNEX_NODE(ringmod);        
    
    struct MetadataClass
    {
        SN_NODE_ID("ringmod");
    };
    
    static constexpr bool isModNode() { return false; };
    static constexpr bool isPolyphonic() { return NV > 1; };
    static constexpr bool hasTail() { return false; };
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

    ringmod()
    {
        this->registerDataCallback<GlobalCablesRingmod::tempo>([this](const var& data)
        {
            // set global tempo here
            currentBPM = data;
        });

        for (int ch = 0; ch < 2; ++ch)
        {
            oscillatorPhase[ch] = 0.0f;
            lfoPhase[ch] = 0.0f;
            envelopeState[ch] = 0.0f;
        }
    }
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate = static_cast<float>(specs.sampleRate);
        numChannels = static_cast<int>(specs.numChannels);
        
        updateInputFilter();
        updateOutputFilter();
        
        reset();
    }       

    void reset() 
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            oscillatorPhase[ch] = 0.0f;
            lfoPhase[ch] = 0.0f;
            envelopeState[ch] = 0.0f;
            
            inputFilterState[ch].reset();
            outputFilterState[ch].reset();
        }
    }
    
    template <typename T> void process(T& data)
    {
        const int numSamples = data.getNumSamples();
        const float mixValue = juce::jlimit(0.0f, 1.0f, mix);
        const float freqValue = juce::jlimit(1.0f, 2000.0f, frequency);
        const float depthValue = juce::jlimit(0.0f, 1.0f, depth);
        float lfoRateValue = juce::jlimit(0.1f, 10.0f, lfoRate);
        const float lfoDepthValue = juce::jlimit(0.0f, 1.0f, lfoDepth);
        const int modeValue = static_cast<int>(mode) % 4;
        const bool stereoModeEnabled = stereoMode;
        const bool tempoSyncEnabled = tempoSync;
        
        if (tempoSyncEnabled && currentBPM > 0.0f)
        {
            const int idx = juce::jlimit(0, 18, (int)std::round(frequencySynced));
            const float beats = getBeatsForSyncedIndex(idx);
            lfoRateValue = (currentBPM / 60.0f) / juce::jmax(0.0001f, beats);
            lfoRateValue = juce::jlimit(0.1f, 10.0f, lfoRateValue);
        }
        
        updateOutputFilter();

        for (int s = 0; s < numSamples; ++s)
        {
            int channelIndex = 0;
            for (auto ch : data)
            {
                if (channelIndex >= numChannels || channelIndex >= 2) break;
                
                dyn<float> channelData = data.toChannelData(ch);
                float input = channelData[s];
                float drySignal = input;

                float filteredInput = inputFilterState[channelIndex].process(input);                
                float baseFreq = freqValue;
                float& lfoPhaseRef = lfoPhase[channelIndex];
                lfoPhaseRef += (2.0f * juce::MathConstants<float>::pi * lfoRateValue) / sampleRate;
                if (lfoPhaseRef >= 2.0f * juce::MathConstants<float>::pi)
                    lfoPhaseRef -= 2.0f * juce::MathConstants<float>::pi;

                float lfoValue = std::sin(lfoPhaseRef) * lfoDepthValue;
                float modulatedFreq = baseFreq * (1.0f + lfoValue * 0.5f);
                
                if (stereoModeEnabled && channelIndex == 1)
                    modulatedFreq *= 1.05f;

                float modulatorSignal = 0.0f;
                float& oscPhaseRef = oscillatorPhase[channelIndex];

                switch (modeValue)
                {
                case 0: // sine
                    modulatorSignal = std::sin(oscPhaseRef);
                    oscPhaseRef += (2.0f * juce::MathConstants<float>::pi * modulatedFreq) / sampleRate;
                    break;

                case 1: // square
                    modulatorSignal = (std::sin(oscPhaseRef) >= 0.0f) ? 1.0f : -1.0f;
                    oscPhaseRef += (2.0f * juce::MathConstants<float>::pi * modulatedFreq) / sampleRate;
                    break;

                case 2: // saw
                    modulatorSignal = (2.0f * (oscPhaseRef / (2.0f * juce::MathConstants<float>::pi))) - 1.0f;
                    oscPhaseRef += (2.0f * juce::MathConstants<float>::pi * modulatedFreq) / sampleRate;
                    break;

                case 3: // envelope follower
                {
                    float inputLevel = std::abs(filteredInput);
                    float& envelopeRef = envelopeState[channelIndex];
                    float attack = 0.001f;  // 1ms
                    float release = 0.1f;   // 100ms

                    if (inputLevel > envelopeRef)
                        envelopeRef += (inputLevel - envelopeRef) * (1.0f - std::exp(-1.0f / (attack * sampleRate)));
                    else
                        envelopeRef += (inputLevel - envelopeRef) * (1.0f - std::exp(-1.0f / (release * sampleRate)));

                    float envModulatedFreq = modulatedFreq * (0.5f + envelopeRef * 2.0f);
                    modulatorSignal = std::sin(oscPhaseRef);
                    oscPhaseRef += (2.0f * juce::MathConstants<float>::pi * envModulatedFreq) / sampleRate;
                    break;
                }
                }

                if (oscPhaseRef >= 2.0f * juce::MathConstants<float>::pi)
                    oscPhaseRef -= 2.0f * juce::MathConstants<float>::pi;
                
                float ringModOutput = filteredInput * (1.0f + modulatorSignal * depthValue);
                float filteredOutput = outputFilterState[channelIndex].process(ringModOutput);
                
                filteredOutput = std::tanh(filteredOutput * 0.7f) * 1.2f;
                
                channelData[s] = drySignal * (1.0f - mixValue) + filteredOutput * mixValue;
                
                channelIndex++;
            }
        }
    }
    
    template <int P> void setParameter(double v)
    {
        switch (P)
        {
        case 0: mix = static_cast<float>(v); break;
        case 1: frequency = static_cast<float>(v); break;
        case 2: depth = static_cast<float>(v); break;
        case 3: mode = static_cast<float>(v); break;
        case 4: lfoRate = static_cast<float>(v); break;
        case 5: frequencySynced = static_cast<float>(v); break; 
        case 6: lfoDepth = static_cast<float>(v); break;
        case 7: filterFrequency = static_cast<float>(v); break;
        case 8: stereoMode = v > 0.5f; break;
        case 9: tempoSync = v > 0.5f; break;        
        }
    }
    
    void createParameters(ParameterDataList& data)
    {       
        {
            parameter::data mix_param("Mix", { 0.0, 1.0 });
            registerCallback<0>(mix_param);
            mix_param.setDefaultValue(0.3);
            data.add(std::move(mix_param));
        }
        {
            parameter::data freq_param("Frequency", { 1.0, 2000.0 });
            registerCallback<1>(freq_param);
            freq_param.setDefaultValue(100.0);
            data.add(std::move(freq_param));
        }
        {
            parameter::data depth_param("Depth", { 0.0, 1.0 });
            registerCallback<2>(depth_param);
            depth_param.setDefaultValue(0.7);
            data.add(std::move(depth_param));
        }
        {
            parameter::data mode_param("Mode", { 0.0, 3.0 });
            registerCallback<3>(mode_param);
            mode_param.setDefaultValue(0.0);
            data.add(std::move(mode_param));
        }
        {
            parameter::data lfo_rate_param("LFORate", { 0.1, 10.0 });
            registerCallback<4>(lfo_rate_param);
            lfo_rate_param.setDefaultValue(2.0);
            data.add(std::move(lfo_rate_param));
        }
        {            
            parameter::data freq_sync_param("LFORateSynced", { 0.0, 18.0 });
            registerCallback<5>(freq_sync_param);
            freq_sync_param.setDefaultValue(5.0); // default to 1/4 note
            data.add(std::move(freq_sync_param));
        }
        {
            parameter::data lfo_depth_param("LFODepth", { 0.0, 1.0 });
            registerCallback<6>(lfo_depth_param);
            lfo_depth_param.setDefaultValue(0.2);
            data.add(std::move(lfo_depth_param));
        }
        {
            parameter::data filter_param("FilterFrequency", { 200.0, 12000.0 });
            registerCallback<7>(filter_param);
            filter_param.setDefaultValue(8000.0);
            data.add(std::move(filter_param));
        }
        {
            parameter::data stereo_param("StereoMode", { 0.0, 1.0 });
            registerCallback<8>(stereo_param);
            stereo_param.setDefaultValue(1.0);
            data.add(std::move(stereo_param));
        }
        {
            parameter::data tempo_param("TempoSync", { 0.0, 1.0 });
            registerCallback<9>(tempo_param);
            tempo_param.setDefaultValue(0.0);
            data.add(std::move(tempo_param));
        }
        
    }

private:
    float sampleRate = 44100.0f;
    int numChannels = 2;
    float currentBPM = 120.0f;

    float mix = 0.3f;
    float frequency = 100.0f;
    float depth = 0.7f;
    float mode = 0.0f;
    float lfoRate = 2.0f;
    float lfoDepth = 0.2f;
    float filterFrequency = 8000.0f;
    bool stereoMode = true;
    bool tempoSync = false;
    float frequencySynced = 5.0f;
    
    float oscillatorPhase[2] = { 0.0f, 0.0f };
    float lfoPhase[2] = { 0.0f, 0.0f };
    float envelopeState[2] = { 0.0f, 0.0f };

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

    BiquadState inputFilterState[2];
    BiquadState outputFilterState[2];

    void updateInputFilter()
    {
        float freq = 80.0f;
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float alpha = sinOmega / (2.0f * 0.707f); // Q = 0.707
        
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

    void updateOutputFilter()
    {
        float freq = juce::jlimit(200.0f, 12000.0f, filterFrequency);
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float alpha = sinOmega / (2.0f * 0.707f); // Q = 0.707
        
        float b0 = (1.0f - cosOmega) / 2.0f;
        float b1 = 1.0f - cosOmega;
        float b2 = (1.0f - cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;
        
        for (int ch = 0; ch < 2; ++ch)
        {
            outputFilterState[ch].b0 = b0 / a0;
            outputFilterState[ch].b1 = b1 / a0;
            outputFilterState[ch].b2 = b2 / a0;
            outputFilterState[ch].a1 = a1 / a0;
            outputFilterState[ch].a2 = a2 / a0;
        }
    }

    float getBeatsForSyncedIndex(int idx) const
    {
        if (idx <= 0) return 4.0f; // 1/1

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
};
}