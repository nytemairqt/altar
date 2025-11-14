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

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;

template <int NV> struct chorus: public data::base
{
    SNEX_NODE(chorus);        
    
    struct MetadataClass
    {
        SN_NODE_ID("chorus");
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

    chorus()
    {        
        delayLines.resize(2);
        delayIndices.assign(2, 0);
                
        for (int i = 0; i < 8; ++i) { lfoPhase[i] = (i % 2) * juce::MathConstants<float>::pi * 0.25f; }        
    }
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate = static_cast<float>(specs.sampleRate);
        numChannels = static_cast<int>(specs.numChannels);
        numChannels = juce::jlimit(1, 2, numChannels);
        
        const float maxDelayTime = 0.05f; // 50 ms
        int maxDelayInSamples = static_cast<int>(maxDelayTime * sampleRate);
        
        delayLines.resize(numChannels);
        delayIndices.resize(numChannels);
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            delayLines[ch].assign(maxDelayInSamples, 0.0f);
            delayIndices[ch] = 0;
        }
        
        updateInputFilter();
        updateOutputFilter();

        currentBPM = 120.0f; // might still end up tempo syncing...
        reset();
    }       

    void reset() 
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            if (ch < delayLines.size())
            {
                std::fill(delayLines[ch].begin(), delayLines[ch].end(), 0.0f);
                delayIndices[ch] = 0;
            }
        }
        
        for (int i = 0; i < 8; ++i) { lfoPhase[i] = (i % 2) * juce::MathConstants<float>::pi * 0.25f; } // reset voices
        for (int ch = 0; ch < 2; ++ch) { inputFilterState[ch].reset(); outputFilterState[ch].reset(); } // reset filters
       
    }
    
    template <typename T> void process(T& data)
    {
        const int numSamples = data.getNumSamples();
        const float mixValue = juce::jlimit(0.0f, 1.0f, mix);
        const float rateValue = juce::jlimit(0.1f, 10.0f, rate);
        const float depthValue = juce::jlimit(0.0f, 1.0f, depth);
        const float feedbackValue = juce::jlimit(0.0f, 0.8f, feedback);
        const float delayTimeValue = juce::jlimit(5.0f, 30.0f, delayTime);
        const float preDelayValue = juce::jlimit(0.0f, 10.0f, preDelay);
        const int voicesValue = juce::jlimit(1, 4, static_cast<int>(voices));
        const bool tempoSyncEnabled = tempoSync;
        
        updateOutputFilter();

        for (int s = 0; s < numSamples; ++s)
        {            
            float actualRate = rateValue;
            if (tempoSyncEnabled && currentBPM > 0.0f)
            {
                float noteValues[] = { 1.0f/8.0f, 1.0f/4.0f, 1.0f/2.0f, 1.0f, 2.0f, 4.0f, 8.0f };
                int noteIndex = juce::jlimit(0, 6, static_cast<int>(rateValue * 6.0f / 10.0f));
                actualRate = (currentBPM / 60.0f) * noteValues[noteIndex];
            }

            int channelIndex = 0;
            for (auto ch : data)
            {
                if (channelIndex >= numChannels || channelIndex >= delayLines.size()) break;
                
                dyn<float> channelData = data.toChannelData(ch);
                float input = channelData[s];
                float drySignal = input;
                
                float filteredInput = inputFilterState[channelIndex].process(input);
                
                float baseDelayMs = delayTimeValue + preDelayValue;
                float baseDelaySamples = (baseDelayMs / 1000.0f) * sampleRate;

                float wetOutput = 0.0f;
                
                for (int voice = 0; voice < voicesValue; ++voice)
                {                    
                    float phaseOffset = voice * juce::MathConstants<float>::pi * 0.5f;
                    if (channelIndex == 1) // right channel offset for width
                        phaseOffset += juce::MathConstants<float>::pi * 0.3f;

                    int lfoIndex = channelIndex * 4 + voice;
                    if (lfoIndex < 8)
                    {
                        float& lfoPhaseRef = lfoPhase[lfoIndex];
                        lfoPhaseRef += (2.0f * juce::MathConstants<float>::pi * actualRate) / sampleRate;
                        if (lfoPhaseRef >= 2.0f * juce::MathConstants<float>::pi)
                            lfoPhaseRef -= 2.0f * juce::MathConstants<float>::pi;
                        
                        float lfoValue;
                        float normalizedPhase = (lfoPhaseRef + phaseOffset);
                        while (normalizedPhase >= 2.0f * juce::MathConstants<float>::pi)
                            normalizedPhase -= 2.0f * juce::MathConstants<float>::pi;
                        while (normalizedPhase < 0.0f)
                            normalizedPhase += 2.0f * juce::MathConstants<float>::pi;

                        normalizedPhase /= (2.0f * juce::MathConstants<float>::pi);

                        // triangle wave                        
                        if (normalizedPhase < 0.5f)
                            lfoValue = 4.0f * normalizedPhase - 1.0f;
                        else
                            lfoValue = 3.0f - 4.0f * normalizedPhase;
                        
                        lfoValue = lfoValue * 0.8f + std::sin(normalizedPhase * 2.0f * juce::MathConstants<float>::pi) * 0.2f;
                        
                        float modulationRange = 5.0f; // ±5ms mod range
                        float modulatedDelayMs = baseDelayMs + (lfoValue * depthValue * modulationRange);
                        float modulatedDelaySamples = juce::jlimit(1.0f, static_cast<float>(delayLines[channelIndex].size() - 1),
                                                                  (modulatedDelayMs / 1000.0f) * sampleRate);
                        
                        float delayIndex = static_cast<float>(delayIndices[channelIndex]) - modulatedDelaySamples;
                        if (delayIndex < 0.0f)
                            delayIndex += static_cast<float>(delayLines[channelIndex].size());

                        int index1 = static_cast<int>(delayIndex);
                        int index2 = (index1 + 1) % delayLines[channelIndex].size();
                        float fraction = delayIndex - static_cast<float>(index1);

                        float delayedSample = delayLines[channelIndex][index1] * (1.0f - fraction) +
                                             delayLines[channelIndex][index2] * fraction;
                        
                        float detuning = 1.0f + (voice - voicesValue * 0.5f) * 0.001f;
                        delayedSample *= detuning;

                        wetOutput += delayedSample / voicesValue; // average voices
                    }
                }
                
                float feedbackSignal = filteredInput + (wetOutput * feedbackValue);                        
                feedbackSignal = std::tanh(feedbackSignal * 0.8f) * 1.2f;

                // write to delay lines
                delayLines[channelIndex][delayIndices[channelIndex]] = feedbackSignal;
                delayIndices[channelIndex] = (delayIndices[channelIndex] + 1) % delayLines[channelIndex].size();
                                
                float filteredOutput = outputFilterState[channelIndex].process(wetOutput); // output lowpass
                
                float noise = (juce::Random::getSystemRandom().nextFloat() - 0.5f) * 0.0002f; // noise floor
                filteredOutput += noise;
                
                float finalWet = filteredOutput;
                if (channelIndex == 1 && voicesValue > 2) // slight R phase variation
                    finalWet *= -0.95f; 

                channelData[s] = drySignal * (1.0f - mixValue) + finalWet * mixValue * 0.7f;                
                channelIndex++;
            }
        }
    }
    
    template <int P> void setParameter(double v)
    {
        switch (P)
        {
        case 0: mix = static_cast<float>(v); break;
        case 1: rate = static_cast<float>(v); break;
        case 2: depth = static_cast<float>(v); break;
        case 3: feedback = static_cast<float>(v); break;
        case 4: delayTime = static_cast<float>(v); break;
        case 5: preDelay = static_cast<float>(v); break;
        case 6: tone = static_cast<float>(v); break;
        case 7: voices = static_cast<float>(v); break;
        case 8: tempoSync = v > 0.5f; break;
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
            parameter::data rate_param("Rate", { 0.1, 10.0 });
            registerCallback<1>(rate_param);
            rate_param.setDefaultValue(0.8);
            data.add(std::move(rate_param));
        }
        {
            parameter::data depth_param("Depth", { 0.0, 1.0 });
            registerCallback<2>(depth_param);
            depth_param.setDefaultValue(0.7);
            data.add(std::move(depth_param));
        }
        {
            parameter::data feedback_param("Feedback", { 0.0, 0.8 });
            registerCallback<3>(feedback_param);
            feedback_param.setDefaultValue(0.2);
            data.add(std::move(feedback_param));
        }
        {
            parameter::data delaytime_param("DelayTime", { 5.0, 30.0 });
            registerCallback<4>(delaytime_param);
            delaytime_param.setDefaultValue(15.0);
            data.add(std::move(delaytime_param));
        }
        {
            parameter::data predelay_param("PreDelay", { 0.0, 10.0 });
            registerCallback<5>(predelay_param);
            predelay_param.setDefaultValue(2.0);
            data.add(std::move(predelay_param));
        }
        {
            parameter::data tone_param("Tone", { 200.0, 10000.0 });
            registerCallback<6>(tone_param);
            tone_param.setDefaultValue(5000.0);
            data.add(std::move(tone_param));
        }
        {
            parameter::data voices_param("Voices", { 1.0, 4.0 });
            registerCallback<7>(voices_param);
            voices_param.setDefaultValue(2.0);
            data.add(std::move(voices_param));
        }
        {
            parameter::data sync_param("TempoSync", { 0.0, 1.0 });
            registerCallback<8>(sync_param);
            sync_param.setDefaultValue(0.0);
            data.add(std::move(sync_param));
        }
    }

private:
    float sampleRate = 44100.0f;
    int numChannels = 2;
    float currentBPM = 120.0f;

    float mix = 0.5f;
    float rate = 0.8f;
    float depth = 0.7f;
    float feedback = 0.2f;
    float delayTime = 15.0f;
    float preDelay = 2.0f;
    float tone = 5000.0f;
    float voices = 2.0f;
    bool tempoSync = false;
    
    std::vector<std::vector<float>> delayLines;
    std::vector<int> delayIndices;
    
    float lfoPhase[8] = { 0.0f }; 
    
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
        float freq = 40.0f;
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
        float freq = juce::jlimit(200.0f, 10000.0f, tone);
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
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
            outputFilterState[ch].b0 = b0 / a0;
            outputFilterState[ch].b1 = b1 / a0;
            outputFilterState[ch].b2 = b2 / a0;
            outputFilterState[ch].a1 = a1 / a0;
            outputFilterState[ch].a2 = a2 / a0;
        }
    }
};
}