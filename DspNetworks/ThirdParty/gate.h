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

/*
    using some of the math from http://github.com/sdatkinson/AudioDSPTools/
*/

#pragma once
#include <JuceHeader.h>

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;

template <int NV> struct gate: public data::base
{
    SNEX_NODE(gate);        
    
    struct MetadataClass
    {
        SN_NODE_ID("gate");
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

    enum class GateState
    {
        HOLDING,
        MOVING
    };

    int handleModulation(double& value) { return 0; }
    template <typename T> void processFrame(T& data) {}
    void handleHiseEvent(HiseEvent& e) {}
    void setExternalData(const ExternalData& data, int index) {}
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate = static_cast<float>(specs.sampleRate);
        float rmsTimeMs = 5.0f;
        rmsCoefficient = 1.0f - std::exp(-1.0f / (rmsTimeMs * 0.001f * sampleRate));            
        alpha = std::pow(0.5, 1.0 / (rmsTimeMs * 0.001f * sampleRate));
        beta = 1.0f - alpha;
    }       

    void reset() 
    {        
        state = GateState::MOVING;
        level = 1e-10f;
        timeHeld = 0.0f;
        lastGainReductionDB = -60.0f; // start closed
    }
    
    template <typename T> void process(T& data)
    {
        const int numSamples = data.getNumSamples();
        
        dyn<float> ch0;
        dyn<float> ch1;

        int ci = 0;
        for (auto ch : data)
        {
            if (ci == 0) ch0 = data.toChannelData(ch);
            else if (ci == 1) ch1 = data.toChannelData(ch);
            ++ci;
        }
        
        if (ci < 2) ch1 = ch0; // safety mirror to avoid null

        const float dt = 1.0f / sampleRate;
        const float maxHold = 0.002f; // 2ms
        const float maxGainReduction = -60.0f;
        
        // Rate of change per sample
        const float dOpen = -maxGainReduction / (attack * 0.001f) * dt;
        const float dClose = maxGainReduction / (release * 0.001f) * dt;

        for (int s = 0; s < numSamples; ++s)
        {
            const float in0 = ch0[s];
            const float in1 = ch1[s];
            
            const float inputSquared = std::max(in0 * in0, in1 * in1);                        
            level = std::max(alpha * level + beta * inputSquared, 1e-10f);                    
            const float levelDB = 10.0f * std::log10(level);

            float gainReductionDB = 0.0f;

            if (state == GateState::HOLDING)
            {
                gainReductionDB = 0.0f;
                lastGainReductionDB = 0.0f;
                
                if (levelDB < threshold)
                {
                    timeHeld += dt;
                    if (timeHeld >= maxHold) { state = GateState::MOVING; }
                }

                else { timeHeld = 0.0f; }

            }
            else 
            {
                float targetGainReduction = 0.0f;
                if (levelDB < threshold) { targetGainReduction = maxGainReduction; }                
                if (targetGainReduction > lastGainReductionDB)
                {
                    // open
                    const float dGain = std::min(0.5f * (targetGainReduction - lastGainReductionDB), dOpen);
                    lastGainReductionDB += dGain;
                    if (lastGainReductionDB >= 0.0f)
                    {
                        lastGainReductionDB = 0.0f;
                        state = GateState::HOLDING;
                        timeHeld = 0.0f;
                    }
                }
                else if (targetGainReduction < lastGainReductionDB)
                {
                    // close
                    const float dGain = std::max(0.5f * (targetGainReduction - lastGainReductionDB), dClose);
                    lastGainReductionDB += dGain;
                    if (lastGainReductionDB < maxGainReduction) { lastGainReductionDB = maxGainReduction; }
                }
                gainReductionDB = lastGainReductionDB;
            }
            
            const float gain = std::pow(10.0f, gainReductionDB / 10.0f);
            
            ch0[s] = in0 * gain;
            ch1[s] = in1 * gain;
        }
    }
    
    template <int P> void setParameter(double v)
    {
        if (P == 0) { threshold = static_cast<float>(v); }
        if (P == 1) { attack = static_cast<float>(v); }
        if (P == 2) { release = static_cast<float>(v); }
    }
    
    void createParameters(ParameterDataList& data)
    {       
        {
            parameter::data threshold("Threshold", { -100.0, 0.0 });
            registerCallback<0>(threshold);
            threshold.setDefaultValue(-24.0);            
            data.add(std::move(threshold));
        }      
        {
            parameter::data attack("Attack", { 1.0, 100.0 });
            registerCallback<1>(attack);
            attack.setDefaultValue(5.0);            
            data.add(std::move(attack));
        }        
        {
            parameter::data release("Release", { 1.0, 200.0 });
            registerCallback<2>(release);
            release.setDefaultValue(15.0);            
            data.add(std::move(release));
        }          
    }

private:
    float sampleRate = 44100.0f;       
    
    GateState state = GateState::MOVING;
    float level = 1e-10f;
    float timeHeld = 0.0f;
    float lastGainReductionDB = -60.0f;

    float attack = 5.0f;
    float release = 15.0f;
    float threshold = -24.0f;    
    
    float alpha = 0.0f;
    float beta = 0.0f;
    float rmsCoefficient = 0.0f;
};
}