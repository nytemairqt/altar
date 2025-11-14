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

template <int NV> struct preprocess: public data::base
{
    SNEX_NODE(preprocess);        
    
    struct MetadataClass
    {
        SN_NODE_ID("preprocess");
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
        
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < 9; ++i)
            {
                eqStates[ch][i].reset();
            }
        }
        
        gainReductionSmoothers.resize(numChannels);
        rmsLevels.resize(numChannels, 0.0f);
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            gainReductionSmoothers[ch].reset(sampleRate, 0.005f);
            gainReductionSmoothers[ch].setCurrentAndTargetValue(0.0f);
        }
        
        rmsCoefficient = 1.0f - std::exp(-1.0f / (10.0f * 0.001f * sampleRate));
        updateTimingCoefficients();
        updateEQCoefficients();
    }       

    void reset() 
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < 9; ++i)
            {
                eqStates[ch][i].reset();
            }
        }
        
        std::fill(rmsLevels.begin(), rmsLevels.end(), 0.0f);
        for (auto& smoother : gainReductionSmoothers)
        {
            smoother.reset(sampleRate, 0.005f);
            smoother.setCurrentAndTargetValue(0.0f);
        }
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
                float sampleData = channel[s];
                
                if (eqFirst)
                {
                    if (eqEnable) { sampleData = processEQ(sampleData, channelIndex); }
                    if (compEnable) { sampleData = processComp(sampleData, channelIndex, s); }
                }
                else
                {
                    if (compEnable) { sampleData = processComp(sampleData, channelIndex, s); }
                    if (eqEnable) { sampleData = processEQ(sampleData, channelIndex); }
                }
                
                if (clipperEnable) { sampleData = processClipper(sampleData); }
                
                channel[s] = sampleData;
            }
            channelIndex++;
        }
    }
    
    template <int P> void setParameter(double v)
    {
        switch (P)
        {
        case 0: hpfFrequency = static_cast<float>(v); updateEQCoefficients(); break;
        case 1: lowShelfFrequency = static_cast<float>(v); updateEQCoefficients(); break;
        case 2: lowShelfGain = static_cast<float>(v); updateEQCoefficients(); break;
        case 3: lowShelfQ = static_cast<float>(v); updateEQCoefficients(); break;
        case 4: lowMidFrequency = static_cast<float>(v); updateEQCoefficients(); break;
        case 5: lowMidGain = static_cast<float>(v); updateEQCoefficients(); break;
        case 6: lowMidQ = static_cast<float>(v); updateEQCoefficients(); break;
        case 7: midFrequency = static_cast<float>(v); updateEQCoefficients(); break;
        case 8: midGain = static_cast<float>(v); updateEQCoefficients(); break;
        case 9: midQ = static_cast<float>(v); updateEQCoefficients(); break;
        case 10: highMidFrequency = static_cast<float>(v); updateEQCoefficients(); break;
        case 11: highMidGain = static_cast<float>(v); updateEQCoefficients(); break;
        case 12: highMidQ = static_cast<float>(v); updateEQCoefficients(); break;
        case 13: highShelfFrequency = static_cast<float>(v); updateEQCoefficients(); break;
        case 14: highShelfGain = static_cast<float>(v); updateEQCoefficients(); break;
        case 15: highShelfQ = static_cast<float>(v); updateEQCoefficients(); break;
        case 16: lpfFrequency = static_cast<float>(v); updateEQCoefficients(); break;
        
        case 17: compThreshold = static_cast<float>(v); break;
        case 18: compRatio = static_cast<float>(v); break;
        case 19: compAttack = static_cast<float>(v); updateTimingCoefficients(); break;
        case 20: compRelease = static_cast<float>(v); updateTimingCoefficients(); break;
        case 21: compKnee = static_cast<float>(v); break;
        case 22: compMix = static_cast<float>(v); break;
        case 23: compMakeupGain = static_cast<float>(v); break;
        
        case 24: clipperInputGain = static_cast<float>(v); break;
        
        case 25: eqFirst = v > 0.5; break;

        case 26: eqEnable = v > 0.5; break;
        case 27: compEnable = v > 0.5; break;
        case 28: clipperEnable = v > 0.5; break;
        }
    }
    
    void createParameters(ParameterDataList& data)
    {       
        {
            parameter::data hpfFreq("HPF Freq", { 20.0, 200.0 });
            registerCallback<0>(hpfFreq);
            hpfFreq.setDefaultValue(40.0);
            data.add(std::move(hpfFreq));
        }
        {
            parameter::data lowShelfFreq("Low Shelf Freq", { 50.0, 500.0 });
            registerCallback<1>(lowShelfFreq);
            lowShelfFreq.setDefaultValue(120.0);
            data.add(std::move(lowShelfFreq));
        }
        {
            parameter::data lowShelfGainParam("Low Shelf Gain", { -15.0, 15.0 });
            registerCallback<2>(lowShelfGainParam);
            lowShelfGainParam.setDefaultValue(0.0);
            data.add(std::move(lowShelfGainParam));
        }
        {
            parameter::data lowShelfQParam("Low Shelf Q", { 0.1, 2.0 });
            registerCallback<3>(lowShelfQParam);
            lowShelfQParam.setDefaultValue(0.7);
            data.add(std::move(lowShelfQParam));
        }
        {
            parameter::data lowMidFreq("Low Mid Freq", { 100.0, 1000.0 });
            registerCallback<4>(lowMidFreq);
            lowMidFreq.setDefaultValue(300.0);
            data.add(std::move(lowMidFreq));
        }
        {
            parameter::data lowMidGainParam("Low Mid Gain", { -15.0, 15.0 });
            registerCallback<5>(lowMidGainParam);
            lowMidGainParam.setDefaultValue(0.0);
            data.add(std::move(lowMidGainParam));
        }
        {
            parameter::data lowMidQParam("Low Mid Q", { 0.1, 5.0 });
            registerCallback<6>(lowMidQParam);
            lowMidQParam.setDefaultValue(1.0);
            data.add(std::move(lowMidQParam));
        }
        {
            parameter::data midFreq("Mid Freq", { 200.0, 2000.0 });
            registerCallback<7>(midFreq);
            midFreq.setDefaultValue(700.0);
            data.add(std::move(midFreq));
        }
        {
            parameter::data midGainParam("Mid Gain", { -15.0, 15.0 });
            registerCallback<8>(midGainParam);
            midGainParam.setDefaultValue(0.0);
            data.add(std::move(midGainParam));
        }
        {
            parameter::data midQParam("Mid Q", { 0.1, 5.0 });
            registerCallback<9>(midQParam);
            midQParam.setDefaultValue(1.0);
            data.add(std::move(midQParam));
        }
        {
            parameter::data highMidFreq("High Mid Freq", { 1000.0, 10000.0 });
            registerCallback<10>(highMidFreq);
            highMidFreq.setDefaultValue(3200.0);
            data.add(std::move(highMidFreq));
        }
        {
            parameter::data highMidGainParam("High Mid Gain", { -15.0, 15.0 });
            registerCallback<11>(highMidGainParam);
            highMidGainParam.setDefaultValue(0.0);
            data.add(std::move(highMidGainParam));
        }
        {
            parameter::data highMidQParam("High Mid Q", { 0.1, 5.0 });
            registerCallback<12>(highMidQParam);
            highMidQParam.setDefaultValue(1.0);
            data.add(std::move(highMidQParam));
        }
        {
            parameter::data highShelfFreq("High Shelf Freq", { 2000.0, 20000.0 });
            registerCallback<13>(highShelfFreq);
            highShelfFreq.setDefaultValue(7000.0);
            data.add(std::move(highShelfFreq));
        }
        {
            parameter::data highShelfGainParam("High Shelf Gain", { -15.0, 15.0 });
            registerCallback<14>(highShelfGainParam);
            highShelfGainParam.setDefaultValue(0.0);
            data.add(std::move(highShelfGainParam));
        }
        {
            parameter::data highShelfQParam("High Shelf Q", { 0.1, 2.0 });
            registerCallback<15>(highShelfQParam);
            highShelfQParam.setDefaultValue(0.7);
            data.add(std::move(highShelfQParam));
        }
        {
            parameter::data lpfFreq("LPF Freq", { 5000.0, 20000.0 });
            registerCallback<16>(lpfFreq);
            lpfFreq.setDefaultValue(14000.0);
            data.add(std::move(lpfFreq));
        }
        {
            parameter::data threshold("Comp Threshold", { -60.0, 0.0 });
            registerCallback<17>(threshold);
            threshold.setDefaultValue(-18.0);
            data.add(std::move(threshold));
        }
        {
            parameter::data ratio("Comp Ratio", { 1.0, 20.0 });
            registerCallback<18>(ratio);
            ratio.setDefaultValue(4.0);
            data.add(std::move(ratio));
        }
        {
            parameter::data attack("Comp Attack", { 0.1, 100.0 });
            registerCallback<19>(attack);
            attack.setDefaultValue(2.0);
            data.add(std::move(attack));
        }
        {
            parameter::data release("Comp Release", { 10.0, 1000.0 });
            registerCallback<20>(release);
            release.setDefaultValue(150.0);
            data.add(std::move(release));
        }
        {
            parameter::data knee("Comp Knee", { 0.0, 10.0 });
            registerCallback<21>(knee);
            knee.setDefaultValue(3.0);
            data.add(std::move(knee));
        }
        {
            parameter::data mix("Comp Mix", { 0.0, 1.0 });
            registerCallback<22>(mix);
            mix.setDefaultValue(1.0);
            data.add(std::move(mix));
        }
        {
            parameter::data makeupGain("Comp Makeup", { -20.0, 20.0 });
            registerCallback<23>(makeupGain);
            makeupGain.setDefaultValue(0.0);
            data.add(std::move(makeupGain));
        }
        {
            parameter::data inputGain("Clipper Gain", { 0.0, 2.0 });
            registerCallback<24>(inputGain);
            inputGain.setDefaultValue(0.0);
            data.add(std::move(inputGain));
        }
        {
            parameter::data processingOrder("EQ First", { 0.0, 1.0 });
            registerCallback<25>(processingOrder);
            processingOrder.setDefaultValue(1.0);
            data.add(std::move(processingOrder));
        }
        {
            parameter::data processingOrder("EQ Enable", { 0.0, 1.0 });
            registerCallback<26>(processingOrder);
            processingOrder.setDefaultValue(1.0);
            data.add(std::move(processingOrder));
        }
        {
            parameter::data processingOrder("Comp Enable", { 0.0, 1.0 });
            registerCallback<27>(processingOrder);
            processingOrder.setDefaultValue(1.0);
            data.add(std::move(processingOrder));
        }
        {
            parameter::data processingOrder("Clipper Enable", { 0.0, 1.0 });
            registerCallback<28>(processingOrder);
            processingOrder.setDefaultValue(1.0);
            data.add(std::move(processingOrder));
        }
    }

