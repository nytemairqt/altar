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

template <int NV> struct reverb: public data::base
{
    SNEX_NODE(reverb);        
    
    struct MetadataClass
    {
        SN_NODE_ID("reverb");
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

    reverb()
    {
        mainDelayLines.resize(2);
        mainDelayIndices.resize(2);
        allpassDelayLines.resize(2);
        allpassIndices.resize(2);
        
        for (int ch = 0; ch < 2; ++ch)
        {
            allpassDelayLines[ch].resize(numAllpassFilters);
            allpassIndices[ch].resize(numAllpassFilters);
        }
    }
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate = static_cast<float>(specs.sampleRate);
        numChannels = static_cast<int>(specs.numChannels);
        
        const float maxDelayTime = 6.0f; // 6 seconds max
        int maxDelayInSamples = static_cast<int>(maxDelayTime * sampleRate);

        for (int i = 0; i < numAllpassFilters; ++i) { allpassChannelOffset[i] = (i * 3 + 5) % 15 + 5;  }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            mainDelayLines[ch].resize(maxDelayInSamples, 0.0f);
            mainDelayIndices[ch] = 0;

            for (int i = 0; i < numAllpassFilters; ++i)
            {
                int allpassDelay = allpassDelayTimes[i] + (ch * allpassChannelOffset[i]);
                allpassDelayLines[ch][i].resize(allpassDelay, 0.0f);
                allpassIndices[ch][i] = 0;
            }
        }

        lfoPhase = 0.0f;
        
        updateDampingFilter();

        preDelayBuffer.resize(numChannels);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            preDelayBuffer[ch].resize(static_cast<int>(preDelayMaxTime * sampleRate), 0.0f);
        }
        preDelayWriteIndex = 0;

        reset();
    }       

    void reset() 
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            std::fill(mainDelayLines[ch].begin(), mainDelayLines[ch].end(), 0.0f);
            mainDelayIndices[ch] = 0;

            for (int i = 0; i < numAllpassFilters; ++i)
            {
                std::fill(allpassDelayLines[ch][i].begin(), allpassDelayLines[ch][i].end(), 0.0f);
                allpassIndices[ch][i] = 0;
            }
            
            if (ch < preDelayBuffer.size())
                std::fill(preDelayBuffer[ch].begin(), preDelayBuffer[ch].end(), 0.0f);
        }

        for (int ch = 0; ch < 2; ++ch) { dampingFilterState[ch].reset(); }       

        preDelayWriteIndex = 0;
        lfoPhase = 0.0f;
        lastDiffusedL = 0.0f;
        lastDiffusedR = 0.0f;
    }
    
    template <typename T> void process(T& data)
    {
        int numSamples = data.getNumSamples();
        const float mixValue = juce::jlimit(0.0f, 1.0f, mix);
        const float roomSizeValue = juce::jlimit(0.0f, 1.0f, roomSize);
        const float decayValue = juce::jlimit(0.0f, 0.995f, decay);
        const float chorusDepthValue = juce::jlimit(0.0f, 1.0f, chorusDepth);
        const float chorusRateValue = juce::jlimit(0.1f, 5.0f, chorusRate);
        const float preDelayTimeValue = juce::jlimit(0.0f, preDelayMaxTime, preDelayTime);

        float baseDelayTime = 0.02f + (roomSizeValue * 0.15f); 
        int preDelaySamples = static_cast<int>(preDelayTimeValue * sampleRate);

        updateDampingFilter();

        for (int s = 0; s < numSamples; ++s)
        {
            lfoPhase += (2.0f * juce::MathConstants<float>::pi * chorusRateValue) / sampleRate;
            if (lfoPhase >= 2.0f * juce::MathConstants<float>::pi)
                lfoPhase -= 2.0f * juce::MathConstants<float>::pi;

            float lfoValue = std::sin(lfoPhase);

            int channelIndex = 0;
            for (auto ch : data)
            {
                if (channelIndex >= numChannels) break;
                
                dyn<float> channelData = data.toChannelData(ch);
                float input = channelData[s];
                float drySignal = input;

                float preDelayedInput = input;
                if (preDelaySamples > 0 && channelIndex < preDelayBuffer.size())
                {
                    int readIndex = (preDelayWriteIndex - preDelaySamples + preDelayBuffer[channelIndex].size()) % preDelayBuffer[channelIndex].size();
                    preDelayedInput = preDelayBuffer[channelIndex][readIndex];
                    preDelayBuffer[channelIndex][preDelayWriteIndex] = input;
                }

                float chorusModulation = chorusDepthValue * 0.002f * lfoValue; // ~2ms modulation
                float modulatedDelayTime = baseDelayTime + chorusModulation;

                if (channelIndex == 1)
                    modulatedDelayTime *= 1.13f; // slight delay on R channel for width

                int delayInSamples = juce::jlimit(1, static_cast<int>(mainDelayLines[channelIndex].size() - 1),
                    static_cast<int>(modulatedDelayTime * sampleRate));

                float delayIndex = static_cast<float>(mainDelayIndices[channelIndex]) - static_cast<float>(delayInSamples);
                if (delayIndex < 0.0f)
                    delayIndex += static_cast<float>(mainDelayLines[channelIndex].size());

                int index1 = static_cast<int>(delayIndex);
                int index2 = (index1 + 1) % mainDelayLines[channelIndex].size();
                float fraction = delayIndex - static_cast<float>(index1);

                float delayedSample = mainDelayLines[channelIndex][index1] * (1.0f - fraction) + 
                                     mainDelayLines[channelIndex][index2] * fraction;

                float diffusedSample = delayedSample;
                for (int i = 0; i < numAllpassFilters; ++i)
                {
                    diffusedSample = processAllpass(diffusedSample, channelIndex, i, allpassGain);
                }

                if (channelIndex == 0) { lastDiffusedL = diffusedSample; }                    
                else { lastDiffusedR = diffusedSample; }
                    
                float filteredSample = dampingFilterState[channelIndex].process(diffusedSample);

                float feedbackSignal = preDelayedInput + (filteredSample * decayValue);
                float crossfeed = 0.3f; // 30% of opposite channel
                if (numChannels == 2)
                {
                    float other = (channelIndex == 0) ? lastDiffusedR : lastDiffusedL;
                    feedbackSignal += crossfeed * other * decayValue;
                }

                mainDelayLines[channelIndex][mainDelayIndices[channelIndex]] = feedbackSignal;
                mainDelayIndices[channelIndex] = (mainDelayIndices[channelIndex] + 1) % mainDelayLines[channelIndex].size();

                float wetSignal = filteredSample * 0.3f; // wet signal is loud, so scale it
                channelData[s] = drySignal * (1.0f - mixValue) + wetSignal * mixValue;
                
                channelIndex++;
            }

            if (preDelaySamples > 0)
                preDelayWriteIndex = (preDelayWriteIndex + 1) % preDelayBuffer[0].size();
        }
    }
    
    template <int P> void setParameter(double v)
    {
        switch (P)
        {
        case 0: mix = static_cast<float>(v); break;
        case 1: preDelayTime = static_cast<float>(v); break;
        case 2: roomSize = static_cast<float>(v); break;
        case 3: decay = static_cast<float>(v); break;
        case 4: dampingFrequency = static_cast<float>(v); break;
        case 5: chorusDepth = static_cast<float>(v); break;
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
            parameter::data predelay_param("PreDelay", { 0.0, 0.2 });
            registerCallback<1>(predelay_param);
            predelay_param.setDefaultValue(0.03);
            data.add(std::move(predelay_param));
        }
        {
            parameter::data roomsize_param("RoomSize", { 0.0, 1.0 });
            registerCallback<2>(roomsize_param);
            roomsize_param.setDefaultValue(0.7);
            data.add(std::move(roomsize_param));
        }
        {
            parameter::data decay_param("Decay", { 0.0, 0.98 });
            registerCallback<3>(decay_param);
            decay_param.setDefaultValue(0.8);
            data.add(std::move(decay_param));
        }
        {
            parameter::data damping_param("DampingFrequency", { 500.0, 20000.0 });
            registerCallback<4>(damping_param);
            damping_param.setDefaultValue(3000.0);
            data.add(std::move(damping_param));
        }
        {
            parameter::data chorus_param("ChorusDepth", { 0.0, 1.0 });
            registerCallback<5>(chorus_param);
            chorus_param.setDefaultValue(0.3);
            data.add(std::move(chorus_param));
        }
    }

