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
#include <initializer_list>
#include <array>

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
        toneStack.reset();
        dcBlocker.reset();
        glass.reset();
        pearl.reset();
        diamond.reset();
        zircon.reset();
        slate.reset();
        obsidian.reset();
        amethyst.reset();
        opal.reset();
        sapphire.reset();
        garnet.reset();
                
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
        case 3: 
        {
            low = static_cast<float>(v); 
            toneStack.setPreGain(0, low);
            toneStack.update(scaledSampleRate); 
            break;
        }
        case 4: 
        {
            mid = static_cast<float>(v); 
            toneStack.setPreGain(1, mid);
            toneStack.update(scaledSampleRate); 
            break;
        }
        case 5: 
        {
            high = static_cast<float>(v); 
            toneStack.setPreGain(2, high);
            toneStack.update(scaledSampleRate); 
            break;
        }
        case 6: 
        {
            presence = static_cast<float>(v); 
            toneStack.setPreGain(3, presence);
            toneStack.update(scaledSampleRate); 
            break;
        }
        case 7: outputGain = static_cast<float>(v); break;           
        case 8: distortionMode = static_cast<int>(v); break;
        case 9: cleanToneStackMode = static_cast<int>(v); break;  
        case 10: dirtyToneStackMode = static_cast<int>(v); break;  
        case 11: latestFilterFreq = static_cast<float>(v); break;
        case 12: latestFilterQ = static_cast<float>(v); break;
        case 13: latestFilterGain = static_cast<float>(v); break;
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
                    case 0: sample = glass.processPre(ch, sample); break;
                    case 1: sample = pearl.processPre(ch, sample); break;
                    case 2: sample = diamond.processPre(ch, sample); break;
                    case 3: sample = zircon.processPre(ch, sample); break;
                }

                sample = getCleanSample(sample); // transfer function  

                //for (int f = 0; f < numToneStackStates; ++f) { sample = toneStackStates[ch][f].process(sample); } // tone stack
                sample = toneStack.processPre(ch, sample);

                switch (cleanToneStackMode)
                {                    
                    case 0: sample = glass.processPost(ch, sample); break;
                    case 1: sample = pearl.processPost(ch, sample); break;
                    case 2: sample = diamond.processPost(ch, sample); break;
                    case 3: sample = zircon.processPost(ch, sample); break;
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
                    case 0: sample = slate.processPre(ch, sample); break;
                    case 1: sample = obsidian.processPre(ch, sample); break;
                    case 2: sample = amethyst.processPre(ch, sample); break;
                    case 3: sample = opal.processPre(ch, sample); break;
                    case 4: sample = sapphire.processPre(ch, sample); break;
                    case 5: sample = garnet.processPre(ch, sample); break;
                }
                
                // dc block
                sample = dcBlocker.processPre(ch, sample);

                switch (distortionMode) // waveshaper algos go here
                {
                    case 0: sample = getTyrantSample(sample, dirtyLowBandMem[ch], dirtyPresenceMem[ch]); break;
                    case 1: sample = getFacelessSample(sample, biasMemLeft); break;
                }                                           

                // tone stack                
                sample = toneStack.processPre(ch, sample);

                // post sculpt
                switch (dirtyToneStackMode)
                {
                    case 0: sample = slate.processPost(ch, sample); break;
                    case 1: sample = obsidian.processPost(ch, sample); break;
                    case 2: sample = amethyst.processPost(ch, sample); break;
                    case 3: sample = opal.processPost(ch, sample); break;
                    case 4: sample = sapphire.processPost(ch, sample); break;
                    case 5: sample = garnet.processPost(ch, sample); break;               
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

    // Tone Stack

    ToneStack toneStack{
        {
            PEAK(160.0f, 0.6f, low),
            PEAK(600.0f, 0.8f, mid),
            PEAK(2600.0f, 0.8f, high),
            PEAK(6000.0f, 0.8f, presence),
        },{}        
    };    

    // DC Blocker

    ToneStack dcBlocker{
        {
            HP(140.0f),
            HP(140.0f),
            HP(140.0f),
            HP(140.0f),            
        }, {}
    };

    // CLEAN

    ToneStack glass{
        // pre
        {
            HP(55.0f),
            PEAK(870.0f, 0.4f, 4.0f),
            PEAK(3700.0f, 0.32f, 7.0f),
            HS(6000.0f, 0.7f, 16.0f)
        },
        // post
        {
            HP(60.0f),
            PEAK(140.0f, 2.4f, -3.5f),
            PEAK(2200.0f, 0.3f, 4.0f),
            PEAK(2600.0f, 5.8f, -5.1f),
            PEAK(6400.0f, 1.0f, 4.8f),
            LP(10000.0f)
        }
    };

    ToneStack pearl{
        {
            HP(55.0f),
            PEAK(870.0f, 0.4f, 4.0f),
            PEAK(3700.0f, 0.32f, 7.0f),
            HS(6000.0f, 0.7f, 16.0f)
        },
        {
            HP(60.0f),
            PEAK(114.0f, 0.37f, 4.88f),
            PEAK(140.0f, 2.4f, -3.5f),
            PEAK(2200.0f, 0.3f, 4.0f),
            PEAK(2600.0f, 5.8f, -5.1f),
            PEAK(5740.0f, 0.71f, 5.58f),
            PEAK(6400.0f, 1.0f, 4.8f),
            LP(10000.0f)
        }
    };

    ToneStack diamond{
        {
            HP(55.0f),
            PEAK(870.0f, 0.4f, 4.0f),
            PEAK(3700.0f, 0.32f, 7.0f),
            HS(6000.0f, 0.7f, 16.0f)
        },
        {
            HP(60.0f),
            PEAK(104.0f, 0.42f, 5.33f),
            PEAK(140.0f, 2.4f, -3.5f),
            PEAK(527.0f, 0.71f, -6.32f),
            PEAK(2200.0f, 0.3f, 4.0f),
            PEAK(2600.0f, 5.8f, -5.1f),
            PEAK(3230.0f, 0.26f, 6.09f),
            PEAK(6400.0f, 1.0f, 4.8f),
            LP(10000.0f)
        }
    };

    ToneStack zircon{
        {
            HP(55.0f),
            PEAK(870.0f, 0.4f, 4.0f),
            PEAK(3700.0f, 0.32f, 7.0f),
            HS(6000.0f, 0.7f, 16.0f)
        },
        {
            HP(60.0f),
            PEAK(85.0f, 0.71f, 5.33f),
            PEAK(140.0f, 2.4f, -3.5f),
            PEAK(1820.0f, 0.26f, 5.08f),
            PEAK(2200.0f, 0.3f, 4.0f),
            PEAK(2600.0f, 5.8f, -5.1f),
            PEAK(6400.0f, 1.0f, 4.8f),
            PEAK(8280.0f, 0.28f, 5.71f),
            LP(10000.0f)
        }
    };

    // DIRTY

    ToneStack slate{
       {
            HP(82.0f),
            PEAK(650.0f, 1.2f, 2.5f),
            PEAK(2200.0f, 0.8f, 3.2f),
            HS(4500.0f, 0.7f, 6.0f)
       },
       {
            HP(40.0f),
            LS(45.0f, 0.5f, 6.0f),
            PEAK(75.1f, 0.46f, 4.38f),
            PEAK(120.0f, 0.5f, 8.0f),
            LS(150.0f, 0.7f, -2.0f),
            PEAK(160.0f, 0.6f, 2.0f),
            PEAK(505.0f, 0.93f, -2.14f),
            PEAK(750.0f, 1.4f, 4.0f),
            PEAK(1800.0f, 1.0f, 2.8f),
            PEAK(2350.0f, 0.69f, 2.35f),
            PEAK(2500.0f, 2.11f, 1.86f),
            PEAK(3200.0f, 1.2f, -1.5f),
            PEAK(3638.0f, 1.156f, 4.616f),
            PEAK(3670.0f, 1.14f, 3.48f),
            PEAK(4000.0f, 5.02f, -2.31f),
            PEAK(5000.0f, 0.8f, 2.0f),
            HS(6500.0f, 0.7f, -1.8f),
            PEAK(7842.0f, 0.938f, 4.032f),
            PEAK(8610.0f, 0.71f, 2.60f),
            HS(9000.0f, 0.6f, 3.5f),
            LP(12000.0f),
       }
    };

    ToneStack obsidian{
        {
            HP(60.0f),
            PEAK(520.0f, 0.9f, 2.8f),
            PEAK(1400.0f, 0.7f, 3.2f),
            PEAK(3800.0f, 1.1f, 4.5f),
        },
        {
            HP(32.0f),
            HP(32.0f),
            RBJ_LS(120.0f, 0.5f, 11.712f),
            PEAK(129.0f, 1.74f, 5.86f),
            PEAK(180.0f, 0.813f, 3.0f),
            PEAK(232.0f, 1.58f, -2.21f),
            PEAK(528.0f, 0.71f, -9.0f),
            PEAK(600.0f, 0.8f, 3.84f),
            PEAK(1100.0f, 1.374f, -5.872f),
            PEAK(1190.0f, 0.71f, 2.11f),
            PEAK(1787.0f, 0.813, -2.496f),
            PEAK(2185.0f, 0.437f, 7.686f),
            PEAK(2350.0f, 0.71f, 2.85f),    
            PEAK(2770.0f, 4.24f, -5.50f),        
            PEAK(2900.0f, 2.03f, 3.60f),
            PEAK(3740.0f, 4.43f, -1.76f),
            HS(6000.0f, 0.8f, 5.0f),
            HS(7760.0f, 0.813f, 2.5f),
            PEAK(8678.0f, 0.688f, 3.532f),
            PEAK(8990.0f, 0.71f, 0.4f),
            LP(17122.0f)
        }
    };

    ToneStack amethyst{
        {
            HP(60.0f),
            PEAK(500.0f, 1.1f, 2.8f),
            PEAK(1600.0f, 0.9f, 3.5f),
            PEAK(4000.0f, 1.3f, 4.0f),
        },
        {
            HP(45.0f),
            LS(45.0f, 0.5f, 6.0f),
            PEAK(58.7f, 0.53f, 8.33f),
            PEAK(120.0f, 0.5f, 8.0f),
            PEAK(180.0f, 1.2f, -1.8f),
            PEAK(400.0f, 0.71f, -2.85f),
            PEAK(600.0f, 1.0f, 3.2f),
            PEAK(1000.0f, 0.93f, 2.74f),            
            PEAK(1200.0f, 0.8f, 2.5f),
            PEAK(2800.0f, 1.4f, -2.0f),
            PEAK(3590.0f, 1.27f, 2.29f),
            PEAK(4500.0f, 1.1f, 4.5f),
            PEAK(6500.0f, 0.9f, 2.8f),
            HS(8500.0f, 0.7f, 3.5f),
            LP(13000.0f),
            PEAK(13500.0f, 0.71f, -3.40f)
        }
    };

    ToneStack opal{
        {
            HP(90.0f),
            PEAK(720.0f, 1.5f, 3.8f),
            PEAK(2000.0f, 1.0f, 2.5f),
            PEAK(5500.0f, 1.8f, 5.2f)
        },
        {
            HP(40.0f),
            HP(45.0f),
            LS(45.0f, 0.5f, 6.0f),
            PEAK(84.0f, 0.5f, 6.29f),
            PEAK(120.0f, 0.5f, 8.0f),
            PEAK(200.0f, 1.5f, -3.2f),
            PEAK(800.0f, 1.2f, 4.5f),
            PEAK(1020.0f, 1.06f, 3.05f),
            PEAK(1600.0f, 0.9f, 2.8f),
            PEAK(3200.0f, 2.0f, -2.8f),
            PEAK(3230.0f, 0.71f, 5.33f),
            PEAK(5800.0f, 1.3f, 4.8f),
            HS(7500.0f, 0.8f, -2.5f),
            PEAK(8250.0f, 0.71f, 2.36f),
            LP(11500.0f)
        }
    };

    ToneStack sapphire{
        {
            HP(85.0f),
            PEAK(680.0f, 1.4f, 4.2f),
            PEAK(1800.0f, 0.9f, 3.5f),
            PEAK(4800.0f, 1.6f, 5.8f),
        },
        {
            HP(45.0f),
            LS(45.0f, 0.5f, 6.0f),
            PEAK(84.0f, 0.5f, 6.29f),
            PEAK(120.0f, 0.5f, 8.0f),
            PEAK(150.0f, 1.2f, -2.5f),
            PEAK(750.0f, 1.1f, 4.8f),
            PEAK(1500.0f, 0.8f, 2.8f),
            PEAK(2670.0f, 1.72f, 3.84f),
            PEAK(2800.0f, 1.8f, -2.2f),
            PEAK(4000.0f, 6.98f, -5.02f),
            PEAK(5200.0f, 1.2f, 5.5f),
            PEAK(7200.0f, 0.9f, 3.2f),
            HS(9000.0f, 0.7f, -1.8f),
            PEAK(10000.0f, 0.52f, 6.31f),
            LP(12800.0f)
        }   
    };

    ToneStack garnet{
        {
            HP(50.0f),
            PEAK(320.0f, 0.8f, 2.2f),
            PEAK(1000.0f, 0.7f, 2.8f),
            PEAK(2800.0f, 0.6f, 3.8f),
        },
        {
            HP(45.0f),
            LS(45.0f, 0.5f, 6.0f),
            LS(100.0f, 0.7f, 2.0f),
            PEAK(120.0f, 0.5f, 8.0f),
            PEAK(160.0f, 0.71f, 0.87f),
            PEAK(360.0f, 0.9f, 2.5f),
            PEAK(599.0f, 1.15f, -1.86f),
            PEAK(850.0f, 0.8f, 2.8f),
            PEAK(1560.0f, 0.71f, 1.86f),
            PEAK(1600.0f, 0.9f, 2.2f),
            PEAK(3200.0f, 1.1f, 3.5f),
            PEAK(3440.0f, 2.14f, -1.67f),
            PEAK(5500.0f, 0.8f, 3.8f),
            PEAK(6960.0f, 0.71f, 2.85f),
            HS(7500.0f, 0.7f, 4.5f),
            LP(16000.0f)
        }
    };

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

        // toneStack
        toneStack.reset();
        toneStack.update(scaledSampleRate);

        // dcBlocker
        dcBlocker.reset();
        dcBlocker.update(scaledSampleRate);

        // clean
        glass.reset();
        glass.update(scaledSampleRate);

        pearl.reset();
        pearl.update(scaledSampleRate);

        diamond.reset();
        diamond.update(scaledSampleRate);

        zircon.reset();
        zircon.update(scaledSampleRate);

        // dirty
        slate.reset();
        slate.update(scaledSampleRate); 

        obsidian.reset();
        obsidian.update(scaledSampleRate);  

        amethyst.reset();
        amethyst.update(scaledSampleRate);   

        opal.reset();
        opal.update(scaledSampleRate);    

        sapphire.reset();
        sapphire.update(scaledSampleRate);    

        garnet.reset();
        garnet.update(scaledSampleRate);            
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
                sample = toneStack.processPre(0, sample);
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
                sample = toneStack.processPre(0, sample);
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
            
            y = toneStack.processPre(0, y);

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