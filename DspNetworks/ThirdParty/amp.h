// ==================================| Third Party Node Template |==================================

#pragma once
#include <JuceHeader.h>
#include <nlohmann/json.hpp>
#include <RTNeural.h>
#include <wavenet/wavenet_model.hpp>

enum class GlobalCables
{
    pitch = 0,
    nam = 1,
};

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;
using cable_manager_t = routing::global_cable_cpp_manager<SN_GLOBAL_CABLE(106677056),
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
        this->registerDataCallback<GlobalCables::nam>([this](const var& data)
        {
            // thread: https://forum.hise.audio/post/103680
            this->loadNAMModelFromJSON(data);
            //jassertfalse;
        });
        
    }
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate = static_cast<float>(specs.sampleRate);
        numChannels = specs.numChannels;
        maxBlockSize = specs.blockSize;
        
        // Store specs for later use
        lastSpecs.sampleRate = specs.sampleRate;
        lastSpecs.numChannels = specs.numChannels;
        lastSpecs.maximumBlockSize = specs.blockSize;
        
        // Initialize oversampling
        updateOversampling();
        
        // Prepare NAM model if loaded
        if (modelLoaded)
        {
            model.prepare(specs.blockSize);
            model.prewarm();
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
            {
                cleanToneStackStates[ch][i].reset();
                dirtyToneStackStates[ch][i].reset();
                namToneStackStates[ch][i].reset();
            }
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
        
        // Process the buffer
        processAudioBuffer(buffer);
        
        // Write back to data
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

    void setExternalData(const ExternalData& data, int index) 
    {
        
    }
    
    template <int P> void setParameter(double v)
    {
        switch (P)
        {
        // Amp Mode & Oversampling
        case 0: ampMode = static_cast<int>(v); break;
        case 1: 
        {
            int oldFactor = oversamplingFactor;
            oversamplingFactor = static_cast<int>(v);
            if (oldFactor != oversamplingFactor)
                updateOversampling();
            break;
        }
        
        // Clean Mode Parameters (2-7)
        case 2: cleanInputGain = static_cast<float>(v); break;
        case 3: cleanLow = static_cast<float>(v); updateCleanToneStack(); break;
        case 4: cleanMid = static_cast<float>(v); updateCleanToneStack(); break;
        case 5: cleanHigh = static_cast<float>(v); updateCleanToneStack(); break;
        case 6: cleanPresence = static_cast<float>(v); updateCleanToneStack(); break;
        case 7: cleanOutputGain = static_cast<float>(v); break;
        
        // Dirty Mode Parameters (8-13)
        case 8: dirtyInputGain = static_cast<float>(v); break;
        case 9: dirtyLow = static_cast<float>(v); updateDirtyToneStack(); break;
        case 10: dirtyMid = static_cast<float>(v); updateDirtyToneStack(); break;
        case 11: dirtyHigh = static_cast<float>(v); updateDirtyToneStack(); break;
        case 12: dirtyPresence = static_cast<float>(v); updateDirtyToneStack(); break;
        case 13: dirtyOutputGain = static_cast<float>(v); break;
        
        // NAM Mode Parameters (14-19)
        case 14: namInputGain = static_cast<float>(v); break;
        case 15: namLow = static_cast<float>(v); updateNamToneStack(); break;
        case 16: namMid = static_cast<float>(v); updateNamToneStack(); break;
        case 17: namHigh = static_cast<float>(v); updateNamToneStack(); break;
        case 18: namPresence = static_cast<float>(v); updateNamToneStack(); break;
        case 19: namOutputGain = static_cast<float>(v); break;
        }
    }
    
    void createParameters(ParameterDataList& data)
    {       
        // Amp Mode
        {
            parameter::data mode("Amp Mode", { 0.0, 2.0 });
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
        
        // Clean Mode Parameters
        {
            parameter::data inputGain("Clean Input Gain", { -60.0, 60.0 });
            registerCallback<2>(inputGain);
            inputGain.setDefaultValue(0.0);
            data.add(std::move(inputGain));
        }
        {
            parameter::data low("Clean Low", { -15.0, 15.0 });
            registerCallback<3>(low);
            low.setDefaultValue(0.0);
            data.add(std::move(low));
        }
        {
            parameter::data mid("Clean Mid", { -15.0, 15.0 });
            registerCallback<4>(mid);
            mid.setDefaultValue(0.0);
            data.add(std::move(mid));
        }
        {
            parameter::data high("Clean High", { -15.0, 15.0 });
            registerCallback<5>(high);
            high.setDefaultValue(0.0);
            data.add(std::move(high));
        }
        {
            parameter::data presence("Clean Presence", { -15.0, 15.0 });
            registerCallback<6>(presence);
            presence.setDefaultValue(0.0);
            data.add(std::move(presence));
        }
        {
            parameter::data outputGain("Clean Output Gain", { -20.0, 20.0 });
            registerCallback<7>(outputGain);
            outputGain.setDefaultValue(0.0);
            data.add(std::move(outputGain));
        }
        
        // Dirty Mode Parameters
        {
            parameter::data inputGain("Dirty Input Gain", { -60.0, 60.0 });
            registerCallback<8>(inputGain);
            inputGain.setDefaultValue(0.0);
            data.add(std::move(inputGain));
        }
        {
            parameter::data low("Dirty Low", { -15.0, 15.0 });
            registerCallback<9>(low);
            low.setDefaultValue(0.0);
            data.add(std::move(low));
        }
        {
            parameter::data mid("Dirty Mid", { -15.0, 15.0 });
            registerCallback<10>(mid);
            mid.setDefaultValue(0.0);
            data.add(std::move(mid));
        }
        {
            parameter::data high("Dirty High", { -15.0, 15.0 });
            registerCallback<11>(high);
            high.setDefaultValue(0.0);
            data.add(std::move(high));
        }
        {
            parameter::data presence("Dirty Presence", { -15.0, 15.0 });
            registerCallback<12>(presence);
            presence.setDefaultValue(0.0);
            data.add(std::move(presence));
        }
        {
            parameter::data outputGain("Dirty Output Gain", { -20.0, 20.0 });
            registerCallback<13>(outputGain);
            outputGain.setDefaultValue(0.0);
            data.add(std::move(outputGain));
        }
        
        // NAM Mode Parameters
        {
            parameter::data inputGain("NAM Input Gain", { -60.0, 60.0 });
            registerCallback<14>(inputGain);
            inputGain.setDefaultValue(0.0);
            data.add(std::move(inputGain));
        }
        {
            parameter::data low("NAM Low", { -15.0, 15.0 });
            registerCallback<15>(low);
            low.setDefaultValue(0.0);
            data.add(std::move(low));
        }
        {
            parameter::data mid("NAM Mid", { -15.0, 15.0 });
            registerCallback<16>(mid);
            mid.setDefaultValue(0.0);
            data.add(std::move(mid));
        }
        {
            parameter::data high("NAM High", { -15.0, 15.0 });
            registerCallback<17>(high);
            high.setDefaultValue(0.0);
            data.add(std::move(high));
        }
        {
            parameter::data presence("NAM Presence", { -15.0, 15.0 });
            registerCallback<18>(presence);
            presence.setDefaultValue(0.0);
            data.add(std::move(presence));
        }
        {
            parameter::data outputGain("NAM Output Gain", { -20.0, 20.0 });
            registerCallback<19>(outputGain);
            outputGain.setDefaultValue(0.0);
            data.add(std::move(outputGain));
        }
    }
    
    // Public method to load NAM models
    void loadNAMModel(const juce::File& file)
    {
        if (!file.existsAsFile())
        {
            DBG("Invalid file for NAM loader.");
            return;
        }

        try
        {
            nlohmann::json modelJson{};
            modelPath = file.getFullPathName();
            std::string pathRaw = modelPath.toStdString();
            std::ifstream{ pathRaw, std::ifstream::binary } >> modelJson;
            model.load_weights(modelJson);
            modelLoaded = true;

            DBG("Loaded model successfully.");
        }
        catch (const std::exception& e)
        {
            DBG("Exception: " << e.what());
        }
        catch (...)
        {
            DBG("Unknown exception caught");
        }
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
    
    // Clean Mode Parameters
    float cleanInputGain = 0.0f;
    float cleanLow = 0.0f;
    float cleanMid = 0.0f;
    float cleanHigh = 0.0f;
    float cleanPresence = 0.0f;
    float cleanOutputGain = 0.0f;
    
    // Dirty Mode Parameters
    float dirtyInputGain = 0.0f;
    float dirtyLow = 0.0f;
    float dirtyMid = 0.0f;
    float dirtyHigh = 0.0f;
    float dirtyPresence = 0.0f;
    float dirtyOutputGain = 0.0f;
    
    // NAM Mode Parameters
    float namInputGain = 0.0f;
    float namLow = 0.0f;
    float namMid = 0.0f;
    float namHigh = 0.0f;
    float namPresence = 0.0f;
    float namOutputGain = 0.0f;
    
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
    BiquadState cleanToneStackStates[2][numToneStackStates]; // 4 filters
    BiquadState dirtyToneStackStates[2][numToneStackStates]; // 4 filters
    BiquadState namToneStackStates[2][numToneStackStates];   // 4 filters

    void loadNAMModelFromJSON(const var& jsonData)
    {
        try
        {
            // Convert JUCE var to nlohmann::json
            nlohmann::json modelJson;
        
            if (jsonData.isObject())
            {
                // Convert JUCE DynamicObject to JSON string, then parse
                String jsonString = JSON::toString(jsonData);
                modelJson = nlohmann::json::parse(jsonString.toStdString());
            }
            else if (jsonData.isString())
            {
                // If it's already a JSON string
                modelJson = nlohmann::json::parse(jsonData.toString().toStdString());
            }
            else
            {
                DBG("Invalid JSON data format for NAM loader.");
                return;
            }
        
            model.load_weights(modelJson);
            modelLoaded = true;
            modelPath = ""; // Clear file path since we're loading from memory
        
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
    }
    
    // NAM Model
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
    
    wavenet::Wavenet_Model<float,
        1,
        wavenet::Layer_Array<float, 1, 1, 8, 16, 3, Dilations, false, NAMMathsProvider>,
        wavenet::Layer_Array<float, 16, 1, 1, 8, 3, Dilations, true, NAMMathsProvider>>
        model;
    
    // Main processing function
    void processAudioBuffer(AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        size_t numCh = buffer.getNumChannels();
        
        // Handle oversampling
        AudioBuffer<float>* processingBuffer = &buffer;
        AudioBuffer<float> oversampledBuffer;
        bool useOversampling = false;
        
        if (oversamplingFactor > 0 && oversampling != nullptr && ampMode < 2)
        {
            dsp::AudioBlock<float> block(buffer);
            dsp::AudioBlock<float> oversampledBlock = oversampling->processSamplesUp(block);
            
            oversampledBuffer.setSize(oversampledBlock.getNumChannels(), 
                                     oversampledBlock.getNumSamples(), false, false, false);
            
            for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch)
            {
                memcpy(oversampledBuffer.getWritePointer(ch), 
                       oversampledBlock.getChannelPointer(ch),
                       oversampledBlock.getNumSamples() * sizeof(float));
            }
            
            processingBuffer = &oversampledBuffer;
            useOversampling = true;
        }
        
        int procSamples = processingBuffer->getNumSamples();
        
        // Process based on amp mode
        if (ampMode == 0) // Clean
        {
            processCleanMode(*processingBuffer, procSamples);
        }
        else if (ampMode == 1) // Dirty
        {
            processDirtyMode(*processingBuffer, procSamples);
        }
        else if (ampMode == 2) // NAM
        {
            processNAMMode(*processingBuffer, procSamples);
        }
        
        // Downsample if needed
        if (useOversampling)
        {
            dsp::AudioBlock<float> oversampledBlock(*processingBuffer);
            dsp::AudioBlock<float> outputBlock(buffer);
            oversampling->processSamplesDown(outputBlock);
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
                    sample = cleanToneStackStates[ch][f].process(sample);
                
                // Input gain and clean processing
                sample *= Decibels::decibelsToGain(cleanInputGain);
                sample = getCleanSample(sample);
                sample *= Decibels::decibelsToGain(cleanOutputGain);
                
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
                    sample = dirtyToneStackStates[ch][f].process(sample);
                
                // Input gain and dirty processing
                sample *= Decibels::decibelsToGain(dirtyInputGain);
                sample = getDirtySample(sample);
                sample *= Decibels::decibelsToGain(dirtyOutputGain);
                
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
        
        for (int s = 0; s < numSamples; ++s)
        {
            float sample = channelData[s];
                                
            // NAM processing
            if (modelLoaded)
            {
                sample *= Decibels::decibelsToGain(namInputGain);
                sample = model.forward(sample);
                sample *= Decibels::decibelsToGain(namOutputGain);
            }

            // Tone stack comes after the neural model (otherwise it does nothing)
            for (int f = 0; f < 4; ++f)
                sample = namToneStackStates[0][f].process(sample);
            
            channelData[s] = sample;
        }
        
        // Copy to right channel (force mono)
        if (buffer.getNumChannels() > 1)
        {
            memcpy(buffer.getWritePointer(1), buffer.getReadPointer(0), 
                   numSamples * sizeof(float));
        }
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
    
    void makePeakFilter(BiquadState states[2], float freq, float Q, float gainDB)
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
        
        setBiquadCoeffs(states[0], b0, b1, b2, a0, a1, a2);
        setBiquadCoeffs(states[1], b0, b1, b2, a0, a1, a2);
    }
    
    void makeHighShelf(BiquadState states[2], float freq, float Q, float gainDB)
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
        
        setBiquadCoeffs(states[0], b0, b1, b2, a0, a1, a2);
        setBiquadCoeffs(states[1], b0, b1, b2, a0, a1, a2);
    }
    
    void makeHighPass(BiquadState states[2], float freq)
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
        
        setBiquadCoeffs(states[0], b0, b1, b2, a0, a1, a2);
        setBiquadCoeffs(states[1], b0, b1, b2, a0, a1, a2);
    }
    
    void makeLowPass(BiquadState states[2], float freq)
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
        
        setBiquadCoeffs(states[0], b0, b1, b2, a0, a1, a2);
        setBiquadCoeffs(states[1], b0, b1, b2, a0, a1, a2);
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
        updateFilterCoefficients();
    }
    
    void updateFilterCoefficients()
    {
        // Pre-sculpt
        makeHighPass(&preSculptStates[0][0], 55.0f);
        makePeakFilter(&preSculptStates[0][1], 870.0f, 0.4f, 4.0f);
        makePeakFilter(&preSculptStates[0][2], 3700.0f, 0.32f, 7.0f);
        makeHighShelf(&preSculptStates[0][3], 6000.0f, 0.7f, 16.0f);
        
        // Post-sculpt
        makeHighPass(&postSculptStates[0][0], 60.0f);
        makePeakFilter(&postSculptStates[0][1], 140.0f, 2.4f, -3.5f);
        makePeakFilter(&postSculptStates[0][2], 2200.0f, 0.3f, 4.0f);
        makePeakFilter(&postSculptStates[0][3], 2600.0f, 5.8f, -5.1f);
        makePeakFilter(&postSculptStates[0][4], 6400.0f, 1.0f, 4.8f);
        makeLowPass(&postSculptStates[0][5], 10000.0f);
        
        // Update tone stacks
        updateCleanToneStack();
        updateDirtyToneStack();
        updateNamToneStack();
    }
    
    void updateCleanToneStack()
    {
        makePeakFilter(&cleanToneStackStates[0][0], 160.0f, 0.6f, cleanLow);
        makePeakFilter(&cleanToneStackStates[0][1], 600.0f, 0.8f, cleanMid);
        makePeakFilter(&cleanToneStackStates[0][2], 2600.0f, 0.8f, cleanHigh);
        makeHighShelf(&cleanToneStackStates[0][3], 6000.0f, 0.8f, cleanPresence);
    }
    
    void updateDirtyToneStack()
    {
        makePeakFilter(&dirtyToneStackStates[0][0], 160.0f, 0.6f, dirtyLow);
        makePeakFilter(&dirtyToneStackStates[0][1], 600.0f, 0.8f, dirtyMid);
        makePeakFilter(&dirtyToneStackStates[0][2], 2600.0f, 0.8f, dirtyHigh);
        makeHighShelf(&dirtyToneStackStates[0][3], 6000.0f, 0.8f, dirtyPresence);
    }
    
    void updateNamToneStack()
    {
        makePeakFilter(&namToneStackStates[0][0], 160.0f, 0.6f, namLow);
        makePeakFilter(&namToneStackStates[0][1], 600.0f, 0.8f, namMid);
        makePeakFilter(&namToneStackStates[0][2], 2600.0f, 0.8f, namHigh);
        makeHighShelf(&namToneStackStates[0][3], 6000.0f, 0.8f, namPresence);
    }
};
}