private:
    float sampleRate = 44100.0f;
    int numChannels = 2;
    
    float mix = 0.5f;
    float preDelayTime = 0.03f;
    float roomSize = 0.7f;
    float decay = 0.8f;
    float dampingFrequency = 3000.0f;
    float chorusDepth = 0.3f;
    float chorusRate = 0.8f;
    
    std::vector<std::vector<float>> mainDelayLines;
    std::vector<int> mainDelayIndices;

    float lastDiffusedL = 0.0f;
    float lastDiffusedR = 0.0f;

    static constexpr int numAllpassFilters = 10;
    static constexpr float allpassGain = 0.7f;
    static constexpr float preDelayMaxTime = 0.2f;
        
    const int allpassDelayTimes[numAllpassFilters] = { 347, 113, 229, 179, 283, 317, 389, 443, 509, 557 };
    int allpassChannelOffset[numAllpassFilters];

    std::vector<std::vector<std::vector<float>>> allpassDelayLines;
    std::vector<std::vector<int>> allpassIndices;
    
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

    std::vector<std::vector<float>> preDelayBuffer;
    int preDelayWriteIndex = 0;

    float lfoPhase = 0.0f;    

    void updateDampingFilter()
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * dampingFrequency / sampleRate;
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
            dampingFilterState[ch].b0 = b0 / a0;
            dampingFilterState[ch].b1 = b1 / a0;
            dampingFilterState[ch].b2 = b2 / a0;
            dampingFilterState[ch].a1 = a1 / a0;
            dampingFilterState[ch].a2 = a2 / a0;
        }
    }

    float processAllpass(float input, int channel, int filterIndex, float gain)
    {
        if (channel >= allpassIndices.size() || filterIndex >= allpassIndices[channel].size())
            return input;
            
        int& index = allpassIndices[channel][filterIndex];
        auto& delayLine = allpassDelayLines[channel][filterIndex];

        float delayedSample = delayLine[index];
        float output = -gain * input + delayedSample;
        delayLine[index] = input + gain * output;

        index = (index + 1) % delayLine.size();
        return output;
    }
    
    
};
}