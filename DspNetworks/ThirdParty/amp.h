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

// xsimd best simd
#define RTNEURAL_USE_EIGEN 0
#define RTNEURAL_USE_XSIMD 1
#define RTNEURAL_DEFAULT_ALIGNMENT 16

#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <cmath>
#include <algorithm>

#include "src/dependencies/nlohmann/json.hpp"
#include "src/dependencies/RTNeural/RTNeural.h"
#include "src/dependencies/RTNeural-NAM/wavenet/wavenet_model.hpp"
#include "src/dependencies/math_approx/math_approx.hpp"
#include "src/dependencies/xsimd/xsimd.hpp"
#include "src/filters.h"

enum class GlobalCablesAmp
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
using namespace altarFilters;
using cable_manager_t = routing::global_cable_cpp_manager<SN_GLOBAL_CABLE(110245659),
                                                          SN_GLOBAL_CABLE(106677056),
                                                          SN_GLOBAL_CABLE(108826)>;

template <int NV> struct amp: public data::base, public cable_manager_t
{
    SNEX_NODE(amp);    
    struct MetadataClass { SN_NODE_ID("amp"); };    
    
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

    amp()
    {        
        this->registerDataCallback<GlobalCablesAmp::nam>([this](const var& data)
        {
            this->loadNAMModel(data);
        });
    }
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate   = static_cast<float>(specs.sampleRate);
        numChannels  = specs.numChannels;
        maxBlockSize = specs.blockSize;
                
        lastSpecs.sampleRate        = specs.sampleRate;
        lastSpecs.numChannels       = specs.numChannels;
        lastSpecs.maximumBlockSize  = specs.blockSize;
                
        updateOversampling();
        
        resamplerConfig.store(true, std::memory_order_release);
        updateNAMResamplers();
        
        // prepare nam model        
        if (auto m = std::atomic_load(&modelShared))
        {
            m->prepare(static_cast<int>(lastSpecs.maximumBlockSize));            
            warmupNAM(*m);
            needsWarmup.store(false, std::memory_order_release);
            modelLoaded = true;
        }                
        
        reset();        
    }           

    void reset() 
    {
        // filter chains
        for (int ch = 0; ch < 2; ++ch)
        {            
            for (int i = 0; i < numDCOffsetStates; ++i) { dcOffset[ch][i].reset(); }
            for (int i = 0; i < numToneStackStates; ++i) { toneStackStates[ch][i].reset(); }

            // clean

            for (int i = 0; i < numGlassPreSculptStates; ++i) { glassPreSculptStates[ch][i].reset(); } 
            for (int i = 0; i < numGlassPostSculptStates; ++i) { glassPostSculptStates[ch][i].reset(); }

            for (int i = 0; i < numPearlPreSculptStates; ++i) { pearlPreSculptStates[ch][i].reset(); } 
            for (int i = 0; i < numPearlPostSculptStates; ++i) { pearlPostSculptStates[ch][i].reset(); }

            for (int i = 0; i < numDiamondPreSculptStates; ++i) { diamondPreSculptStates[ch][i].reset(); } 
            for (int i = 0; i < numDiamondPostSculptStates; ++i) { diamondPostSculptStates[ch][i].reset(); }

            for (int i = 0; i < numZirconPreSculptStates; ++i) { zirconPreSculptStates[ch][i].reset(); } 
            for (int i = 0; i < numZirconPostSculptStates; ++i) { zirconPostSculptStates[ch][i].reset(); }

            // dirty
            
            for (int i = 0; i < numSlatePreSculptStates; ++i) { slatePreSculptStates[ch][i].reset(); }
            for (int i = 0; i < numSlatePostSculptStates; ++i) { slatePostSculptStates[ch][i].reset(); }

            for (int i = 0; i < numObsidianPreSculptStates; ++i) {obsidianPreSculptStates[ch][i].reset(); }
            for (int i = 0; i < numObsidianPostSculptStates; ++i) { obsidianPostSculptStates[ch][i].reset(); }

            for (int i = 0; i < numAmethystPreSculptStates; ++i) { amethystPreSculptStates[ch][i].reset(); }
            for (int i = 0; i < numAmethystPostSculptStates; ++i) { amethystPostSculptStates[ch][i].reset(); }

            for (int i = 0; i < numOpalPreSculptStates; ++i) { opalPreSculptStates[ch][i].reset(); }
            for (int i = 0; i < numOpalPostSculptStates; ++i) { opalPostSculptStates[ch][i].reset(); }

            for (int i = 0; i < numSapphirePreSculptStates; ++i) { sapphirePreSculptStates[ch][i].reset(); }
            for (int i = 0; i < numSapphirePostSculptStates; ++i) { sapphirePostSculptStates[ch][i].reset(); }

            for (int i = 0; i < numGarnetPreSculptStates; ++i) { garnetPreSculptStates[ch][i].reset(); }
            for (int i = 0; i < numGarnetPostSculptStates; ++i) { garnetPostSculptStates[ch][i].reset(); }                    
        }
                
        if (oversamplingFactor > 0 && oversampling != nullptr) { oversampling->reset(); }
        for (int ch = 0; ch < 2; ++ch) { dirtyLowBandMem[ch] = 0.0f; dirtyPresenceMem[ch] = 0.0f; }
        
        inputResampler.resetState();
        outputResampler.resetState();                
        upsampleAAFilter.reset();
    }
    
    template <typename T> void process(T& data)
    {
        int numSamples = data.getNumSamples();
        size_t numCh = juce::jmin(2, (int)data.getNumChannels());

        AudioBuffer<float> buffer((int)numCh, numSamples); // FIX ME: use SNEX audio buffer         
        int channelIndex = 0;
        for(auto ch: data)
        {   
            // FIX ME: weird logic, I think I can just use ch
            if (channelIndex >= (int)numCh) break;
            dyn<float> channel = data.toChannelData(ch);
            for (int s = 0; s < numSamples; ++s)
            {
                buffer.setSample(channelIndex, s, channel[s]);
            }
            channelIndex++;
        }
                
        processAudioBuffer(buffer);
        
        // write back to data // FIX ME: make inline by using SNEX buffer
        channelIndex = 0;
        for(auto ch: data)
        {
            if (channelIndex >= (int)numCh) break;
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
        case 0: ampMode = static_cast<int>(v); break; // clean | dirty | nam
        case 1: 
        {
            int oldFactor = oversamplingFactor;
            oversamplingFactor = static_cast<int>(v);
            if (oldFactor != oversamplingFactor)
                updateOversampling();
            break;
        }        
        case 2: inputGain = static_cast<float>(v); break;
        case 3: low = static_cast<float>(v); updateToneStack(); break;
        case 4: mid = static_cast<float>(v); updateToneStack(); break;
        case 5: high = static_cast<float>(v); updateToneStack(); break;
        case 6: presence = static_cast<float>(v); updateToneStack(); break;
        case 7: outputGain = static_cast<float>(v); break;           
        case 8: distortionMode = static_cast<int>(v); break;
        case 9: cleanToneStackMode = static_cast<int>(v); break;  
        case 10: dirtyToneStackMode = static_cast<int>(v); break;  
        case 11: latestFilterFreq = static_cast<float>(v); updateSlate(); break;
        case 12: latestFilterQ = static_cast<float>(v); updateSlate(); break;
        case 13: latestFilterGain = static_cast<float>(v); updateSlate(); break;
        }
    }
    
    void createParameters(ParameterDataList& data)
    {               
        { 
            parameter::data mode("Mode", { 0.0, 2.0 });                   
            registerCallback<0>(mode);          
            mode.setDefaultValue(1.0);          
            data.add(std::move(mode)); 
        }            
        { 
            parameter::data oversamp("Oversampling", { 0.0, 3.0 });       
            registerCallback<1>(oversamp);      
            oversamp.setDefaultValue(1.0);      
            data.add(std::move(oversamp)); 
        }        
        { 
            parameter::data inputGain("InputGain", { -100.0, 100.0 });    
            registerCallback<2>(inputGain);     
            inputGain.setDefaultValue(0.0);     
            data.add(std::move(inputGain)); 
        }
        { 
            parameter::data low("Low", { -12.0, 12.0 });                  
            registerCallback<3>(low);           
            low.setDefaultValue(0.0);           
            data.add(std::move(low)); 
        }
        { 
            parameter::data mid("Mid", { -12.0, 12.0 });                  
            registerCallback<4>(mid);           
            mid.setDefaultValue(0.0);           
            data.add(std::move(mid)); 
        }
        { 
            parameter::data high("High", { -12.0, 12.0 });                
            registerCallback<5>(high);          
            high.setDefaultValue(0.0);          
            data.add(std::move(high)); 
        }
        { 
            parameter::data presence("Presence", { -12.0, 12.0 });        
            registerCallback<6>(presence);      
            presence.setDefaultValue(0.0);      
            data.add(std::move(presence)); 
        }
        { 
            parameter::data outputGain("OutputGain", { -100.0, 100.0 });  
            registerCallback<7>(outputGain);    
            outputGain.setDefaultValue(0.0);    
            data.add(std::move(outputGain)); 
        }          
        {
            parameter::data distortionMode("DistortionMode", { 0.0, 1.0 });  
            registerCallback<8>(distortionMode);    
            distortionMode.setDefaultValue(0.0);    
            data.add(std::move(distortionMode)); 
        }   
        {
            parameter::data cleanToneStackMode("CleanToneStackMode", { 0.0, 3.0 });  
            registerCallback<9>(cleanToneStackMode);    
            cleanToneStackMode.setDefaultValue(0.0);    
            data.add(std::move(cleanToneStackMode)); 
        }     
        {
            parameter::data dirtyToneStackMode("DirtyToneStackMode", { 0.0, 5.0 });  
            registerCallback<10>(dirtyToneStackMode);    
            dirtyToneStackMode.setDefaultValue(0.0);    
            data.add(std::move(dirtyToneStackMode)); 
        }   

        // TS design
        {
            parameter::data latestFilterFreq("LatestFilterFreq", { 20.0, 20000.0 });  
            registerCallback<11>(latestFilterFreq);    
            latestFilterFreq.setDefaultValue(10000.0);    
            data.add(std::move(latestFilterFreq)); 
        }
        {
            parameter::data latestFilterQ("LatestFilterQ", { 0.2, 8.0 });  
            registerCallback<12>(latestFilterQ);    
            latestFilterQ.setDefaultValue(1.0);    
            data.add(std::move(latestFilterQ)); 
        }
        {
            parameter::data latestFilterGain("LatestFilterGain", { -24.0, 24.0 });  
            registerCallback<13>(latestFilterGain);    
            latestFilterGain.setDefaultValue(0.0);    
            data.add(std::move(latestFilterGain)); 
        }

           
    }    
    