private:
    float sampleRate = 44100.0f;
    int numChannels = 2;
    bool eqFirst = true;

    bool eqEnable = true;
    bool compEnable = true;
    bool clipperEnable = true;
    
    float hpfFrequency = 40.0f;
    float lowShelfFrequency = 120.0f;
    float lowShelfGain = 0.0f;
    float lowShelfQ = 0.7f;
    float lowMidFrequency = 300.0f;
    float lowMidGain = 0.0f;
    float lowMidQ = 1.0f;
    float midFrequency = 700.0f;
    float midGain = 0.0f;
    float midQ = 1.0f;
    float highMidFrequency = 3200.0f;
    float highMidGain = 0.0f;
    float highMidQ = 1.0f;
    float highShelfFrequency = 7000.0f;
    float highShelfGain = 0.0f;
    float highShelfQ = 0.7f;
    float lpfFrequency = 14000.0f;
    
    float compThreshold = -18.0f;
    float compRatio = 4.0f;
    float compAttack = 2.0f;
    float compRelease = 150.0f;
    float compKnee = 3.0f;
    float compMix = 1.0f;
    float compMakeupGain = 0.0f;
    
    float clipperInputGain = 0.0f;
    
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
            if (!std::isfinite(input)) { input = 0.0f; }
            float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            
            x2 = x1;
            x1 = input;
            y2 = y1;
            y1 = output;
            
            return output;
        }
    };
    
    BiquadState eqStates[2][9]; // 2 channels, 9 filters
    
    std::vector<float> rmsLevels;
    std::vector<juce::SmoothedValue<float>> gainReductionSmoothers;
    float rmsCoefficient = 0.0f;
    float attackCoefficient = 0.0f;
    float releaseCoefficient = 0.0f;

    float processEQ(float input, int channel)
    {
        float output = input;
        
        for (int i = 0; i < 9; ++i) { output = eqStates[channel][i].process(output); }        
        
        return output;
    }

    float processComp(float input, int channel, int sampleIndex)
    {
        if (!std::isfinite(input)) { input = 0.0f; }
        
        const float inputSquared = input * input;
        rmsLevels[channel] = rmsLevels[channel] * (1.0f - rmsCoefficient) + inputSquared * rmsCoefficient;
        const float rmsLevel = std::sqrt(rmsLevels[channel] + 1e-12f);
        const float inputDB = juce::Decibels::gainToDecibels(rmsLevel);
        const float targetGainReductionDB = calculateGainReduction(inputDB);
        
        gainReductionSmoothers[channel].setTargetValue(targetGainReductionDB);
        const float smoothedGainReductionDB = gainReductionSmoothers[channel].getNextValue();
        const float totalGainLinear = juce::Decibels::decibelsToGain(smoothedGainReductionDB + compMakeupGain);
        const float compressedSample = input * totalGainLinear;
        
        return input * (1.0f - compMix) + compressedSample * compMix;
    }

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
    
    void updateTimingCoefficients()
    {
        float attackTime = compAttack * 0.001f;
        float releaseTime = compRelease * 0.001f;

        attackCoefficient = 1.0f - std::exp(-1.0f / (attackTime * sampleRate));
        releaseCoefficient = 1.0f - std::exp(-1.0f / (releaseTime * sampleRate));
    }
    
    float calculateGainReduction(float inputDB)
    {
        float thresholdDB = compThreshold;
        float ratioValue = compRatio;
        float kneeDB = compKnee;

        float kneeStart = thresholdDB - kneeDB * 0.5f;
        float kneeEnd = thresholdDB + kneeDB * 0.5f;

        float outputDB = inputDB;

        if (inputDB <= kneeStart)
        {
            outputDB = inputDB;
        }
        else if (inputDB >= kneeEnd)
        {
            float overDB = inputDB - thresholdDB;
            outputDB = thresholdDB + (overDB / ratioValue);
        }
        else
        {
            float kneeRatio = (inputDB - kneeStart) / kneeDB;
            float curve = kneeRatio * kneeRatio * (3.0f - 2.0f * kneeRatio);
            float linearOutput = inputDB;
            float overdB = inputDB - thresholdDB;
            float compressedOutput = thresholdDB + (overdB / ratioValue);
            outputDB = linearOutput + curve * (compressedOutput - linearOutput);            
        }

        float gainReduction = outputDB - inputDB;
        return gainReduction;
    }
    
    void updateEQCoefficients()
    {
        for (int ch = 0; ch < 2; ++ch) // force stereo
        {   
            calculateHighpassCoeffs(hpfFrequency, eqStates[0][0], eqStates[1][0]);
            calculateHighpassCoeffs(hpfFrequency, eqStates[0][1], eqStates[1][1]);            
            calculateLowShelfCoeffs(lowShelfFrequency, lowShelfQ, lowShelfGain, eqStates[0][2], eqStates[1][2]);        
            calculatePeakCoeffs(lowMidFrequency, lowMidQ, lowMidGain, eqStates[0][3], eqStates[1][3]);            
            calculatePeakCoeffs(midFrequency, midQ, midGain, eqStates[0][4], eqStates[1][4]);            
            calculatePeakCoeffs(highMidFrequency, highMidQ, highMidGain, eqStates[0][5], eqStates[1][5]);            
            calculateHighShelfCoeffs(highShelfFrequency, highShelfQ, highShelfGain, eqStates[0][6], eqStates[1][6]);            
            calculateLowpassCoeffs(lpfFrequency, eqStates[0][7], eqStates[1][7]);
            calculateLowpassCoeffs(lpfFrequency, eqStates[0][8], eqStates[1][8]);
        }
    }
    
    void calculateHighpassCoeffs(float freq, BiquadState& state0, BiquadState& state1)
    {
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
        
        state0.b0 = state1.b0 = b0 / a0;
        state0.b1 = state1.b1 = b1 / a0;
        state0.b2 = state1.b2 = b2 / a0;
        state0.a1 = state1.a1 = a1 / a0;
        state0.a2 = state1.a2 = a2 / a0;
    }
    
    void calculateLowpassCoeffs(float freq, BiquadState& state0, BiquadState& state1)
    {
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
        
        state0.b0 = state1.b0 = b0 / a0;
        state0.b1 = state1.b1 = b1 / a0;
        state0.b2 = state1.b2 = b2 / a0;
        state0.a1 = state1.a1 = a1 / a0;
        state0.a2 = state1.a2 = a2 / a0;
    }
    
    void calculateLowShelfCoeffs(float freq, float Q, float gainDB, BiquadState& state0, BiquadState& state1)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float A = std::pow(10.0f, gainDB / 40.0f);
        float S = 1.0f;
        float beta = std::sqrt(A) / Q;
        
        float b0 = A * ((A + 1.0f) - (A - 1.0f) * cosOmega + beta * sinOmega);
        float b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosOmega);
        float b2 = A * ((A + 1.0f) - (A - 1.0f) * cosOmega - beta * sinOmega);
        float a0 = (A + 1.0f) + (A - 1.0f) * cosOmega + beta * sinOmega;
        float a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosOmega);
        float a2 = (A + 1.0f) + (A - 1.0f) * cosOmega - beta * sinOmega;
        
        state0.b0 = state1.b0 = b0 / a0;
        state0.b1 = state1.b1 = b1 / a0;
        state0.b2 = state1.b2 = b2 / a0;
        state0.a1 = state1.a1 = a1 / a0;
        state0.a2 = state1.a2 = a2 / a0;
    }
    
    void calculateHighShelfCoeffs(float freq, float Q, float gainDB, BiquadState& state0, BiquadState& state1)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float A = std::pow(10.0f, gainDB / 40.0f);
        float S = 1.0f;
        float beta = std::sqrt(A) / Q;
        
        float b0 = A * ((A + 1.0f) + (A - 1.0f) * cosOmega + beta * sinOmega);
        float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosOmega);
        float b2 = A * ((A + 1.0f) + (A - 1.0f) * cosOmega - beta * sinOmega);
        float a0 = (A + 1.0f) - (A - 1.0f) * cosOmega + beta * sinOmega;
        float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosOmega);
        float a2 = (A + 1.0f) - (A - 1.0f) * cosOmega - beta * sinOmega;
        
        state0.b0 = state1.b0 = b0 / a0;
        state0.b1 = state1.b1 = b1 / a0;
        state0.b2 = state1.b2 = b2 / a0;
        state0.a1 = state1.a1 = a1 / a0;
        state0.a2 = state1.a2 = a2 / a0;
    }
    
    void calculatePeakCoeffs(float freq, float Q, float gainDB, BiquadState& state0, BiquadState& state1)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float A = std::pow(10.0f, gainDB / 40.0f);
        float alpha = sinOmega / (2.0f * Q);
        
        float b0 = 1.0f + alpha * A;
        float b1 = -2.0f * cosOmega;
        float b2 = 1.0f - alpha * A;
        float a0 = 1.0f + alpha / A;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha / A;
        
        state0.b0 = state1.b0 = b0 / a0;
        state0.b1 = state1.b1 = b1 / a0;
        state0.b2 = state1.b2 = b2 / a0;
        state0.a1 = state1.a1 = a1 / a0;
        state0.a2 = state1.a2 = a2 / a0;
    }
};
}