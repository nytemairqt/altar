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
#include "src/filters.h"

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;
using namespace altarFilters;

template <int NV> struct overdrive: public data::base
{
    SNEX_NODE(overdrive);        
    
    struct MetadataClass
    {
        SN_NODE_ID("overdrive");
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
        maxBlockSize = specs.blockSize;
        
        lastSpecs.sampleRate = specs.sampleRate;
        lastSpecs.numChannels = specs.numChannels;
        lastSpecs.maximumBlockSize = specs.blockSize;
        
        updateOversampling();
        
        driveSmoothed.reset(scaledSampleRate, 0.01f);
        driveSmoothed.setCurrentAndTargetValue(drive);
        outputSmoothed.reset(scaledSampleRate, 0.01f);
        outputSmoothed.setCurrentAndTargetValue(outputGain);        
        mixSmoothed.reset(scaledSampleRate, 0.01f);
        mixSmoothed.setCurrentAndTargetValue(mix);        
        circuitBendSmoothed.reset(scaledSampleRate, 0.02f);
        circuitBendSmoothed.setCurrentAndTargetValue(circuitBend);
        
        updateCircuitBendFrequency();        
        reset();        
    }           

    void reset() 
    {
        driveSmoothed.reset(scaledSampleRate, 0.01f);
        outputSmoothed.reset(scaledSampleRate, 0.01f);
        mixSmoothed.reset(scaledSampleRate, 0.01f);
        circuitBendSmoothed.reset(scaledSampleRate, 0.02f);
        
        for (int ch = 0; ch < 2; ++ch)
        {
            inputHPFStates[ch].reset();
            outputLPFStates[ch].reset();
            postHPFStates[ch].reset(); 

            screamerHPFStates[ch].reset();
            screamerPeakStates[ch].reset();
            screamerHighShelfStates[ch].reset();

            ratHPFStates[ch].reset();
            ratPeakStates[ch].reset();
            ratHighShelfStates[ch].reset();

            fuzzHPFStates[ch].reset();
            fuzzMidScoopStates[ch].reset();
            fuzzHighShelfStates[ch].reset();

            bendOscPhase[ch] = 0.0f;
        }
        
        for (int ch = 0; ch < 2; ++ch)
        {
            lastBitcrushedSample[ch] = 0.0f;
            bitcrusherCounter[ch] = 0;
            wavefolderState[ch] = 0.0f;
        }
        
        if (oversamplingFactor > 0 && oversampling != nullptr) { oversampling->reset(); }
    }
    
    template <typename T> void process(T& data)
    {
        int numSamples = data.getNumSamples();
        size_t numCh = juce::jmin(2, (int)data.getNumChannels());
        
        AudioBuffer<float> buffer(numCh, numSamples);
        
        int channelIndex = 0;
        for(auto ch: data)
        {
            if (channelIndex >= numCh) break;
            dyn<float> channel = data.toChannelData(ch);
            for (int s = 0; s < numSamples; ++s)
            {
                buffer.setSample(channelIndex, s, channel[s]);
            }
            channelIndex++;
        }
        
        processAudioBuffer(buffer);
        
        channelIndex = 0;
        for(auto ch: data)
        {
            if (channelIndex >= numCh) break;
            dyn<float> channel = data.toChannelData(ch);
            for (int s = 0; s < numSamples; ++s)
            {
                channel[s] = buffer.getSample(channelIndex, s);
            }
            channelIndex++;
        }
    }    
    
    template <int P> void setParameter(double v)
    {
        switch (P)
        {
        case 0: 
        {
            distortionMode = static_cast<int>(v);
            updateAllFilters(); 
            break;
        }
        case 1: 
        {
            int oldFactor = oversamplingFactor;
            oversamplingFactor = static_cast<int>(v);
            if (oldFactor != oversamplingFactor)
                updateOversampling();
            break;
        }
        case 2: drive = static_cast<float>(v); break;
        case 3: 
        {
            circuitBend = static_cast<float>(v); 
            updateCircuitBendFrequency(); 
            break;
        }
        case 4: 
        {
            circuitBendFreq = static_cast<float>(v); 
            updateCircuitBendFrequency(); 
            break;
        }
        case 5: outputGain = static_cast<float>(v); break;
        case 6: mix = static_cast<float>(v); break;
        case 7: bits = static_cast<float>(v); break;
        case 8: sampleRateReduction = static_cast<float>(v); break;
        case 9: foldAmount = static_cast<float>(v); break;
        case 10: circuitBendTrigger = static_cast<float>(v); break;
        case 11: 
        {
            tone = static_cast<float>(v);
            updateAllFilters(); 
            break;
        }
        }
    }
    
    void createParameters(ParameterDataList& data)
    {       
        {
            parameter::data mode("Mode", { 0.0, 5.0 });
            registerCallback<0>(mode);
            mode.setDefaultValue(0.0);
            data.add(std::move(mode));
        }        
        {
            parameter::data oversamp("Oversampling", { 0.0, 3.0 });
            registerCallback<1>(oversamp);
            oversamp.setDefaultValue(1.0);
            data.add(std::move(oversamp));
        }
        {
            parameter::data driveParam("Drive", { 0.1, 20.0 });
            registerCallback<2>(driveParam);
            driveParam.setDefaultValue(5.0);
            data.add(std::move(driveParam));
        }        
        {
            parameter::data bendParam("CircuitBend", { -48.0, 48.0 });
            registerCallback<3>(bendParam);
            bendParam.setDefaultValue(0.0);
            data.add(std::move(bendParam));
        }        
        {
            parameter::data bendFreqParam("CircuitBendFreq", { 400.0, 5000.0 });
            registerCallback<4>(bendFreqParam);
            bendFreqParam.setDefaultValue(400.0);
            data.add(std::move(bendFreqParam));
        }        
        {
            parameter::data outputParam("OutputGain", { -24.0, 24.0 });
            registerCallback<5>(outputParam);
            outputParam.setDefaultValue(0.0);
            data.add(std::move(outputParam));
        }        
        {
            parameter::data mixParam("Mix", { 0.0, 1.0 });
            registerCallback<6>(mixParam);
            mixParam.setDefaultValue(1.0);
            data.add(std::move(mixParam));
        }        
        {
            parameter::data bitsParam("Bits", { 1.0, 16.0 });
            registerCallback<7>(bitsParam);
            bitsParam.setDefaultValue(16.0);
            data.add(std::move(bitsParam));
        }        
        {
            parameter::data srReductionParam("SRReduction", { 1.0, 32.0 });
            registerCallback<8>(srReductionParam);
            srReductionParam.setDefaultValue(1.0);
            data.add(std::move(srReductionParam));
        }        
        {
            parameter::data foldParam("FoldAmount", { 1.0, 8.0 });
            registerCallback<9>(foldParam);
            foldParam.setDefaultValue(2.0);
            data.add(std::move(foldParam));
        }
        {
            parameter::data triggerParam("BendTrigger", { 0.0, 1.0 });
            registerCallback<10>(triggerParam);
            triggerParam.setDefaultValue(0.0);
            data.add(std::move(triggerParam));
        }
        {
            parameter::data toneParam("Tone", { -1.0, 1.0 });
            registerCallback<11>(toneParam);
            toneParam.setDefaultValue(0.0);
            data.add(std::move(toneParam));
        }
    }

private:
    float sampleRate = 44100.0f;
    int numChannels = 2;
    int maxBlockSize = 512;
    
    int distortionMode = 0; // 0 = screamer, 1 = fuzz, 2 = rat, 3 = wavefolder, 4 = bitcrusher, 5 = glitch
    int oversamplingFactor = 1;
    float drive = 5.0f;
    float circuitBend = 0.0f;
    float circuitBendFreq = 440.0f;
    float outputGain = 0.0f;
    float mix = 1.0f;
    float bits = 16.0f;
    float sampleRateReduction = 1.0f;
    float foldAmount = 2.0f;
    float circuitBendTrigger = 0.0f;
    float tone = 0.0f; 

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    float scaledSampleRate = 44100.0f;
    
    struct ProcessSpec
    {
        double sampleRate = 44100.0;
        uint32 numChannels = 2;
        uint32 maximumBlockSize = 512;
    };
    ProcessSpec lastSpecs;
    ProcessSpec specsOversampling;
    
    SmoothedValue<float> driveSmoothed;
    SmoothedValue<float> outputSmoothed;
    SmoothedValue<float> mixSmoothed;
    SmoothedValue<float> circuitBendSmoothed;
    
    BiquadState inputHPFStates[2];
    BiquadState outputLPFStates[2];
    BiquadState postHPFStates[2]; 
    
    BiquadState screamerHPFStates[2];       
    BiquadState screamerPeakStates[2];      
    BiquadState screamerHighShelfStates[2]; 

    BiquadState ratHPFStates[2];       
    BiquadState ratPeakStates[2];      
    BiquadState ratHighShelfStates[2]; 

    BiquadState fuzzHPFStates[2];       
    BiquadState fuzzMidScoopStates[2];  
    BiquadState fuzzHighShelfStates[2]; 
    
    float bendOscPhase[2] = { 0.0f, 0.0f };
    float bendOscFrequency = 440.0f;
    float bendOscPhaseIncrement = 0.0f;
    
    float lastBitcrushedSample[2] = { 0.0f, 0.0f };
    int bitcrusherCounter[2] = { 0, 0 };
    float wavefolderState[2] = { 0.0f, 0.0f };
    
    void processAudioBuffer(juce::AudioBuffer<float>& buffer)
    {
        juce::ScopedNoDenormals noDenormals;
        
        const bool useOversampling = (oversamplingFactor > 0 && oversampling != nullptr);
        juce::dsp::AudioBlock<float> inBlock(buffer);

        auto runDSP = [&](juce::AudioBuffer<float>& procBuf)
        {
            const int numCh = juce::jmin(2, procBuf.getNumChannels());
            const int procSamples = procBuf.getNumSamples();

            driveSmoothed.setTargetValue(juce::jlimit(0.1f, 20.0f, drive));
            outputSmoothed.setTargetValue(juce::jlimit(-24.0f, 24.0f, outputGain));
            mixSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, mix));
            circuitBendSmoothed.setTargetValue(juce::jlimit(-48.0f, 48.0f, circuitBend));

            for (int ch = 0; ch < numCh; ++ch)
            {
                float* channelData = procBuf.getWritePointer(ch);
                for (int s = 0; s < procSamples; ++s)
                    channelData[s] = inputHPFStates[ch].process(channelData[s]);
            }

            for (int s = 0; s < procSamples; ++s)
            {
                const float driveValue  = driveSmoothed.getNextValue();
                const float outputValue = outputSmoothed.getNextValue();
                const float mixValue    = mixSmoothed.getNextValue();
                const float bendValue   = circuitBendSmoothed.getNextValue();

                for (int ch = 0; ch < numCh; ++ch)
                {
                    const float inputSample = procBuf.getSample(ch, s);
                    const float drySignal   = inputSample;

                    float circuitBentSample = applyCircuitBend(inputSample, bendValue, ch);

                    float distortedSample = processDistortion(circuitBentSample, driveValue, ch);                                    
                    
                    distortedSample *= juce::Decibels::decibelsToGain(outputValue);

                    const float outputSample = drySignal * (1.0f - mixValue) + distortedSample * mixValue;
                    procBuf.setSample(ch, s, outputSample);
                }
            }

            for (int ch = 0; ch < numCh; ++ch)
            {
                float* channelData = procBuf.getWritePointer(ch);
                for (int s = 0; s < procSamples; ++s)
                {
                    float sample = channelData[s];
                    sample = outputLPFStates[ch].process(sample);
                    channelData[s] = sample;
                }
            }

            for (int ch = 0; ch < numCh; ++ch)
            {
                float* channelData = procBuf.getWritePointer(ch);
                for (int s = 0; s < procSamples; ++s)
                {
                    channelData[s] = postHPFStates[ch].process(channelData[s]);
                }
            }
        };

        if (useOversampling)
        {
            auto upBlock = oversampling->processSamplesUp(inBlock);

            const int upCh     = static_cast<int>(upBlock.getNumChannels());
            const int upFrames = static_cast<int>(upBlock.getNumSamples());

            juce::HeapBlock<float*> chanPtrs(upCh);
            for (int ch = 0; ch < upCh; ++ch)
                chanPtrs[ch] = upBlock.getChannelPointer(static_cast<size_t>(ch));

            juce::AudioBuffer<float> osBuffer(chanPtrs.getData(), upCh, upFrames);
            runDSP(osBuffer);
            oversampling->processSamplesDown(inBlock);
        }
        else
        {
            runDSP(buffer);
        }
    }
    
    float applyCircuitBend(float input, float bendValue, int channel)
    {
        if (circuitBendTrigger < 0.5f) { return input; }            

        float frequency = bendOscFrequency * std::pow(2.0f, bendValue / 12.0f);
        const float minFreq = juce::jmax(5.0f, bendOscFrequency * std::pow(2.0f, -4.0f));
        frequency = juce::jlimit(minFreq, 0.45f * scaledSampleRate, frequency);
        
        float phaseIncrement = 2.0f * juce::MathConstants<float>::pi * frequency / scaledSampleRate;
        
        bendOscPhase[channel] += phaseIncrement;
        if (bendOscPhase[channel] >= 2.0f * juce::MathConstants<float>::pi)
        {
            bendOscPhase[channel] -= 2.0f * juce::MathConstants<float>::pi;
        }
        
        float sineWave = std::sin(bendOscPhase[channel]);
        
        float triangleWave;
        float normalizedPhase = bendOscPhase[channel] / (2.0f * juce::MathConstants<float>::pi);
        if (normalizedPhase < 0.25f)
            triangleWave = 4.0f * normalizedPhase;
        else if (normalizedPhase < 0.75f)
            triangleWave = 2.0f - 4.0f * normalizedPhase;
        else
            triangleWave = 4.0f * normalizedPhase - 4.0f;
        
        float oscillatorOutput = 0.3f * sineWave + 0.7f * triangleWave;
        
        float baseBendAmount = 0.2f;
        float additionalBendAmount = juce::jlimit(0.0f, 0.8f, std::abs(bendValue) / 48.0f);
        float totalBendAmount = baseBendAmount + additionalBendAmount;
        
        oscillatorOutput *= totalBendAmount * 0.4f; 
        
        float baseMixRatio = 0.15f; 
        float additionalMixRatio = additionalBendAmount * 0.45f; 
        float totalMixRatio = baseMixRatio + additionalMixRatio; 
        
        float output = input * (1.0f - totalMixRatio) + oscillatorOutput * totalMixRatio;
                
        float modulatedOsc = oscillatorOutput * (1.0f + input * 0.3f);
        output = output * 0.8f + modulatedOsc * 0.2f * totalBendAmount;
        
        return juce::jlimit(-1.5f, 1.5f, output);
    }
    
    float processDistortion(float input, float driveValue, int channel)
    {
        switch (distortionMode)
        {
        case 0: return processScreamer(input, driveValue, channel);
        case 1: return processFuzz(input, driveValue, channel);        
        case 2: return processRat(input, driveValue, channel);
        case 3: return processWavefolder(input, channel);
        case 4: return processBitcrusher(input, channel);
        case 5: return processGlitch(input, channel);            
        default: return input;
        }
    }

    float processScreamer(float input, float driveValue, int channel)
    {
        float filtered = screamerHPFStates[channel].process(input);        
        filtered = screamerPeakStates[channel].process(filtered);
        
        float preGain = driveValue * 0.5f;
        float driven = filtered * preGain;
        float output = (2.0f / juce::MathConstants<float>::pi) * std::atan(driven * 2.0f);
        float harmonic = std::sin(output * juce::MathConstants<float>::pi * 2.0f) * 0.05f;
        output *= (1.0f + std::abs(output) * 0.1f);

        output = screamerHighShelfStates[channel].process(output);

        return juce::jlimit(-1.0f, 1.0f, output + harmonic);
    }

    float processFuzz(float input, float driveValue, int channel)
    {
        float filtered = fuzzHPFStates[channel].process(input);        
        filtered = fuzzMidScoopStates[channel].process(filtered);
        
        float driven = filtered * driveValue * 0.8f;
        if (driven > 0.0f)
            driven = std::tanh(driven * 1.5f);
        else
        {
            driven = -std::pow(std::abs(driven), 0.7f) * std::copysign(1.0f, driven);
            driven = juce::jlimit(-1.0f, 1.0f, driven * 1.2f);
        }

        float harmonic = std::sin(driven * juce::MathConstants<float>::pi) * 0.1f;
        float output = juce::jlimit(-1.0f, 1.0f, driven + harmonic);

        output = fuzzHighShelfStates[channel].process(output);

        return output;
    }

    float processRat(float input, float driveValue, int channel)
    {
        float filtered = ratHPFStates[channel].process(input);        
        filtered = ratPeakStates[channel].process(filtered);
        
        float driven = filtered * driveValue * 0.6f;
        if (std::abs(driven) > 0.7f)
        {
            float sign = driven > 0.0f ? 1.0f : -1.0f;
            float excess = std::abs(driven) - 0.7f;
            driven = sign * (0.7f + std::tanh(excess * 3.0f) * 0.3f);
        }

        float harmonic = std::sin(driven * juce::MathConstants<float>::pi * 3.0f) * 0.08f;
        float output = juce::jlimit(-1.0f, 1.0f, driven + harmonic);

        output = ratHighShelfStates[channel].process(output);

        return output;
    }    

    float processBitcrusher(float input, int channel)
    {
        float bitsValue = juce::jlimit(1.0f, 16.0f, bits);
        float reductionFactor = juce::jlimit(1.0f, 32.0f, sampleRateReduction);

        if (++bitcrusherCounter[channel] >= static_cast<int>(reductionFactor))
        {
            bitcrusherCounter[channel] = 0;
            
            float levels = std::pow(2.0f, bitsValue) - 1.0f;
            float quantized = std::round(input * levels) / levels;

            lastBitcrushedSample[channel] = quantized;
        }

        return lastBitcrushedSample[channel];
    }

    float processGlitch(float input, int channel)
    {
        float reductionFactor = juce::jlimit(1.0f, 64.0f, sampleRateReduction * 2.0f);

        if (++bitcrusherCounter[channel] >= static_cast<int>(reductionFactor))
        {
            bitcrusherCounter[channel] = 0;

            float bitsValue = juce::jlimit(1.0f, 8.0f, bits * 0.5f);
            float levels = std::pow(2.0f, bitsValue) - 1.0f;

            float quantized = std::round(input * levels) / levels;

            if (juce::Random::getSystemRandom().nextFloat() < 0.1f)
            {
                quantized *= -1.0f; 
            }

            if (juce::Random::getSystemRandom().nextFloat() < 0.05f)
            {
                quantized = 0.0f; 
            }

            lastBitcrushedSample[channel] = quantized;
        }

        lastBitcrushedSample[channel] *= 0.8f;
        return lastBitcrushedSample[channel];
    }

    float processWavefolder(float input, int channel)
    {
        float foldValue = juce::jlimit(1.0f, 8.0f, foldAmount);
        float driven = input * foldValue;
        float folded = driven;

        while (std::abs(folded) > 1.0f)
        {
            if (folded > 1.0f)
                folded = 2.0f - folded;
            else if (folded < -1.0f)
                folded = -2.0f - folded;
        }
        
        wavefolderState[channel] = wavefolderState[channel] * 0.9f + folded * 0.1f;
        float output = folded * 0.8f + wavefolderState[channel] * 0.2f;

        return juce::jlimit(-1.0f, 1.0f, output);
    }
    
    void updateScreamerFilters()
    {                
        float peakGain = tone * 12.0f;        
        float highShelfGain = tone * 8.0f;
        makeHighPassStereo(screamerHPFStates[0], screamerHPFStates[1], 80.0f, scaledSampleRate);
        makePeakFilterStereo(screamerPeakStates[0], screamerPeakStates[1], 2500.0f, 0.5f, peakGain, scaledSampleRate);        
        makeHighShelfStereo(screamerHighShelfStates[0], screamerHighShelfStates[1], 8000.0f, 0.5f, highShelfGain, scaledSampleRate);
    }

    void updateRatFilters()
    {
        float midPeakGain = tone * 8.0f;
        makeHighPassStereo(ratHPFStates[0], ratHPFStates[1], 80.0f, scaledSampleRate);                
        makePeakFilterStereo(ratPeakStates[0], ratPeakStates[1], 1500.0f, 0.7f, midPeakGain, scaledSampleRate);        
        makeHighShelfStereo(ratHighShelfStates[0], ratHighShelfStates[1], 6000.0f, 0.7f, -3.0f, scaledSampleRate);
    }

    void updateFuzzFilters()
    {
        float midScoopGain = -tone * 6.0f; 
        float highShelfGain = tone * 6.0f;
        makeHighPassStereo(fuzzHPFStates[0], fuzzHPFStates[1], 30.0f, scaledSampleRate);                
        makePeakFilterStereo(fuzzMidScoopStates[0], fuzzMidScoopStates[1], 800.0f, 0.6f, midScoopGain, scaledSampleRate);                
        makeHighShelfStereo(fuzzHighShelfStates[0], fuzzHighShelfStates[1], 10000.0f, 0.5f, highShelfGain, scaledSampleRate);
    }

    void updateAllFilters()
    {        
        updateScreamerFilters();
        updateRatFilters();
        updateFuzzFilters();
    }
    
    void updateOversampling()
    {
        if (lastSpecs.sampleRate <= 0) return;
        
        std::unique_ptr<juce::dsp::Oversampling<float>> newOversampling;
        
        if (oversamplingFactor > 0)
        {
            newOversampling = std::make_unique<juce::dsp::Oversampling<float>>(
                static_cast<int>(lastSpecs.numChannels),
                oversamplingFactor,
                juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
                false
            );
            
            newOversampling->initProcessing(lastSpecs.maximumBlockSize);
            newOversampling->reset();
            
            specsOversampling.numChannels = lastSpecs.numChannels;
            specsOversampling.sampleRate = lastSpecs.sampleRate * (1 << oversamplingFactor);
            specsOversampling.maximumBlockSize = lastSpecs.maximumBlockSize * (1 << oversamplingFactor);
            
            scaledSampleRate = specsOversampling.sampleRate;
        }
        else
        {
            scaledSampleRate = lastSpecs.sampleRate;
        }
        
        oversampling.swap(newOversampling);
        updateCircuitBendFrequency();
        updateFilterCoefficients();
    }
    
    void updateFilterCoefficients()
    {
        makeHighPassStereo(inputHPFStates[0], inputHPFStates[1], 20.0f, scaledSampleRate);        
        makeLowPassStereo(outputLPFStates[0], outputLPFStates[1], 12000.0f, scaledSampleRate);
        makeHighPassStereo(postHPFStates[0], postHPFStates[1], 50.0f, scaledSampleRate);        
        updateAllFilters();
    }
    
    void updateCircuitBendFrequency()
    {
        bendOscFrequency = juce::jlimit(5.0f, 5000.0f, circuitBendFreq);        
        bendOscPhaseIncrement = 2.0f * juce::MathConstants<float>::pi * bendOscFrequency / scaledSampleRate;
    }
};
}