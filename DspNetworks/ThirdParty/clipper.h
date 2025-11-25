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

template <int NV> struct clipper: public data::base
{
    SNEX_NODE(clipper);        
    
    struct MetadataClass
    {
        SN_NODE_ID("clipper");
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
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate = static_cast<float>(specs.sampleRate);
        numChannels = specs.numChannels;    
    }       

    void reset() 
    {

    }
    
    template <typename T> void process(T& data)
    {
        int numSamples = data.getNumSamples();
        int channelIndex = 0;
        
        for(auto ch: data)
        {
            dyn<float> channel = data.toChannelData(ch);
            
            for (int s = 0; s < numSamples; ++s)
            {
                channel[s] = processClipper(channel[s]);
            }

            channelIndex++;
        }
    }
    
    template <int P> void setParameter(double v)
    {
        if (P == 0) { clipperInputGain = static_cast<float>(v); }
    }
    
    void createParameters(ParameterDataList& data)
    {               
        {
            parameter::data clipperGain("Clipper Gain", { 0.0, 2.0 });
            registerCallback<0>(clipperGain);
            clipperGain.setDefaultValue(0.0);
            data.add(std::move(clipperGain));
        }        
    }

private:
    float sampleRate = 44100.0f;
    int numChannels = 2;        
    
    float clipperInputGain = 0.0f;        

    float processClipper(float input)
    {
        float output = input * (1.0f + clipperInputGain);
        
        const float threshold = 0.8f;
        const float knee = 0.1f;
        const float absInput = std::abs(output);

        if (absInput < threshold - knee)
            return output;
        if (absInput > threshold + knee)
            return (output > 0.0f) ? threshold : -threshold;
                
        const float x = (absInput - (threshold - knee)) / (2.0f * knee);
        const float curve = 0.5f * (1.0f + std::tanh(juce::MathConstants<float>::pi * (x - 0.5f)));
        const float clipped = threshold - knee + curve * 2.0f * knee;
        return (output > 0.0f) ? clipped : -clipped;
    }    
};
}