// ==================================| Third Party Node Template |==================================

#pragma once
#include <JuceHeader.h>

// Switch RTNeural backend to xsimd to avoid Eigen crashes
#define RTNEURAL_USE_EIGEN 0
#define RTNEURAL_USE_XSIMD 1
#define RTNEURAL_DEFAULT_ALIGNMENT 16

#include <atomic>
#include <memory>
#include <thread>

#include "src/nlohmann/json.hpp"
#include "src/RTNeural/RTNeural.h"
#include "src/RTNeural-NAM/wavenet/wavenet_model.hpp"
#include "src/math_approx/math_approx.hpp"

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
using cable_manager_t = routing::global_cable_cpp_manager<SN_GLOBAL_CABLE(110245659),
                                                          SN_GLOBAL_CABLE(106677056),
                                                          SN_GLOBAL_CABLE(108826)>;


// ==========================| The node class with all required callbacks |==========================

/* pretty much all of this is vibe-coded/regenerated because I had to transfer it from my original JUCE version */

template <int NV> struct amp: public data::base, public cable_manager_t
{
    SNEX_NODE(amp);
    
    struct MetadataClass
    {
        SN_NODE_ID("amp");
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


    amp()
    {        
        // Load NAM from global cable asynchronously and atomically swap the model when ready
        this->registerDataCallback<GlobalCablesAmp::nam>([this](const var& data)
        {
            this->loadNAMModelFromJSONAsync(data);
        });
    }
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate   = static_cast<float>(specs.sampleRate);
        numChannels  = specs.numChannels;
        maxBlockSize = specs.blockSize;
        
        // Store specs for later use
        lastSpecs.sampleRate        = specs.sampleRate;
        lastSpecs.numChannels       = specs.numChannels;
        lastSpecs.maximumBlockSize  = specs.blockSize;
        
        // Initialize oversampling
        updateOversampling();
        
        // Prepare current NAM model (host not running audio here)
        if (auto m = std::atomic_load(&modelShared))
        {
            m->prepare(static_cast<int>(lastSpecs.maximumBlockSize));
            // Safe warmup using forward() calls (no Eigen prewarm)
            safeWarmup(*m);
            needsWarmup.store(false, std::memory_order_release);
            modelLoaded = true;
        }        
        
        // Reset all filters
        reset();        
    }           

    void reset() 
    {
        // Reset pre and post sculpt filters
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < numPreSculptStates; ++i)
                preSculptStates[ch][i].reset();
            for (int i = 0; i < numPostSculptStates; ++i)
                postSculptStates[ch][i].reset();
            for (int i = 0; i < numToneStackStates; ++i)                
                toneStackStates[ch][i].reset();
        }
        
        // Reset oversampling if active
        if (oversamplingFactor > 0 && oversampling != nullptr)
        {
            oversampling->reset();
        }
    }
    
    template <typename T> void process(T& data)
    {
        int numSamples = data.getNumSamples();
        size_t numCh = juce::jmin(2, (int)data.getNumChannels());
        
        // Convert to AudioBuffer for easier manipulation
        AudioBuffer<float> buffer((int)numCh, numSamples);
        
        int channelIndex = 0;
        for(auto ch: data)
        {
            if (channelIndex >= (int)numCh) break;
            dyn<float> channel = data.toChannelData(ch);
            for (int s = 0; s < numSamples; ++s)
            {
                buffer.setSample(channelIndex, s, channel[s]);
            }
            channelIndex++;
        }
        
        // Process the buffer
        processAudioBuffer(buffer);
        
        // Write back to data
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

    void setExternalData(const ExternalData& data, int index) 
    {
        
    }
    
    template <int P> void setParameter(double v)
    {
        switch (P)
        {
        // Amp Mode & Oversampling
        case 0: ampMode = static_cast<int>(v); break;                 // 0=clean, 1=dirty, 2=NAM
        case 1: 
        {
            int oldFactor = oversamplingFactor;
            oversamplingFactor = static_cast<int>(v);
            if (oldFactor != oversamplingFactor)
                updateOversampling();
            break;
        }

        // Amp Parameters
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
        // Amp Mode
        {
            parameter::data mode("Mode", { 0.0, 2.0 });
            registerCallback<0>(mode);
            mode.setDefaultValue(1.0);
            data.add(std::move(mode));
        }
        
        // Oversampling Factor
        {
            parameter::data oversamp("Oversampling", { 0.0, 3.0 });
            registerCallback<1>(oversamp);
            oversamp.setDefaultValue(1.0);
            data.add(std::move(oversamp));
        }

        // Amp Parameters
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
    
    // Public method to load NAM models from file (async and safe)
    void loadNAMModel(const juce::File& file)
    {
        if (!file.existsAsFile())
        {
            DBG("Invalid file for NAM loader.");
            return;
        }
        if (isLoading.exchange(true))
            return; // already loading

        const std::string path = file.getFullPathName().toStdString();

        std::thread([this, path]()
        {
            try
            {
                nlohmann::json modelJson{};
                std::ifstream{ path, std::ifstream::binary } >> modelJson;

                auto newModel = std::make_shared<Model>();
                newModel->load_weights(modelJson);
                // Prepare now (safe off-thread)
                newModel->prepare(static_cast<int>(lastSpecs.maximumBlockSize));

                std::atomic_store(&modelShared, std::move(newModel));
                modelLoaded = true;
                modelPath = juce::String(path);

                // Request a warmup on next cycle (will run via forward())
                needsWarmup.store(true, std::memory_order_release);

                DBG("Loaded NAM model from file successfully.");
            }
            catch (const std::exception& e)
            {
                DBG("Exception: " << e.what());
                modelLoaded = false;
            }
            catch (...)
            {
                DBG("Unknown exception caught");
                modelLoaded = false;
            }

            isLoading.store(false);
        }).detach();
    }
    
    juce::String returnModelPath()
    {
        return modelPath;
    }

private:
    // Audio processing
    float sampleRate = 44100.0f;
    int numChannels = 2;
    int maxBlockSize = 512;
    
    // Mode
    int ampMode = 1; // 0=clean, 1=dirty, 2=NAM
    
    // Oversampling
    int oversamplingFactor = 1;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    float scaledSampleRate = 44100.0f;
    
    // Store specs
    struct ProcessSpec
    {
        double sampleRate = 44100.0;
        uint32 numChannels = 2;
        uint32 maximumBlockSize = 512;
    };
    ProcessSpec lastSpecs;
    ProcessSpec specsOversampling;

    static const int numPreSculptStates = 4;
    static const int numPostSculptStates = 6;
    static const int numToneStackStates = 4;

    // Amp Parameters
    float inputGain = 0.0f;
    float low = 0.0f;
    float mid = 0.0f;
    float high = 0.0f;
    float presence = 0.0f;
    float outputGain = 0.0f;
    
    // Biquad Filter State
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
    
    // Filter states [channel][filter]
    BiquadState preSculptStates[2][numPreSculptStates];      // 4 filters
    BiquadState postSculptStates[2][numPostSculptStates];     // 6 filters    
    BiquadState toneStackStates[2][numToneStackStates];

    // Async JSON loader (thread-safe). Builds a new model off-thread, then atomically swaps it in.
    void loadNAMModelFromJSONAsync(const var& jsonData)
    {
        // Serialize JUCE var to string on this thread (UI callback)
        juce::String jsonString;
        if (jsonData.isObject())
        {
            jsonString = juce::JSON::toString(jsonData);
        }
        else if (jsonData.isString())
        {
            jsonString = jsonData.toString();
        }
        else
        {
            DBG("Invalid JSON data format for NAM loader.");
            return;
        }

        if (jsonString.isEmpty())
            return;

        if (isLoading.exchange(true))
            return; // already loading

        const std::string jsonCopy = jsonString.toStdString();

        std::thread([this, jsonCopy]()
        {
            try
            {
                nlohmann::json modelJson = nlohmann::json::parse(jsonCopy);

                auto newModel = std::make_shared<Model>();
                newModel->load_weights(modelJson);
                // Prepare now (safe off-thread)
                newModel->prepare(static_cast<int>(lastSpecs.maximumBlockSize));

                std::atomic_store(&modelShared, std::move(newModel));
                modelLoaded = true;
                modelPath = {}; // memory-loaded

                // Request a warmup on next cycle (will run via forward())
                needsWarmup.store(true, std::memory_order_release);

                DBG("Loaded NAM model from JSON successfully.");
            }
            catch (const std::exception& e)
            {
                DBG("Exception loading NAM model: " << e.what());
                modelLoaded = false;
            }
            catch (...)
            {
                DBG("Unknown exception loading NAM model");
                modelLoaded = false;
            }

            isLoading.store(false);
        }).detach();
    }
    
    // NAM Math provider
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

    // Atomically swappable model instance
    std::shared_ptr<Model> modelShared;
    std::atomic<bool> isLoading { false };

    std::atomic<bool> needsWarmup { false }; // run warmup via forward() at next opportunity
    
    // Main processing function

    void processAudioBuffer(juce::AudioBuffer<float>& buffer)
    {
        // Use oversampling only for Clean/Dirty (ampMode < 2). NAM (2) is processed at native rate.
        const bool useOversampling = (oversamplingFactor > 0 && oversampling != nullptr && ampMode < 2);
        juce::dsp::AudioBlock<float> inBlock(buffer);

        if (useOversampling)
        {
            // Upsample into the oversampler's internal buffer
            auto upBlock = oversampling->processSamplesUp(inBlock);

            const int upCh     = static_cast<int>(upBlock.getNumChannels());
            const int upFrames = static_cast<int>(upBlock.getNumSamples());

            juce::HeapBlock<float*> chanPtrs(upCh);
            for (int ch = 0; ch < upCh; ++ch)
                chanPtrs[ch] = upBlock.getChannelPointer(static_cast<size_t>(ch));

            // Non-owning AudioBuffer view over the upsampled data
            juce::AudioBuffer<float> osBuffer(chanPtrs.getData(), upCh, upFrames);

            // Process based on amp mode at the upsampled rate
            if (ampMode == 0)           // Clean
                processCleanMode(osBuffer, osBuffer.getNumSamples());
            else                        // Dirty
                processDirtyMode(osBuffer, osBuffer.getNumSamples());

            // Downsample back into the original buffer
            oversampling->processSamplesDown(inBlock);
        }
        else
        {
            // No oversampling path (or NAM mode)
            const int numSamples = buffer.getNumSamples();

            if (ampMode == 0)           // Clean
                processCleanMode(buffer, numSamples);
            else if (ampMode == 1)      // Dirty
                processDirtyMode(buffer, numSamples);
            else if (ampMode == 2)      // NAM
                processNAMMode(buffer, numSamples);
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
                
                // Pre-sculpt
                for (int f = 0; f < 4; ++f)
                    sample = preSculptStates[ch][f].process(sample);
                
                // Tone stack
                for (int f = 0; f < 4; ++f)
                    sample = toneStackStates[ch][f].process(sample);
                
                // Input gain and clean processing                
                sample *= Decibels::decibelsToGain(inputGain);
                sample = getCleanSample(sample);
                sample *= Decibels::decibelsToGain(outputGain);
                
                // Post-sculpt
                for (int f = 0; f < 6; ++f)
                    sample = postSculptStates[ch][f].process(sample);
                
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
                
                // Pre-sculpt
                for (int f = 0; f < 4; ++f)
                    sample = preSculptStates[ch][f].process(sample);
                
                // Tone stack
                for (int f = 0; f < 4; ++f)
                    sample = toneStackStates[ch][f].process(sample);
                
                // Input gain and dirty processing
                sample *= Decibels::decibelsToGain(inputGain);
                sample = getDirtySample(sample);
                sample *= Decibels::decibelsToGain(outputGain);
                
                // Post-sculpt
                for (int f = 0; f < 6; ++f)
                    sample = postSculptStates[ch][f].process(sample);
                
                channelData[s] = sample;
            }
        }
    }
    
    void processNAMMode(AudioBuffer<float>& buffer, int numSamples)
    {
        // NAM processes mono only
        float* channelData = buffer.getWritePointer(0);
        auto m = std::atomic_load(&modelShared);

        // If a model was just swapped in, warm it once via forward() calls
        if (m && needsWarmup.load(std::memory_order_acquire))
        {
            bool expected = true;
            if (needsWarmup.compare_exchange_strong(expected, false, std::memory_order_acq_rel))
                safeWarmup(*m);
        }
        
        for (int s = 0; s < numSamples; ++s)
        {
            float sample = channelData[s];
                                
            // NAM processing
            if (m)
            {
                sample *= Decibels::decibelsToGain(inputGain);
                sample = m->forward(sample);
                sample *= Decibels::decibelsToGain(outputGain);
            }

            // Tone stack comes after the neural model (otherwise it does nothing)
            for (int f = 0; f < 4; ++f)
                sample = toneStackStates[0][f].process(sample);
            
            channelData[s] = sample;
        }
        
        // Copy to right channel (force mono)
        if (buffer.getNumChannels() > 1)
        {
            memcpy(buffer.getWritePointer(1), buffer.getReadPointer(0), 
                   (size_t)numSamples * sizeof(float));
        }
    }

    // Warmup helper using forward() to avoid Eigen internals
    void safeWarmup(Model& m)
    {
        for (int i = 0; i < 2048; ++i)
            (void)m.forward(0.0f);
    }
    
    // Amp algorithms
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
    
    float getDirtySample(float input)
    {
        static const float pi = juce::MathConstants<float>::pi;
        float threshold = 0.7f;
        float y = 0.0f;
        
        // Stage 1
        if (input > 0.0f)
            y = (2.0f / pi) * std::atan(input * 1.5f);
        else
        {
            y = std::tanh(input * 2.2f);
            y *= 0.85f;
        }
        
        // Stage 2
        if (std::abs(y) > threshold)
        {
            float sign = (y > 0.0f) ? 1.0f : -1.0f;
            float excess = std::abs(y) - threshold;
            float clipped = threshold + std::tanh(excess * 8.0f) * 0.2f;
            y = sign * clipped;
        }
        
        return y;
    }
    
    // Filter coefficient calculations
    void setBiquadCoeffs(BiquadState& state, 
                        float b0, float b1, float b2, 
                        float a0, float a1, float a2)
    {
        state.b0 = b0 / a0;
        state.b1 = b1 / a0;
        state.b2 = b2 / a0;
        state.a1 = a1 / a0;
        state.a2 = a2 / a0;
    }

    // Stereo-safe helpers (apply same coeffs to Left and Right)
    void setBiquadCoeffsStereo(BiquadState& L, BiquadState& R,
                               float b0, float b1, float b2,
                               float a0, float a1, float a2)
    {
        setBiquadCoeffs(L, b0, b1, b2, a0, a1, a2);
        setBiquadCoeffs(R, b0, b1, b2, a0, a1, a2);
    }

    void makePeakFilterStereo(BiquadState& L, BiquadState& R, float freq, float Q, float gainDB)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / scaledSampleRate;
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
        
        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
    }
    
    void makeHighShelfStereo(BiquadState& L, BiquadState& R, float freq, float Q, float gainDB)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / scaledSampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float A = std::pow(10.0f, gainDB / 40.0f);
        float beta = std::sqrt(A) / Q;
        
        float b0 = A * ((A + 1.0f) + (A - 1.0f) * cosOmega + beta * sinOmega);
        float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosOmega);
        float b2 = A * ((A + 1.0f) + (A - 1.0f) * cosOmega - beta * sinOmega);
        float a0 = (A + 1.0f) - (A - 1.0f) * cosOmega + beta * sinOmega;
        float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosOmega);
        float a2 = (A + 1.0f) - (A - 1.0f) * cosOmega - beta * sinOmega;
        
        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
    }
    
    void makeHighPassStereo(BiquadState& L, BiquadState& R, float freq)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / scaledSampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float alpha = sinOmega / (2.0f * 0.707f);
        
        float b0 = (1.0f + cosOmega) / 2.0f;
        float b1 = -(1.0f + cosOmega);
        float b2 = (1.0f + cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;
        
        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
    }
    
    void makeLowPassStereo(BiquadState& L, BiquadState& R, float freq)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / scaledSampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float alpha = sinOmega / (2.0f * 0.707f);
        
        float b0 = (1.0f - cosOmega) / 2.0f;
        float b1 = 1.0f - cosOmega;
        float b2 = (1.0f - cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;
        
        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
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
        else
        {
            scaledSampleRate = static_cast<float>(lastSpecs.sampleRate);
        }
        
        oversampling.swap(newOversampling);
        updateFilterCoefficients();
    }
    
    void updateFilterCoefficients()
    {
        // Pre-sculpt (apply identical coeffs to L/R for each filter index)
        makeHighPassStereo (preSculptStates[0][0], preSculptStates[1][0], 55.0f);
        makePeakFilterStereo(preSculptStates[0][1], preSculptStates[1][1], 870.0f, 0.4f, 4.0f);
        makePeakFilterStereo(preSculptStates[0][2], preSculptStates[1][2], 3700.0f, 0.32f, 7.0f);
        makeHighShelfStereo(preSculptStates[0][3], preSculptStates[1][3], 6000.0f, 0.7f, 16.0f);
        
        // Post-sculpt
        makeHighPassStereo (postSculptStates[0][0], postSculptStates[1][0], 60.0f);
        makePeakFilterStereo(postSculptStates[0][1], postSculptStates[1][1], 140.0f, 2.4f, -3.5f);
        makePeakFilterStereo(postSculptStates[0][2], postSculptStates[1][2], 2200.0f, 0.3f, 4.0f);
        makePeakFilterStereo(postSculptStates[0][3], postSculptStates[1][3], 2600.0f, 5.8f, -5.1f);
        makePeakFilterStereo(postSculptStates[0][4], postSculptStates[1][4], 6400.0f, 1.0f, 4.8f);
        makeLowPassStereo  (postSculptStates[0][5], postSculptStates[1][5], 10000.0f);
        
        // Update tone stacks last (depends on user params)
        updateToneStack();
    }

    void updateToneStack()
    {
        makePeakFilterStereo(toneStackStates[0][0], toneStackStates[1][0], 160.0f, 0.6f, low);
        makePeakFilterStereo(toneStackStates[0][1], toneStackStates[1][1], 600.0f, 0.8f, mid);
        makePeakFilterStereo(toneStackStates[0][2], toneStackStates[1][2], 2600.0f, 0.8f, high);
        makeHighShelfStereo(toneStackStates[0][3], toneStackStates[1][3], 6000.0f, 0.8f, presence);
    }   
};
}