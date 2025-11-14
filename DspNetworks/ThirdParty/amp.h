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
        // pre & post sculpt filters
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < numCleanPreSculptStates; ++i) { cleanPreSculptStates[ch][i].reset(); }
            for (int i = 0; i < numCleanPostSculptStates; ++i) { cleanPostSculptStates[ch][i].reset(); }
            for (int i = 0; i < numDirtyPreSculptStates; ++i) { dirtyPreSculptStates[ch][i].reset(); }
            for (int i = 0; i < numDirtyPostSculptStates; ++i) { dirtyPostSculptStates[ch][i].reset(); }
            for (int i = 0; i < numToneStackStates; ++i) { toneStackStates[ch][i].reset(); }
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

    float dirtyLowBandMem[2] = {0.0f, 0.0f};
    float dirtyPresenceMem[2] = {0.0f, 0.0f};

    static const int numCleanPreSculptStates = 4;
    static const int numCleanPostSculptStates = 6;
    static const int numDirtyPreSculptStates = 4;
    static const int numDirtyPostSculptStates = 16;
    static const int numToneStackStates = 4;    

    BiquadState cleanPreSculptStates[2][numCleanPreSculptStates];
    BiquadState cleanPostSculptStates[2][numCleanPostSculptStates];

    BiquadState dirtyPreSculptStates[2][numDirtyPreSculptStates];
    BiquadState dirtyPostSculptStates[2][numDirtyPostSculptStates];

    BiquadState toneStackStates[2][numToneStackStates];
    
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

    // nam 
    
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
    bool modelLoaded = false;
    juce::String modelPath = "";
    
    using Model = wavenet::Wavenet_Model<float,
        1,
        wavenet::Layer_Array<float, 1, 1, 8, 16, 3, Dilations, false, NAMMathsProvider>,
        wavenet::Layer_Array<float, 16, 1, 1, 8, 3, Dilations, true, NAMMathsProvider>>;
    
    std::shared_ptr<Model> modelShared;
    std::atomic<bool> isLoading { false };
    std::atomic<bool> needsWarmup { false };

    // nam resampler
               
    int resamplerMode = 0; // default lagrange
    int resamplerKernelSize = 32; // used by lanczos and sinc
    bool allowUpsampleResampling = false; // allow resampling when host < model sample rate (disable by default)
    std::atomic<double> modelSampleRate { 48000.0 }; // most nam models are trained at 48000
    std::atomic<bool>   resamplerConfig { true };
        
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
    
    UpsampleAAFilter upsampleAAFilter;

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

    StreamingResampler inputResampler;   // host -> model
    StreamingResampler outputResampler;  // model -> host
    std::vector<float> scratchUp;        // upsampled input for NAM
    std::vector<float> scratchDown;      // NAM output resampled back
    std::vector<float> scratchNAM;       // NAM processing buffer (mono)
    std::vector<float> scratchPreFilter; // Pre-filtered input for HQ upsampling

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
                for (int f = 0; f < numCleanPreSculptStates; ++f) { sample = cleanPreSculptStates[ch][f].process(sample); } // pre sculpt                                                                    
                sample = getCleanSample(sample); // transfer function  
                for (int f = 0; f < numToneStackStates; ++f) { sample = toneStackStates[ch][f].process(sample); } // tone stack
                for (int f = 0; f < numCleanPostSculptStates; ++f) { sample = cleanPostSculptStates[ch][f].process(sample); } // post sculpt
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
                for (int f = 0; f < numDirtyPreSculptStates; ++f) { sample = dirtyPreSculptStates[ch][f].process(sample); } // pre sculpt                                                                    
                sample = getDirtySample(sample, dirtyLowBandMem[ch], dirtyPresenceMem[ch]); // transfer function
                for (int f = 0; f < numToneStackStates; ++f) { sample = toneStackStates[ch][f].process(sample); } // tone stack
                for (int f = 0; f < numDirtyPostSculptStates; ++f) { sample = dirtyPostSculptStates[ch][f].process(sample); } // post sculpt
                sample *= Decibels::decibelsToGain(outputGain); // output gain
                
                channelData[s] = sample;
            }
        }
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
    
    void warmupNAM(Model& m)
    {
        // calls forward on a bunch of 0 samples then discards them to warmup the model
        for (int i = 0; i < 2048; ++i) { (void)m.forward(0.0f); }            
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
    
    float getDirtySample(float input, float& lowBandMem, float& presenceMem)
    {
        // uses 2 separate processors for low & high frequencies
        float y = input;

        const float crossover = 0.15f;
        lowBandMem = lowBandMem + crossover * (y - lowBandMem);
        float highBand = y - lowBandMem;        
        float lowSat = std::tanh(lowBandMem * 2.8f) * 0.9f;
        
        float highDrive = highBand * 4.5f;
        float highSat;
        if (highDrive > 0.0f) {
            highSat = 1.0f - std::exp(-highDrive * 2.0f);
        } else {
            highSat = -(1.0f - std::exp(highDrive * 1.8f));
        }
        highSat = highSat * 0.4f;

        y = lowSat + highSat;
        
        const float ceiling = 0.95f;
        if (std::abs(y) > ceiling) {
            float sign = y > 0.0f ? 1.0f : -1.0f;
            float excess = std::abs(y) - ceiling;
            y = sign * (ceiling + excess / (1.0f + excess * 8.0f));
        }
        
        float presenceDiff = y - presenceMem;
        presenceMem = y;
        y = y + presenceDiff * 0.08f;

        return y * 0.91f;
    }

    void updateCleanSculptCoefficients()
    {
        // [f, q, gain]
        // pre 
        makeHighPassStereo(cleanPreSculptStates[0][0], cleanPreSculptStates[1][0], 55.0f, scaledSampleRate);
        makePeakFilterStereo(cleanPreSculptStates[0][1], cleanPreSculptStates[1][1], 870.0f, 0.4f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(cleanPreSculptStates[0][2], cleanPreSculptStates[1][2], 3700.0f, 0.32f, 7.0f, scaledSampleRate);
        makeHighShelfStereo(cleanPreSculptStates[0][3], cleanPreSculptStates[1][3], 6000.0f, 0.7f, 16.0f, scaledSampleRate);

        // post 
        makeHighPassStereo(cleanPostSculptStates[0][0], cleanPostSculptStates[1][0], 60.0f, scaledSampleRate);
        makePeakFilterStereo(cleanPostSculptStates[0][1], cleanPostSculptStates[1][1], 140.0f, 2.4f, -3.5f, scaledSampleRate);
        makePeakFilterStereo(cleanPostSculptStates[0][2], cleanPostSculptStates[1][2], 2200.0f, 0.3f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(cleanPostSculptStates[0][3], cleanPostSculptStates[1][3], 2600.0f, 5.8f, -5.1f, scaledSampleRate);
        makePeakFilterStereo(cleanPostSculptStates[0][4], cleanPostSculptStates[1][4], 6400.0f, 1.0f, 4.8f, scaledSampleRate);
        makeLowPassStereo(cleanPostSculptStates[0][5], cleanPostSculptStates[1][5], 10000.0f, scaledSampleRate);
    }

    void updateDirtySculptCoefficients()
    {
        // [f, q, gain]
        // pre 
        makeHighPassStereo(dirtyPreSculptStates[0][0], dirtyPreSculptStates[1][0], 55.0f, scaledSampleRate);
        makePeakFilterStereo(dirtyPreSculptStates[0][1], dirtyPreSculptStates[1][1], 870.0f, 0.4f, 4.0f, scaledSampleRate);
        makePeakFilterStereo(dirtyPreSculptStates[0][2], dirtyPreSculptStates[1][2], 3700.0f, 0.32f, 7.0f, scaledSampleRate);
        makeHighShelfStereo(dirtyPreSculptStates[0][3], dirtyPreSculptStates[1][3], 6000.0f, 0.7f, 16.0f, scaledSampleRate);        

        // post 
        makeHighPassStereo(dirtyPostSculptStates[0][0], dirtyPostSculptStates[1][0], 25.0f, scaledSampleRate);
        makeHighPassStereo(dirtyPostSculptStates[0][1], dirtyPostSculptStates[1][1], 60.0f, scaledSampleRate);
        makeLowShelfStereo(dirtyPostSculptStates[0][2], dirtyPostSculptStates[1][2], 120.0f, 0.17f, 10.0f, scaledSampleRate);        
        makePeakFilterStereo(dirtyPostSculptStates[0][3], dirtyPostSculptStates[1][3], 270.0f, 0.71f, 3.17f, scaledSampleRate);
        makePeakFilterStereo(dirtyPostSculptStates[0][4], dirtyPostSculptStates[1][4], 426.0f, 2.4f, -4.38f, scaledSampleRate);
        makePeakFilterStereo(dirtyPostSculptStates[0][5], dirtyPostSculptStates[1][5], 958.0f, 1.09f, -2.11f, scaledSampleRate);
        makePeakFilterStereo(dirtyPostSculptStates[0][6], dirtyPostSculptStates[1][6], 1000.0f, 0.71f, 2.95f, scaledSampleRate);
        makePeakFilterStereo(dirtyPostSculptStates[0][7], dirtyPostSculptStates[1][7], 2002.0f, 1.86f, 3.52f, scaledSampleRate);
        makePeakFilterStereo(dirtyPostSculptStates[0][8], dirtyPostSculptStates[1][8], 2300.0f, 0.3f, 6.0f, scaledSampleRate);
        makePeakFilterStereo(dirtyPostSculptStates[0][9], dirtyPostSculptStates[1][9], 2600.0f, 5.8f, -3.1f, scaledSampleRate);
        makePeakFilterStereo(dirtyPostSculptStates[0][10], dirtyPostSculptStates[1][10], 2950.0f, 0.71f, 4.00f, scaledSampleRate);
        makePeakFilterStereo(dirtyPostSculptStates[0][11], dirtyPostSculptStates[1][11], 4600.0f, 3.0f, 2.00f, scaledSampleRate);
        makePeakFilterStereo(dirtyPostSculptStates[0][12], dirtyPostSculptStates[1][12], 6400.0f, 1.0f, 4.8f, scaledSampleRate);
        makePeakFilterStereo(dirtyPostSculptStates[0][13], dirtyPostSculptStates[1][13], 7740.0f, 1.46f, 2.55f, scaledSampleRate);
        makeHighShelfStereo(dirtyPostSculptStates[0][14], dirtyPostSculptStates[1][14], 5390.0f, 0.27f, -4.10f, scaledSampleRate);        
        makeHighShelfStereo(dirtyPostSculptStates[0][15], dirtyPostSculptStates[1][15], 7740.0f, 0.71f, 9.0f, scaledSampleRate);                
    }

    void updateToneStack()
    {
        makePeakFilterStereo(toneStackStates[0][0], toneStackStates[1][0], 160.0f, 0.6f, low, scaledSampleRate);
        makePeakFilterStereo(toneStackStates[0][1], toneStackStates[1][1], 600.0f, 0.8f, mid, scaledSampleRate);
        makePeakFilterStereo(toneStackStates[0][2], toneStackStates[1][2], 2600.0f, 0.8f, high, scaledSampleRate);
        makeHighShelfStereo(toneStackStates[0][3], toneStackStates[1][3], 6000.0f, 0.8f, presence, scaledSampleRate);
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

        updateCleanSculptCoefficients();
        updateDirtySculptCoefficients();
        updateToneStack();
    }
   
};
}