private:

    float sampleRate = 44100.0f;
    int numChannels = 2;
    int maxBlockSize = 512;    
    int ampMode = 1; // 0 = clean | 1 = dirty | 2 = nam
    float inputGain = 0.0f;
    float low = 0.0f;
    float mid = 0.0f;
    float high = 0.0f;
    float presence = 0.0f;
    float outputGain = 0.0f;        

    int distortionMode = 0;    
    int cleanToneStackMode = 0;
    int dirtyToneStackMode = 0;    

    float dirtyLowBandMem[2] = {0.0f, 0.0f};
    float dirtyPresenceMem[2] = {0.0f, 0.0f};
    float biasMemLeft = 0.0f;

    float latestFilterFreq = 1000.0f;
    float latestFilterQ = 0.81f;
    float latestFilterGain = 0.0f;    
    
    // oversampling
    struct ProcessSpec
    {
        double sampleRate = 44100.0;
        uint32 numChannels = 2;
        uint32 maximumBlockSize = 512;
    };

    ProcessSpec lastSpecs;
    ProcessSpec specsOversampling;
    int oversamplingFactor = 1;    
    float scaledSampleRate = 44100.0f;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;    
    
    // main process function
    void processAudioBuffer(juce::AudioBuffer<float>& buffer)
    {        
        const bool useOversampling = (oversamplingFactor > 0 && oversampling != nullptr && ampMode < 2); // oversample (only for non-NAM modes)
        juce::dsp::AudioBlock<float> inBlock(buffer);

        if (useOversampling)
        {            
            auto upBlock = oversampling->processSamplesUp(inBlock);
            const int upCh     = static_cast<int>(upBlock.getNumChannels());
            const int upFrames = static_cast<int>(upBlock.getNumSamples());
            juce::HeapBlock<float*> chanPtrs(upCh);
            for (int ch = 0; ch < upCh; ++ch) { chanPtrs[ch] = upBlock.getChannelPointer(static_cast<size_t>(ch)); }
                
            // non-owning view of the upsampled data
            juce::AudioBuffer<float> osBuffer(chanPtrs.getData(), upCh, upFrames);

            // process
            if (ampMode == 0) { processCleanMode(osBuffer, osBuffer.getNumSamples()); }
            else { processDirtyMode(osBuffer, osBuffer.getNumSamples()); }

            oversampling->processSamplesDown(inBlock); // downsample back to original sr
        }
        else // no oversampling
        {            
            const int numSamples = buffer.getNumSamples(); 
            if (ampMode == 0) { processCleanMode(buffer, numSamples); }
            else if (ampMode == 1) { processDirtyMode(buffer, numSamples); }
            else if (ampMode == 2) { processNAMMode(buffer, numSamples); }
        }
    }

    void processCleanMode(AudioBuffer<float>& buffer, int numSamples)
    {
        int numCh = juce::jmin(2, buffer.getNumChannels());
        
        for (int ch = 0; ch < numCh; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            
            for (int s = 0; s < numSamples; ++s)
            {
                float sample = channelData[s];

                sample *= Decibels::decibelsToGain(inputGain); // input gain                                                

                switch (cleanToneStackMode)
                {
                    case 0: for (int f = 0; f < numGlassPreSculptStates; ++f) { sample = glassPreSculptStates[ch][f].process(sample); } break;
                    case 1: for (int f = 0; f < numPearlPreSculptStates; ++f) { sample = pearlPreSculptStates[ch][f].process(sample); } break;
                    case 2: for (int f = 0; f < numDiamondPreSculptStates; ++f) { sample = diamondPreSculptStates[ch][f].process(sample); } break;
                    case 3: for (int f = 0; f < numZirconPreSculptStates; ++f) { sample = zirconPreSculptStates[ch][f].process(sample); } break;                    
                }

                sample = getCleanSample(sample); // transfer function  

                for (int f = 0; f < numToneStackStates; ++f) { sample = toneStackStates[ch][f].process(sample); } // tone stack

                switch (cleanToneStackMode)
                {
                    case 0: for (int f = 0; f < numGlassPostSculptStates; ++f) { sample = glassPostSculptStates[ch][f].process(sample); } break;
                    case 1: for (int f = 0; f < numPearlPostSculptStates; ++f) { sample = pearlPostSculptStates[ch][f].process(sample); } break;
                    case 2: for (int f = 0; f < numDiamondPostSculptStates; ++f) { sample = diamondPostSculptStates[ch][f].process(sample); } break;
                    case 3: for (int f = 0; f < numZirconPostSculptStates; ++f) { sample = zirconPostSculptStates[ch][f].process(sample); } break;                    
                }
                
                sample *= Decibels::decibelsToGain(outputGain); // output gain
                
                channelData[s] = sample;
            }
        }
    }

    void processDirtyMode(AudioBuffer<float>& buffer, int numSamples)
    {
        int numCh = juce::jmin(2, buffer.getNumChannels());
        
        for (int ch = 0; ch < numCh; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            
            for (int s = 0; s < numSamples; ++s)
            {
                float sample = channelData[s];

                sample *= Decibels::decibelsToGain(inputGain); // input gain                     

                // pre sculpt
                switch (dirtyToneStackMode)
                {                    
                    case 0: for (int f = 0; f < numSlatePreSculptStates; ++f) { sample = slatePreSculptStates[ch][f].process(sample); } break;
                    case 1: for (int f = 0; f < numObsidianPreSculptStates; ++f) { sample = obsidianPreSculptStates[ch][f].process(sample); } break;
                    case 2: for (int f = 0; f < numAmethystPreSculptStates; ++f) { sample = amethystPreSculptStates[ch][f].process(sample); } break;
                    case 3: for (int f = 0; f < numOpalPreSculptStates; ++f) { sample = opalPreSculptStates[ch][f].process(sample); } break;
                    case 4: for (int f = 0; f < numSapphirePreSculptStates; ++f) { sample = sapphirePreSculptStates[ch][f].process(sample); } break;
                    case 5: for (int f = 0; f < numGarnetPreSculptStates; ++f) { sample = garnetPreSculptStates[ch][f].process(sample); } break;
                }

                for (int f = 0; f < numDCOffsetStates; ++f) { sample = dcOffset[ch][f].process(sample); } // dc blocker filter

                switch (distortionMode) // waveshaper algos go here
                {
                    case 0: sample = getTyrantSample(sample, dirtyLowBandMem[ch], dirtyPresenceMem[ch]); break;
                    case 1: sample = getFacelessSample(sample, biasMemLeft); break;
                }                                           

                // tone stack
                for (int f = 0; f < numToneStackStates; ++f) { sample = toneStackStates[ch][f].process(sample); } // tone stack

                // post sculpt
                switch (dirtyToneStackMode)
                {
                    case 0: for (int f = 0; f < numSlatePostSculptStates; ++f) { sample = slatePostSculptStates[ch][f].process(sample); } break;
                    case 1: for (int f = 0; f < numObsidianPostSculptStates; ++f) { sample = obsidianPostSculptStates[ch][f].process(sample); } break;
                    case 2: for (int f = 0; f < numAmethystPostSculptStates; ++f) { sample = amethystPostSculptStates[ch][f].process(sample); } break;
                    case 3: for (int f = 0; f < numOpalPostSculptStates; ++f) { sample = opalPostSculptStates[ch][f].process(sample); } break;
                    case 4: for (int f = 0; f < numSapphirePostSculptStates; ++f) { sample = sapphirePostSculptStates[ch][f].process(sample); } break;
                    case 5: for (int f = 0; f < numGarnetPostSculptStates; ++f) { sample = garnetPostSculptStates[ch][f].process(sample); } break;                                       
                }                

                sample *= Decibels::decibelsToGain(outputGain); // output gain
                
                channelData[s] = sample; 
            }
        }
    }    
    
    // amp algos
    float getCleanSample(float input)
    {
        float threshold = 0.8f;
        float warmth = 0.3f;
        float scaledInput = input * (0.3f + warmth * 0.5f);
        float absInput = std::abs(scaledInput);
        
        if (absInput > threshold)
        {
            float excess = absInput - threshold;
            float compressed = threshold + std::tanh(excess * 2.0f) * 0.2f;
            scaledInput = (scaledInput > 0.0f) ? compressed : -compressed;
        }
        
        float harmonic = std::sin(scaledInput * 4.0f) * warmth * 0.02f;
        return scaledInput + harmonic;
    }
    
    float getTyrantSample(float x, float& lowBandMem, float& presenceMem)
    {
        // the original distortion, renamed
        // processes low & high bands separately 

        float y = x;
        float highSat;
        const float crossover = 0.15f;
        const float ceiling = 0.95f;
            
        // low band
        lowBandMem = lowBandMem + crossover * (y - lowBandMem);
        float lowSat = std::tanh(lowBandMem * 2.8f) * 0.9f;

        // high band
        float highBand = y - lowBandMem;                
        float highDrive = highBand * 4.5f;        
        if (highDrive > 0.0f) { highSat = 1.0f - std::exp(-highDrive * 2.0f); }            
        else { highSat = -(1.0f - std::exp(highDrive * 1.8f)); }            
        highSat = highSat * 0.4f;

        // combine
        y = lowSat + highSat;
        
        // hard clip
        if (std::abs(y) > ceiling) 
        {
            float sign = y > 0.0f ? 1.0f : -1.0f;
            float excess = std::abs(y) - ceiling;
            y = sign * (ceiling + excess / (1.0f + excess * 8.0f));
        }
        
        // presence
        float presenceDiff = y - presenceMem;
        presenceMem = y;
        y = y + presenceDiff * 0.08f;

        return y * 0.91f;
    }    
    
    float triode(float x, float bias, float gain, float asym)
    {
        // simple triode helper with grid bias for getFacelessSample()
        const float alpha = 0.7f;
        
        // simple grid into tanh
        x = gain * x + bias;        
        float y = std::tanh(x) + asym * x * x * 0.05f + asym * x * x * x * 0.1f;
                
        // remove bias after for symmetric y
        return y - (alpha * std::tanh(bias));
    }

    float getFacelessSample(float x, float& biasMem) 
    {        
        // slightly more aggressive & using multi-stage triode setup
        // cheeky nod to a very well-known plugin
        const float gain1 = 3.2f;
        const float gain2 = 2.7f;
        const float asym1 = 0.14f; 
        const float asym2 = 0.09f;
        const float biasSpeed = 0.01f; 
        
        // dynamic bias
        float biasTarget = 0.23f + x * 0.09f;
        biasMem += biasSpeed * (biasTarget - biasMem);

        // triode stages
        float stage1 = triode(x, biasMem, gain1, asym1);
        float stage2 = triode(stage1, biasMem * 0.75f, gain2, asym2);

        // simple presence shelf
        float presence = stage2 + (stage2 - x) * 0.12f;

        // scale y out
        float y = std::clamp(presence * 0.75f, -1.0f, 1.0f);
        return y;
    }    

    // Filter Chains

    // dc offset
    static const int numDCOffsetStates = 4;
    BiquadState dcOffset[2][numDCOffsetStates]; // 48db/oct    

    void updateDCOffset()
    {     
        // i was going to run a single 12db/oct filter at each stage of distortion
        // but this sounds fine as it is, might revisit later...
        for (int i = 0; i<numDCOffsetStates; ++i) { makeHighPassStereo(dcOffset[0][i], dcOffset[1][i], 140.0f, scaledSampleRate); }
    }

    // tone stack
    static const int numToneStackStates = 4;    
    BiquadState toneStackStates[2][numToneStackStates];

    void updateToneStack()
    {
        makePeakFilterStereo(toneStackStates[0][0], toneStackStates[1][0], 160.0f, 0.6f, low, scaledSampleRate);
        makePeakFilterStereo(toneStackStates[0][1], toneStackStates[1][1], 600.0f, 0.8f, mid, scaledSampleRate);
        makePeakFilterStereo(toneStackStates[0][2], toneStackStates[1][2], 2600.0f, 0.8f, high, scaledSampleRate);
        makeHighShelfStereo(toneStackStates[0][3], toneStackStates[1][3], 6000.0f, 0.8f, presence, scaledSampleRate);
    }

    // glass
    static const int numGlassPreSculptStates = 4;
    static const int numGlassPostSculptStates = 6;
    BiquadState glassPreSculptStates[2][numGlassPreSculptStates];
    BiquadState glassPostSculptStates[2][numGlassPostSculptStates];

    void updateGlass()
    {
        // [f, q, gain]
        // pre 
        makeHighPassStereo(glassPreSculptStates[0][0], glassPreSculptStates[1][0], 55.0f, scaledSampleRate);
        makePeakFilterStereo(glassPreSculptStates[0][1], glassPreSculptStates[1][1], 870.0f, 0.4f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(glassPreSculptStates[0][2], glassPreSculptStates[1][2], 3700.0f, 0.32f, 7.0f, scaledSampleRate);
        makeHighShelfStereo(glassPreSculptStates[0][3], glassPreSculptStates[1][3], 6000.0f, 0.7f, 16.0f, scaledSampleRate);

        // post 
        makeHighPassStereo(glassPostSculptStates[0][0], glassPostSculptStates[1][0], 60.0f, scaledSampleRate);
        makePeakFilterStereo(glassPostSculptStates[0][1], glassPostSculptStates[1][1], 140.0f, 2.4f, -3.5f, scaledSampleRate);
        makePeakFilterStereo(glassPostSculptStates[0][2], glassPostSculptStates[1][2], 2200.0f, 0.3f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(glassPostSculptStates[0][3], glassPostSculptStates[1][3], 2600.0f, 5.8f, -5.1f, scaledSampleRate);
        makePeakFilterStereo(glassPostSculptStates[0][4], glassPostSculptStates[1][4], 6400.0f, 1.0f, 4.8f, scaledSampleRate);
        makeLowPassStereo(glassPostSculptStates[0][5], glassPostSculptStates[1][5], 10000.0f, scaledSampleRate);
    } 

    // pearl
    static const int numPearlPreSculptStates = 4;
    static const int numPearlPostSculptStates = 6;
    BiquadState pearlPreSculptStates[2][numPearlPreSculptStates];
    BiquadState pearlPostSculptStates[2][numPearlPostSculptStates];

    void updatePearl()
    {
        // [f, q, gain]
        // pre 
        makeHighPassStereo(pearlPreSculptStates[0][0], pearlPreSculptStates[1][0], 55.0f, scaledSampleRate);
        makePeakFilterStereo(pearlPreSculptStates[0][1], pearlPreSculptStates[1][1], 870.0f, 0.4f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(pearlPreSculptStates[0][2], pearlPreSculptStates[1][2], 3700.0f, 0.32f, 7.0f, scaledSampleRate);
        makeHighShelfStereo(pearlPreSculptStates[0][3], pearlPreSculptStates[1][3], 6000.0f, 0.7f, 16.0f, scaledSampleRate);

        // post 
        makeHighPassStereo(pearlPostSculptStates[0][0], pearlPostSculptStates[1][0], 60.0f, scaledSampleRate);
        makePeakFilterStereo(pearlPostSculptStates[0][1], pearlPostSculptStates[1][1], 140.0f, 2.4f, -3.5f, scaledSampleRate);
        makePeakFilterStereo(pearlPostSculptStates[0][2], pearlPostSculptStates[1][2], 2200.0f, 0.3f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(pearlPostSculptStates[0][3], pearlPostSculptStates[1][3], 2600.0f, 5.8f, -5.1f, scaledSampleRate);
        makePeakFilterStereo(pearlPostSculptStates[0][4], pearlPostSculptStates[1][4], 6400.0f, 1.0f, 4.8f, scaledSampleRate);
        makeLowPassStereo(pearlPostSculptStates[0][5], pearlPostSculptStates[1][5], 10000.0f, scaledSampleRate);
    }   

    // diamond
    static const int numDiamondPreSculptStates = 4;
    static const int numDiamondPostSculptStates = 6;
    BiquadState diamondPreSculptStates[2][numDiamondPreSculptStates];
    BiquadState diamondPostSculptStates[2][numDiamondPostSculptStates];

    void updateDiamond()
    {
        // [f, q, gain]
        // pre 
        makeHighPassStereo(diamondPreSculptStates[0][0], diamondPreSculptStates[1][0], 55.0f, scaledSampleRate);
        makePeakFilterStereo(diamondPreSculptStates[0][1], diamondPreSculptStates[1][1], 870.0f, 0.4f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(diamondPreSculptStates[0][2], diamondPreSculptStates[1][2], 3700.0f, 0.32f, 7.0f, scaledSampleRate);
        makeHighShelfStereo(diamondPreSculptStates[0][3], diamondPreSculptStates[1][3], 6000.0f, 0.7f, 16.0f, scaledSampleRate);

        // post 
        makeHighPassStereo(diamondPostSculptStates[0][0], diamondPostSculptStates[1][0], 60.0f, scaledSampleRate);
        makePeakFilterStereo(diamondPostSculptStates[0][1], diamondPostSculptStates[1][1], 140.0f, 2.4f, -3.5f, scaledSampleRate);
        makePeakFilterStereo(diamondPostSculptStates[0][2], diamondPostSculptStates[1][2], 2200.0f, 0.3f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(diamondPostSculptStates[0][3], diamondPostSculptStates[1][3], 2600.0f, 5.8f, -5.1f, scaledSampleRate);
        makePeakFilterStereo(diamondPostSculptStates[0][4], diamondPostSculptStates[1][4], 6400.0f, 1.0f, 4.8f, scaledSampleRate);
        makeLowPassStereo(diamondPostSculptStates[0][5], diamondPostSculptStates[1][5], 10000.0f, scaledSampleRate);
    }

    // zircon
    static const int numZirconPreSculptStates = 4;
    static const int numZirconPostSculptStates = 6;
    BiquadState zirconPreSculptStates[2][numZirconPreSculptStates];
    BiquadState zirconPostSculptStates[2][numZirconPostSculptStates];

    void updateZircon()
    {
        // [f, q, gain]
        // pre 
        makeHighPassStereo(zirconPreSculptStates[0][0], zirconPreSculptStates[1][0], 55.0f, scaledSampleRate);
        makePeakFilterStereo(zirconPreSculptStates[0][1], zirconPreSculptStates[1][1], 870.0f, 0.4f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(zirconPreSculptStates[0][2], zirconPreSculptStates[1][2], 3700.0f, 0.32f, 7.0f, scaledSampleRate);
        makeHighShelfStereo(zirconPreSculptStates[0][3], zirconPreSculptStates[1][3], 6000.0f, 0.7f, 16.0f, scaledSampleRate);

        // post 
        makeHighPassStereo(zirconPostSculptStates[0][0], zirconPostSculptStates[1][0], 60.0f, scaledSampleRate);
        makePeakFilterStereo(zirconPostSculptStates[0][1], zirconPostSculptStates[1][1], 140.0f, 2.4f, -3.5f, scaledSampleRate);
        makePeakFilterStereo(zirconPostSculptStates[0][2], zirconPostSculptStates[1][2], 2200.0f, 0.3f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(zirconPostSculptStates[0][3], zirconPostSculptStates[1][3], 2600.0f, 5.8f, -5.1f, scaledSampleRate);
        makePeakFilterStereo(zirconPostSculptStates[0][4], zirconPostSculptStates[1][4], 6400.0f, 1.0f, 4.8f, scaledSampleRate);
        makeLowPassStereo(zirconPostSculptStates[0][5], zirconPostSculptStates[1][5], 10000.0f, scaledSampleRate);
    }

    // slate 
    static const int numSlatePreSculptStates = 4;
    static const int numSlatePostSculptStates = 17;
    BiquadState slatePreSculptStates[2][numSlatePreSculptStates];
    BiquadState slatePostSculptStates[2][numSlatePostSculptStates];

    void updateSlate()
    {
        // [f, q, gain]
        // pre 
        makeHighPassStereo(slatePreSculptStates[0][0], slatePreSculptStates[1][0], 82.0f, scaledSampleRate);
        makePeakFilterStereo(slatePreSculptStates[0][1], slatePreSculptStates[1][1], 650.0f, 1.2f, 2.5f, scaledSampleRate);
        makePeakFilterStereo(slatePreSculptStates[0][2], slatePreSculptStates[1][2], 2200.0f, 0.8f, 3.2f, scaledSampleRate);
        makeHighShelfStereo(slatePreSculptStates[0][3], slatePreSculptStates[1][3], 4500.0f, 0.7f, 6.0f, scaledSampleRate);    

        // post
        makeHighPassStereo(slatePostSculptStates[0][0], slatePostSculptStates[1][0], 40.0f, scaledSampleRate);
        makeLowShelfStereo(slatePostSculptStates[0][1], slatePostSculptStates[1][1], 45.0f, 0.5f, 6.0f, scaledSampleRate);
        makePeakFilterStereo(slatePostSculptStates[0][2], slatePostSculptStates[1][2], 75.1f, 0.46f, 4.38f, scaledSampleRate);
        makePeakFilterStereo(slatePostSculptStates[0][3], slatePostSculptStates[1][3], 120.0f, 0.5f, 8.0f, scaledSampleRate);
        makeLowShelfStereo(slatePostSculptStates[0][4], slatePostSculptStates[1][4], 150.0f, 0.7f, -2.0f, scaledSampleRate);
        makePeakFilterStereo(slatePostSculptStates[0][5], slatePostSculptStates[1][5], 160.0f, 0.6f, 2.0f, scaledSampleRate);
        makePeakFilterStereo(slatePostSculptStates[0][6], slatePostSculptStates[1][6], 750.0f, 1.4f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(slatePostSculptStates[0][7], slatePostSculptStates[1][7], 1800.0f, 1.0f, 2.8f, scaledSampleRate);
        makePeakFilterStereo(slatePostSculptStates[0][8], slatePostSculptStates[1][8], 2500.0f, 2.11f, 1.86f, scaledSampleRate);
        makePeakFilterStereo(slatePostSculptStates[0][9], slatePostSculptStates[1][9], 3200.0f, 1.2f, -1.5f, scaledSampleRate);
        makePeakFilterStereo(slatePostSculptStates[0][10], slatePostSculptStates[1][10], 3638.0f, 1.156f, 4.616f, scaledSampleRate);  
        makePeakFilterStereo(slatePostSculptStates[0][11], slatePostSculptStates[1][11], 3670.0f, 1.14f, 3.48f, scaledSampleRate);      
        makePeakFilterStereo(slatePostSculptStates[0][12], slatePostSculptStates[1][12], 5000.0f, 0.8f, 2.0f, scaledSampleRate);
        makeHighShelfStereo(slatePostSculptStates[0][13], slatePostSculptStates[1][13], 6500.0f, 0.7f, -1.8f, scaledSampleRate);
        makePeakFilterStereo(slatePostSculptStates[0][14], slatePostSculptStates[1][14], 7842.0f, 0.938f, 4.032f, scaledSampleRate);
        makeHighShelfStereo(slatePostSculptStates[0][15], slatePostSculptStates[1][15], 9000.0f, 0.6f, 3.5f, scaledSampleRate);
        makeLowPassStereo(slatePostSculptStates[0][16], slatePostSculptStates[1][16], 12000.0f, scaledSampleRate);        
    }

    // obsidian
    static const int numObsidianPreSculptStates = 4;
    static const int numObsidianPostSculptStates = 15;
    BiquadState obsidianPreSculptStates[2][numObsidianPreSculptStates];
    BiquadState obsidianPostSculptStates[2][numObsidianPostSculptStates];

    void updateObsidian()
    {
        // [f, q, gain]
        // pre
        makeHighPassStereo(obsidianPreSculptStates[0][0], obsidianPreSculptStates[1][0], 65.0f, scaledSampleRate);
        makePeakFilterStereo(obsidianPreSculptStates[0][1], obsidianPreSculptStates[1][1], 520.0f, 0.9f, 2.8f, scaledSampleRate);
        makePeakFilterStereo(obsidianPreSculptStates[0][2], obsidianPreSculptStates[1][2], 1400.0f, 0.7f, 3.2f, scaledSampleRate);
        makePeakFilterStereo(obsidianPreSculptStates[0][3], obsidianPreSculptStates[1][3], 3800.0f, 1.1f, 4.5f, scaledSampleRate);
                
        // post
        makeHighPassStereo(obsidianPostSculptStates[0][0], obsidianPostSculptStates[1][0], 32.0f, scaledSampleRate);
        makeHighPassStereo(obsidianPostSculptStates[0][1], obsidianPostSculptStates[1][1], 32.0f, scaledSampleRate);
        makeRBJLowShelfStereo(obsidianPostSculptStates[0][2], obsidianPostSculptStates[1][2], 120.0f, 0.5f, 11.712f, scaledSampleRate);
        makePeakFilterStereo(obsidianPostSculptStates[0][3], obsidianPostSculptStates[1][3], 129.0f, 1.74f, 5.86f, scaledSampleRate);
        makePeakFilterStereo(obsidianPostSculptStates[0][4], obsidianPostSculptStates[1][4], 180.0f, 0.813f, 3.0f, scaledSampleRate);
        makePeakFilterStereo(obsidianPostSculptStates[0][5], obsidianPostSculptStates[1][5], 528.0f, 0.71f, -9.0f, scaledSampleRate);
        makePeakFilterStereo(obsidianPostSculptStates[0][6], obsidianPostSculptStates[1][6], 600.0f, 0.8f, 3.84f, scaledSampleRate);
        makePeakFilterStereo(obsidianPostSculptStates[0][7], obsidianPostSculptStates[1][7], 1100.0f, 1.374f, -5.872f, scaledSampleRate);
        makePeakFilterStereo(obsidianPostSculptStates[0][8], obsidianPostSculptStates[1][8], 1787.0f, 0.813, -2.496f, scaledSampleRate);
        makePeakFilterStereo(obsidianPostSculptStates[0][9], obsidianPostSculptStates[1][9], 2185.0f, 0.437f, 7.686f, scaledSampleRate);
        makePeakFilterStereo(obsidianPostSculptStates[0][10], obsidianPostSculptStates[1][10], 2900.0f, 2.03f, 3.60f, scaledSampleRate);
        makeHighShelfStereo(obsidianPostSculptStates[0][11], obsidianPostSculptStates[1][11], 6000.0f, 0.8f, 5.0f, scaledSampleRate);
        makeHighShelfStereo(obsidianPostSculptStates[0][12], obsidianPostSculptStates[1][12], 7760.0f, 0.813f, 2.5f, scaledSampleRate);   
        makePeakFilterStereo(obsidianPostSculptStates[0][13], obsidianPostSculptStates[1][13], 8678.0f, 0.688f, 3.532f, scaledSampleRate);
        makeLowPassStereo(obsidianPostSculptStates[0][14], obsidianPostSculptStates[1][14], 17122.0f, scaledSampleRate);        
    }    

    // amethyst
    static const int numAmethystPreSculptStates = 4;
    static const int numAmethystPostSculptStates = 10;
    BiquadState amethystPreSculptStates[2][numAmethystPreSculptStates];
    BiquadState amethystPostSculptStates[2][numAmethystPostSculptStates];

    void updateAmethyst()
    {
        // [f, q, gain]
        // pre
        makeHighPassStereo(amethystPreSculptStates[0][0], amethystPreSculptStates[1][0], 60.0f, scaledSampleRate);
        makePeakFilterStereo(amethystPreSculptStates[0][1], amethystPreSculptStates[1][1], 500.0f, 1.1f, 2.8f, scaledSampleRate);
        makePeakFilterStereo(amethystPreSculptStates[0][2], amethystPreSculptStates[1][2], 1600.0f, 0.9f, 3.5f, scaledSampleRate);
        makePeakFilterStereo(amethystPreSculptStates[0][3], amethystPreSculptStates[1][3], 4000.0f, 1.3f, 4.0f, scaledSampleRate);
        
        // post
        makeHighPassStereo(amethystPostSculptStates[0][0], amethystPostSculptStates[1][0], 45.0f, scaledSampleRate);
        makeLowShelfStereo(amethystPostSculptStates[0][1], amethystPostSculptStates[1][1], 45.0f, 0.5f, 6.0f, scaledSampleRate);
        makePeakFilterStereo(amethystPostSculptStates[0][2], amethystPostSculptStates[1][2], 120.0f, 0.5f, 8.0f, scaledSampleRate);
        makePeakFilterStereo(amethystPostSculptStates[0][3], amethystPostSculptStates[1][3], 180.0f, 1.2f, -1.8f, scaledSampleRate);
        makePeakFilterStereo(amethystPostSculptStates[0][4], amethystPostSculptStates[1][4], 600.0f, 1.0f, 3.2f, scaledSampleRate);
        makePeakFilterStereo(amethystPostSculptStates[0][5], amethystPostSculptStates[1][5], 1200.0f, 0.8f, 2.5f, scaledSampleRate);
        makePeakFilterStereo(amethystPostSculptStates[0][6], amethystPostSculptStates[1][6], 2800.0f, 1.4f, -2.0f, scaledSampleRate);
        makePeakFilterStereo(amethystPostSculptStates[0][7], amethystPostSculptStates[1][7], 4500.0f, 1.1f, 4.5f, scaledSampleRate);
        makePeakFilterStereo(amethystPostSculptStates[0][8], amethystPostSculptStates[1][8], 6500.0f, 0.9f, 2.8f, scaledSampleRate);
        makeHighShelfStereo(amethystPostSculptStates[0][9], amethystPostSculptStates[1][9], 8500.0f, 0.7f, 3.5f, scaledSampleRate);
        makeLowPassStereo(amethystPostSculptStates[0][10], amethystPostSculptStates[1][10], 13000.0f, scaledSampleRate);        
    }

    // opal
    static const int numOpalPreSculptStates = 4;
    static const int numOpalPostSculptStates = 10;
    BiquadState opalPreSculptStates[2][numOpalPreSculptStates];
    BiquadState opalPostSculptStates[2][numOpalPostSculptStates];
      
    void updateOpal()
    {
        // [f, q, gain]
        // pre
        makeHighPassStereo(opalPreSculptStates[0][0], opalPreSculptStates[1][0], 90.0f, scaledSampleRate);
        makePeakFilterStereo(opalPreSculptStates[0][1], opalPreSculptStates[1][1], 720.0f, 1.5f, 3.8f, scaledSampleRate);
        makePeakFilterStereo(opalPreSculptStates[0][2], opalPreSculptStates[1][2], 2000.0f, 1.0f, 2.5f, scaledSampleRate);
        makePeakFilterStereo(opalPreSculptStates[0][3], opalPreSculptStates[1][3], 5500.0f, 1.8f, 5.2f, scaledSampleRate);
        
        // post
        makeHighPassStereo(opalPostSculptStates[0][0], opalPostSculptStates[1][0], 40.0f, scaledSampleRate);
        makeHighPassStereo(opalPostSculptStates[0][1], opalPostSculptStates[1][1], 45.0f, scaledSampleRate);
        makeLowShelfStereo(opalPostSculptStates[0][2], opalPostSculptStates[1][2], 45.0f, 0.5f, 6.0f, scaledSampleRate);
        makePeakFilterStereo(opalPostSculptStates[0][3], opalPostSculptStates[1][3], 120.0f, 0.5f, 8.0f, scaledSampleRate);
        makePeakFilterStereo(opalPostSculptStates[0][4], opalPostSculptStates[1][4], 200.0f, 1.5f, -3.2f, scaledSampleRate);
        makePeakFilterStereo(opalPostSculptStates[0][5], opalPostSculptStates[1][5], 800.0f, 1.2f, 4.5f, scaledSampleRate);
        makePeakFilterStereo(opalPostSculptStates[0][6], opalPostSculptStates[1][6], 1600.0f, 0.9f, 2.8f, scaledSampleRate);
        makePeakFilterStereo(opalPostSculptStates[0][7], opalPostSculptStates[1][7], 3200.0f, 2.0f, -2.8f, scaledSampleRate);
        makePeakFilterStereo(opalPostSculptStates[0][8], opalPostSculptStates[1][8], 5800.0f, 1.3f, 4.8f, scaledSampleRate);
        makeHighShelfStereo(opalPostSculptStates[0][9], opalPostSculptStates[1][9], 7500.0f, 0.8f, -2.5f, scaledSampleRate);
        makeLowPassStereo(opalPostSculptStates[0][10], opalPostSculptStates[1][10], 11500.0f, scaledSampleRate);                
    }  

    // sapphire
    static const int numSapphirePreSculptStates = 4;
    static const int numSapphirePostSculptStates = 10;
    BiquadState sapphirePreSculptStates[2][numSapphirePreSculptStates];
    BiquadState sapphirePostSculptStates[2][numSapphirePostSculptStates];

    void updateSapphire()
    {
        // [f, q, gain]
        // pre
        makeHighPassStereo(garnetPreSculptStates[0][0], garnetPreSculptStates[1][0], 85.0f, scaledSampleRate);
        makePeakFilterStereo(garnetPreSculptStates[0][1], garnetPreSculptStates[1][1], 680.0f, 1.4f, 4.2f, scaledSampleRate);
        makePeakFilterStereo(garnetPreSculptStates[0][2], garnetPreSculptStates[1][2], 1800.0f, 0.9f, 3.5f, scaledSampleRate);
        makePeakFilterStereo(garnetPreSculptStates[0][3], garnetPreSculptStates[1][3], 4800.0f, 1.6f, 5.8f, scaledSampleRate);
        
        // post
        makeHighPassStereo(garnetPostSculptStates[0][0], garnetPostSculptStates[1][0], 45.0f, scaledSampleRate);
        makeLowShelfStereo(garnetPostSculptStates[0][1], garnetPostSculptStates[1][1], 45.0f, 0.5f, 6.0f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][2], garnetPostSculptStates[1][2], 120.0f, 0.5f, 8.0f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][3], garnetPostSculptStates[1][3], 150.0f, 1.2f, -2.5f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][4], garnetPostSculptStates[1][4], 750.0f, 1.1f, 4.8f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][5], garnetPostSculptStates[1][5], 1500.0f, 0.8f, 2.8f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][6], garnetPostSculptStates[1][6], 2800.0f, 1.8f, -2.2f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][7], garnetPostSculptStates[1][7], 5200.0f, 1.2f, 5.5f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][8], garnetPostSculptStates[1][8], 7200.0f, 0.9f, 3.2f, scaledSampleRate);
        makeHighShelfStereo(garnetPostSculptStates[0][9], garnetPostSculptStates[1][9], 9000.0f, 0.7f, -1.8f, scaledSampleRate);
        makeLowPassStereo(garnetPostSculptStates[0][10], garnetPostSculptStates[1][10], 12800.0f, scaledSampleRate);                
    }

    // garnet
    static const int numGarnetPreSculptStates = 4;
    static const int numGarnetPostSculptStates = 10;
    BiquadState garnetPreSculptStates[2][numGarnetPreSculptStates];
    BiquadState garnetPostSculptStates[2][numGarnetPostSculptStates];

    void updateGarnet()
    {
        // [f, q, gain]
        // pre
        makeHighPassStereo(garnetPreSculptStates[0][0], garnetPreSculptStates[1][0], 50.0f, scaledSampleRate);
        makePeakFilterStereo(garnetPreSculptStates[0][1], garnetPreSculptStates[1][1], 320.0f, 0.8f, 2.2f, scaledSampleRate);
        makePeakFilterStereo(garnetPreSculptStates[0][2], garnetPreSculptStates[1][2], 1000.0f, 0.7f, 2.8f, scaledSampleRate);
        makeHighShelfStereo(garnetPreSculptStates[0][3], garnetPreSculptStates[1][3], 2800.0f, 0.6f, 3.8f, scaledSampleRate);
        
        // post
        makeHighPassStereo(garnetPostSculptStates[0][0], garnetPostSculptStates[1][0], 45.0f, scaledSampleRate);
        makeLowShelfStereo(garnetPostSculptStates[0][1], garnetPostSculptStates[1][1], 45.0f, 0.5f, 6.0f, scaledSampleRate);
        makeLowShelfStereo(garnetPostSculptStates[0][2], garnetPostSculptStates[1][2], 100.0f, 0.7f, 2.0f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][3], garnetPostSculptStates[1][3], 120.0f, 0.5f, 8.0f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][4], garnetPostSculptStates[1][4], 360.0f, 0.9f, 2.5f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][5], garnetPostSculptStates[1][5], 850.0f, 0.8f, 2.8f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][6], garnetPostSculptStates[1][6], 1600.0f, 0.9f, 2.2f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][7], garnetPostSculptStates[1][7], 3200.0f, 1.1f, 3.5f, scaledSampleRate);
        makePeakFilterStereo(garnetPostSculptStates[0][8], garnetPostSculptStates[1][8], 5500.0f, 0.8f, 3.8f, scaledSampleRate);
        makeHighShelfStereo(garnetPostSculptStates[0][9], garnetPostSculptStates[1][9], 7500.0f, 0.7f, 4.5f, scaledSampleRate);
        makeLowPassStereo(garnetPostSculptStates[0][10], garnetPostSculptStates[1][10], 16000.0f, scaledSampleRate);        
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
            
            scaledSampleRate = static_cast<float>(specsOversampling.sampleRate);
        }

        else { scaledSampleRate = static_cast<float>(lastSpecs.sampleRate); }       
        
        oversampling.swap(newOversampling);        

        updateDCOffset();
        updateToneStack();

        updateGlass();
        updatePearl();
        updateDiamond();
        updateZircon();

        updateSlate();
        updateObsidian();
        updateAmethyst();
        updateOpal();
        updateSapphire();
        updateGarnet();        
    }

    // nam

    // nam math provider (xsimd because eigen was a flop)
    struct NAMMathsProvider
    {
        #if RTNEURAL_USE_EIGEN
                template <typename Matrix>
                static auto tanh(const Matrix& x)
                {
                    const auto x_poly = x.array() * (1.0f + 0.183428244899f * x.array().square());
                    return x_poly.array() * (x_poly.array().square() + 1.0f).array().rsqrt();
                }
        #elif RTNEURAL_USE_XSIMD
                template <typename T>
                static T tanh(const T& x)
                {
                    return math_approx::tanh<3>(x);
                }
        #endif
    };  

    using Dilations = wavenet::Dilations<1, 2, 4, 8, 16, 32, 64, 128, 256, 512>;

    using Model = wavenet::Wavenet_Model<float,
        1,
        wavenet::Layer_Array<float, 1, 1, 8, 16, 3, Dilations, false, NAMMathsProvider>,
        wavenet::Layer_Array<float, 16, 1, 1, 8, 3, Dilations, true, NAMMathsProvider>>;
    
    void loadNAMModel(const var& jsonData)
    {
        // thread-safe JSON loader, creates new model off-thread & atomically swaps it in        
        juce::String jsonString;

        if (jsonData.isObject()) { jsonString = juce::JSON::toString(jsonData); }
        else if (jsonData.isString()) { jsonString = jsonData.toString(); }        
        else { DBG("Invalid JSON data format for NAM loader."); return; }

        if (jsonString.isEmpty()) { return; }
        if (isLoading.exchange(true)) { return; }

        const std::string jsonCopy = jsonString.toStdString();

        std::thread([this, jsonCopy]()
        {
            try
            {
                nlohmann::json modelJson = nlohmann::json::parse(jsonCopy);

                auto newModel = std::make_shared<Model>();
                newModel->load_weights(modelJson);                
                newModel->prepare(static_cast<int>(lastSpecs.maximumBlockSize)); // prepare off-thread

                // get sample rate or assume 48000 if not found
                double sr = 48000.0;
                if (modelJson.contains("sample_rate"))
                {
                    try { sr = static_cast<double>(modelJson["sample_rate"]); }
                    catch (...) {}
                }
                modelSampleRate.store(sr, std::memory_order_release);
                resamplerConfig.store(true, std::memory_order_release);

                std::atomic_store(&modelShared, std::move(newModel));
                modelLoaded = true;
                modelPath = {}; 
                needsWarmup.store(true, std::memory_order_release); // request warmup 
            }
            catch (const std::exception& e) { DBG("Exception loading NAM model: " << e.what()); modelLoaded = false; }            
            catch (...) { DBG("Unknown exception loading NAM model"); modelLoaded = false; }            

            isLoading.store(false);
        }).detach();
    }
 
    struct UpsampleAAFilter
    {
        // this is all vibe-coded but it sounds fine and doesn't blow up the cpu

        BiquadState stage1, stage2;
        
        void reset()
        {
            stage1.reset();
            stage2.reset();
        }
        
        void setup(float sampleRate, float cutoffFreq)
        {
            setupLowpass(stage1, sampleRate, cutoffFreq, 0.541f);
            setupLowpass(stage2, sampleRate, cutoffFreq, 1.307f);
        }
        
        float process(float input)
        {
            float out = stage1.process(input);
            return stage2.process(out);
        }
        
    private:
        void setupLowpass(BiquadState& state, float sr, float freq, float Q)
        {
            float omega = 2.0f * juce::MathConstants<float>::pi * freq / sr;
            float cosOmega = std::cos(omega);
            float sinOmega = std::sin(omega);
            float alpha = sinOmega / (2.0f * Q);
            
            float b0 = (1.0f - cosOmega) / 2.0f;
            float b1 = 1.0f - cosOmega;
            float b2 = (1.0f - cosOmega) / 2.0f;
            float a0 = 1.0f + alpha;
            float a1 = -2.0f * cosOmega;
            float a2 = 1.0f - alpha;
            
            state.b0 = b0 / a0;
            state.b1 = b1 / a0;
            state.b2 = b2 / a0;
            state.a1 = a1 / a0;
            state.a2 = a2 / a0;
        }
    };

    struct StreamingResampler
    {        
        enum class ResampleAlgo { Lagrange = 0, Lanczos = 1, WindowedSinc = 2, HQUpsampling = 3 };

        void setup(double inRate, double outRate, ResampleAlgo algo, int kernelSize)
        {
            inSR = inRate;
            outSR = outRate;
            algoType = algo;
            isUpsampling = outRate > inRate;                        
            step = inSR / outSR;
 
            // anti-alias filter
            if (isUpsampling) { cutoff = static_cast<float>(std::min(0.45, (inSR * 0.45) / outSR)); }                        

            resetState();
        }

        void resetState()
        {
            pos = 0.0;
            history.clear();
            history.resize(HistoryLen(), 0.0f);            
        }
        
        void processBlock(const float* in, int inCount, std::vector<float>& out, int outCount)
        {
            // process a block to produce outCount samples
            const int half = HalfKernel();
            const int postPad = half + 8; // extra safety at block tails
                        
            // source buffer [history][in][post zeros]
            src.resize(history.size() + inCount + postPad);
                        
            if (!history.empty()) { std::memcpy(src.data(), history.data(), history.size() * sizeof(float)); } // copy history                            
            if (inCount > 0) { std::memcpy(src.data() + history.size(), in, inCount * sizeof(float)); } // copy input                                        
            if (postPad > 0) { std::memset(src.data() + history.size() + inCount, 0, postPad * sizeof(float)); } // zero post-pad
                
            out.resize(outCount);
            const double base = static_cast<double>(history.size());
            
            for (int m = 0; m < outCount; ++m)
            {
                const double x = pos + base + m * step;
                out[m] = lagrange6(x);                
            }
            
            double newPos = pos + outCount * step - static_cast<double>(inCount); // update position
            
            const double maxDrift = static_cast<double>(HistoryLen()); 
            pos = juce::jlimit(-maxDrift, maxDrift, newPos); // clamp to prevent excessive drift

            const int keepN = HistoryLen(); 
            history.resize(keepN); // update history
            
            if (inCount >= keepN) { std::memcpy(history.data(), in + (inCount - keepN), keepN * sizeof(float)); }
            else
            {
                // not enough new input, shift old history and append new input
                const int fromHistory = keepN - inCount;
                const int historyStart = static_cast<int>(history.size()) - fromHistory; // equals inCount

                // overlapping copy: must use memmove
                if (historyStart >= 0 && fromHistory > 0) { std::memmove(history.data(), history.data() + historyStart, fromHistory * sizeof(float)); }
                
                if (inCount > 0) { std::memcpy(history.data() + fromHistory, in, inCount * sizeof(float)); }
            }
        }

        // inline helpers
        inline int HalfKernel() const { return kSize / 2; }
        inline int HistoryLen() const { return HalfKernel() + 8; }

        inline float lagrange6(double posInSrc) const
        {
            // vibe coded
            // lagrange sounded the best from my testing
            // 6-point Lagrange interpolation using nodes at [-2, -1, 0, 1, 2, 3]
            const int baseIdx = (int)std::floor(posInSrc);
            const float mu = (float)(posInSrc - (double)baseIdx);

            static constexpr int taps = 6;
            static constexpr int nodes[taps] = { -2, -1, 0, 1, 2, 3 };

            float y[taps];
            for (int i = 0; i < taps; ++i)
            {
                int idx = baseIdx + nodes[i];
                if ((unsigned)idx < (unsigned)src.size())
                    y[i] = src[(size_t)idx];
                else
                    y[i] = 0.0f;
            }

            float out = 0.0f;

            for (int i = 0; i < taps; ++i)
            {
                float Li = 1.0f;
                const float xi = (float)nodes[i];
                for (int j = 0; j < taps; ++j)
                {
                    if (i == j) continue;
                    const float xj = (float)nodes[j];
                    Li *= (mu - xj) / (xi - xj);
                }
                out += Li * y[i];
            }

            return out;
        }
        
        double inSR { 48000.0 };
        double outSR { 48000.0 };
        double step { 1.0 };
        double pos  { 0.0 };
        int    kSize { 6 };
        float  cutoff { 1.0f };
        bool   isUpsampling { false };
        ResampleAlgo algoType { ResampleAlgo::Lagrange };

        std::vector<float> history;
        mutable std::vector<float> src;
    };

    

    void updateNAMResamplers()
    {
        if (!resamplerConfig.load(std::memory_order_acquire)) { return; }            

        const double hostSR  = lastSpecs.sampleRate > 0 ? lastSpecs.sampleRate : (double)sampleRate;
        const double modelSR = modelSampleRate.load(std::memory_order_acquire);

        StreamingResampler::ResampleAlgo algo = StreamingResampler::ResampleAlgo::Lagrange;
        
        inputResampler.setup(hostSR, modelSR, algo, resamplerKernelSize);
        outputResampler.setup(modelSR, hostSR, algo, resamplerKernelSize);

        // prepare scratches        
        const int maxIn = (int)lastSpecs.maximumBlockSize;
        const int upEstimate = (int)std::ceil(maxIn * (modelSR / hostSR)) + 32;
        scratchUp.resize(upEstimate);
        scratchNAM.resize(upEstimate);
        scratchDown.resize(maxIn + 32);
        scratchPreFilter.resize(maxIn + 32);

        resamplerConfig.store(false, std::memory_order_release);
    }

    void warmupNAM(Model& m)
    {
        // calls forward on a bunch of 0 samples then discards them to warmup the model
        for (int i = 0; i < 2048; ++i) { (void)m.forward(0.0f); }            
    }

    void processNAMMode(AudioBuffer<float>& buffer, int numSamples)
    {
        updateNAMResamplers();
        
        float* channelDataL = buffer.getWritePointer(0); // nam processes mono only (for now, might implement stereo later)
        auto m = std::atomic_load(&modelShared);
        
        if (m && needsWarmup.load(std::memory_order_acquire)) // if model needs warming, warm it
        {
            bool expected = true;
            if (needsWarmup.compare_exchange_strong(expected, false, std::memory_order_acq_rel))
                warmupNAM(*m);
        }

        // safety check in case model doesn't exist
        if (!m)
        {
            if (buffer.getNumChannels() > 1)
                std::memcpy(buffer.getWritePointer(1), buffer.getReadPointer(0), (size_t)numSamples * sizeof(float));
            return;
        }

        const double hostSR  = (double)sampleRate;
        const double modelSR = modelSampleRate.load(std::memory_order_acquire);

        // check if upsampling is enabled (by default it's off cause it sounded bad lol)
        const bool isUpsampling = modelSR > hostSR;
        if (isUpsampling && !allowUpsampleResampling) // skip upsampling (thru)
        {            
            for (int s = 0; s < numSamples; ++s)
            {
                float sample = channelDataL[s];

                sample *= Decibels::decibelsToGain(inputGain); // input gain
                sample = m->forward(sample); // nam transfer function                                
                for (int f = 0; f < numToneStackStates; ++f) { sample = toneStackStates[0][f].process(sample); } // apply tone stack AFTER resampling and transfer
                sample *= Decibels::decibelsToGain(outputGain); // output gain
                    
                channelDataL[s] = sample;                
            }

            if (buffer.getNumChannels() > 1) // copy to R
                std::memcpy(buffer.getWritePointer(1), buffer.getReadPointer(0), (size_t)numSamples * sizeof(float));
            return;
        }
        
        const bool needsResample = std::abs(modelSR - hostSR) > 0.5; // check for necessary downsampling

        if (!needsResample) // no resampling
        {            
            for (int s = 0; s < numSamples; ++s)
            {
                float sample = channelDataL[s];
                
                sample *= Decibels::decibelsToGain(inputGain); // input gain
                sample = m->forward(sample); // nam transfer function                                
                for (int f = 0; f < numToneStackStates; ++f) { sample = toneStackStates[0][f].process(sample); } // apply tone stack after resampling and transfer func                    
                sample *= Decibels::decibelsToGain(outputGain); // output gain

                channelDataL[s] = sample;
            }

            if (buffer.getNumChannels() > 1) // copy to R
                std::memcpy(buffer.getWritePointer(1), buffer.getReadPointer(0), (size_t)numSamples * sizeof(float));

            return;
        }
        
        // resample start

        float* inputData = channelDataL;
        const double upRatio = modelSR / hostSR;
        const int upCountTarget = (int)std::llround((double)numSamples * upRatio);

        inputResampler.processBlock(inputData, numSamples, scratchUp, upCountTarget);

        // process resampled data
        const int upCount = (int)scratchUp.size();
        scratchNAM.resize(upCount);
        for (int i = 0; i < upCount; ++i)
        {
            float x = scratchUp[i];

            x *= Decibels::decibelsToGain(inputGain); // inputGain
            x = m->forward(x); // nam transfer function

            scratchNAM[i] = x;
        }

        // resample end
        outputResampler.processBlock(scratchNAM.data(), upCount, scratchDown, numSamples);

        // tone stack & output gain        
        float* outL = channelDataL;
        for (int s = 0; s < numSamples; ++s)
        {
            float y = scratchDown[s];
            for (int f = 0; f < numToneStackStates; ++f)
                y = toneStackStates[0][f].process(y); // use host SR for coefficients

            y *= Decibels::decibelsToGain(outputGain); // outputGain
            outL[s] = y;
        }

        if (buffer.getNumChannels() > 1) // copy to R
            std::memcpy(buffer.getWritePointer(1), buffer.getReadPointer(0), (size_t)numSamples * sizeof(float));
    }

    
    bool modelLoaded = false;
    juce::String modelPath = "";
    
    std::shared_ptr<Model> modelShared;
    std::atomic<bool> isLoading { false };
    std::atomic<bool> needsWarmup { false };

    // nam resampler
               
    int resamplerMode = 0; // default lagrange
    int resamplerKernelSize = 32; // used by lanczos and sinc
    bool allowUpsampleResampling = false; // allow resampling when host < model sample rate (disable by default)
    std::atomic<double> modelSampleRate { 48000.0 }; // most nam models are trained at 48000
    std::atomic<bool>   resamplerConfig { true };

    UpsampleAAFilter upsampleAAFilter;

    StreamingResampler inputResampler;   // host -> model
    StreamingResampler outputResampler;  // model -> host
    std::vector<float> scratchUp;        // upsampled input for NAM
    std::vector<float> scratchDown;      // NAM output resampled back
    std::vector<float> scratchNAM;       // NAM processing buffer (mono)
    std::vector<float> scratchPreFilter; // Pre-filtered input for HQ upsampling
   
};
}