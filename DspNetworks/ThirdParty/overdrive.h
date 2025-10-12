// ==================================| Third Party Node Template |==================================

#pragma once
#include <JuceHeader.h>

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;

// ==========================| The node class with all required callbacks |==========================

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
        
        // Initialize smoothed values
        driveSmoothed.reset(scaledSampleRate, 0.01f);
        driveSmoothed.setCurrentAndTargetValue(drive);
        
        outputSmoothed.reset(scaledSampleRate, 0.01f);
        outputSmoothed.setCurrentAndTargetValue(outputGain);
        
        mixSmoothed.reset(scaledSampleRate, 0.01f);
        mixSmoothed.setCurrentAndTargetValue(mix);
        
        // Reset all filters and states
        reset();        
    }           

    void reset() 
    {
        // Reset smoothed values
        driveSmoothed.reset(scaledSampleRate, 0.01f);
        outputSmoothed.reset(scaledSampleRate, 0.01f);
        mixSmoothed.reset(scaledSampleRate, 0.01f);
        
        // Reset filter states
        for (int ch = 0; ch < 2; ++ch)
        {
            inputHPFStates[ch].reset();
            outputLPFStates[ch].reset();
            toneFilterStates[ch].reset();
        }
        
        // Reset distortion states
        for (int ch = 0; ch < 2; ++ch)
        {
            lastBitcrushedSample[ch] = 0.0f;
            bitcrusherCounter[ch] = 0;
            wavefolderState[ch] = 0.0f;
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
        case 0: distortionMode = static_cast<int>(v); break;
        case 1: 
        {
            int oldFactor = oversamplingFactor;
            oversamplingFactor = static_cast<int>(v);
            if (oldFactor != oversamplingFactor)
                updateOversampling();
            break;
        }
        case 2: drive = static_cast<float>(v); break;
        case 3: tone = static_cast<float>(v); updateFilterCoefficients(); break;
        case 4: toneFreq = static_cast<float>(v); updateFilterCoefficients(); break;
        case 5: outputGain = static_cast<float>(v); break;
        case 6: mix = static_cast<float>(v); break;
        case 7: bits = static_cast<float>(v); break;
        case 8: sampleRateReduction = static_cast<float>(v); break;
        case 9: foldAmount = static_cast<float>(v); break;
        }
    }
    
    void createParameters(ParameterDataList& data)
    {       
        // Distortion Mode
        {
            parameter::data mode("Mode", { 0.0, 5.0 });
            registerCallback<0>(mode);
            mode.setDefaultValue(0.0);
            data.add(std::move(mode));
        }
        
        // Oversampling Factor
        {
            parameter::data oversamp("Oversampling", { 0.0, 3.0 });
            registerCallback<1>(oversamp);
            oversamp.setDefaultValue(1.0);
            data.add(std::move(oversamp));
        }

        // Drive
        {
            parameter::data driveParam("Drive", { 0.1, 20.0 });
            registerCallback<2>(driveParam);
            driveParam.setDefaultValue(5.0);
            data.add(std::move(driveParam));
        }
        
        // Tone
        {
            parameter::data toneParam("Tone", { -12.0, 12.0 });
            registerCallback<3>(toneParam);
            toneParam.setDefaultValue(0.0);
            data.add(std::move(toneParam));
        }
        
        // Tone Frequency
        {
            parameter::data toneFreqParam("ToneFreq", { 200.0, 5000.0 });
            registerCallback<4>(toneFreqParam);
            toneFreqParam.setDefaultValue(1000.0);
            data.add(std::move(toneFreqParam));
        }
        
        // Output Gain
        {
            parameter::data outputParam("OutputGain", { -24.0, 24.0 });
            registerCallback<5>(outputParam);
            outputParam.setDefaultValue(0.0);
            data.add(std::move(outputParam));
        }
        
        // Mix
        {
            parameter::data mixParam("Mix", { 0.0, 1.0 });
            registerCallback<6>(mixParam);
            mixParam.setDefaultValue(1.0);
            data.add(std::move(mixParam));
        }
        
        // Bits (for bitcrusher)
        {
            parameter::data bitsParam("Bits", { 1.0, 16.0 });
            registerCallback<7>(bitsParam);
            bitsParam.setDefaultValue(16.0);
            data.add(std::move(bitsParam));
        }
        
        // Sample Rate Reduction
        {
            parameter::data srReductionParam("SRReduction", { 1.0, 32.0 });
            registerCallback<8>(srReductionParam);
            srReductionParam.setDefaultValue(1.0);
            data.add(std::move(srReductionParam));
        }
        
        // Fold Amount (for wavefolder)
        {
            parameter::data foldParam("FoldAmount", { 1.0, 8.0 });
            registerCallback<9>(foldParam);
            foldParam.setDefaultValue(2.0);
            data.add(std::move(foldParam));
        }
    }

private:
    // Audio processing
    float sampleRate = 44100.0f;
    int numChannels = 2;
    int maxBlockSize = 512;
    
    // Parameters
    int distortionMode = 0; // 0=fuzz, 1=screamer, 2=rat, 3=bitcrusher, 4=glitch, 5=wavefolder
    int oversamplingFactor = 1;
    float drive = 5.0f;
    float tone = 0.0f;
    float toneFreq = 1000.0f;
    float outputGain = 0.0f;
    float mix = 1.0f;
    float bits = 16.0f;
    float sampleRateReduction = 1.0f;
    float foldAmount = 2.0f;
    
    // Oversampling
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
    
    // Smoothed values
    SmoothedValue<float> driveSmoothed;
    SmoothedValue<float> outputSmoothed;
    SmoothedValue<float> mixSmoothed;
    
    // Biquad Filter State (same as in amp.h)
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
    
    // Filter states [channel]
    BiquadState inputHPFStates[2];
    BiquadState outputLPFStates[2];
    BiquadState toneFilterStates[2];
    
    // Distortion states
    float lastBitcrushedSample[2] = { 0.0f, 0.0f };
    int bitcrusherCounter[2] = { 0, 0 };
    float wavefolderState[2] = { 0.0f, 0.0f };
    
    // Main processing function

    void processAudioBuffer(juce::AudioBuffer<float>& buffer)
    {
        const bool useOversampling = (oversamplingFactor > 0 && oversampling != nullptr);
        juce::dsp::AudioBlock<float> inBlock(buffer);

        // Helper to run the existing DSP on any AudioBuffer
        auto runDSP = [&](juce::AudioBuffer<float>& procBuf)
        {
            const int numCh = juce::jmin(2, procBuf.getNumChannels());
            const int procSamples = procBuf.getNumSamples();

            // Set smoothed value targets
            driveSmoothed.setTargetValue(juce::jlimit(0.1f, 20.0f, drive));
            outputSmoothed.setTargetValue(juce::jlimit(-24.0f, 24.0f, outputGain));
            mixSmoothed.setTargetValue(juce::jlimit(0.0f, 1.0f, mix));

            // Input high-pass filter
            for (int ch = 0; ch < numCh; ++ch)
            {
                float* channelData = procBuf.getWritePointer(ch);
                for (int s = 0; s < procSamples; ++s)
                    channelData[s] = inputHPFStates[ch].process(channelData[s]);
            }

            // Main distortion processing
            for (int s = 0; s < procSamples; ++s)
            {
                const float driveValue  = driveSmoothed.getNextValue();
                const float outputValue = outputSmoothed.getNextValue();
                const float mixValue    = mixSmoothed.getNextValue();

                for (int ch = 0; ch < numCh; ++ch)
                {
                    const float inputSample = procBuf.getSample(ch, s);
                    const float drySignal   = inputSample;

                    float distortedSample = processDistortion(inputSample, driveValue, ch);
                    distortedSample *= juce::Decibels::decibelsToGain(outputValue);

                    const float outputSample = drySignal * (1.0f - mixValue) + distortedSample * mixValue;
                    procBuf.setSample(ch, s, outputSample);
                }
            }

            // Tone and output filters
            for (int ch = 0; ch < numCh; ++ch)
            {
                float* channelData = procBuf.getWritePointer(ch);
                for (int s = 0; s < procSamples; ++s)
                {
                    float sample = channelData[s];
                    sample = toneFilterStates[ch].process(sample);
                    sample = outputLPFStates[ch].process(sample);
                    channelData[s] = sample;
                }
            }
        };

        if (useOversampling)
        {
            // Upsample into the oversampler's internal buffer
            auto upBlock = oversampling->processSamplesUp(inBlock);

            // Create a small array of channel pointers for AudioBuffer view
            const int upCh     = static_cast<int>(upBlock.getNumChannels());
            const int upFrames = static_cast<int>(upBlock.getNumSamples());

            juce::HeapBlock<float*> chanPtrs(upCh);
            for (int ch = 0; ch < upCh; ++ch)
                chanPtrs[ch] = upBlock.getChannelPointer(static_cast<size_t>(ch));

            // Non-owning AudioBuffer view over the upsampled data
            juce::AudioBuffer<float> osBuffer(chanPtrs.getData(), upCh, upFrames);

            // Run your DSP on the upsampled buffer
            runDSP(osBuffer);

            // Downsample back into the original buffer
            oversampling->processSamplesDown(inBlock);
        }
        else
        {
            // Process at native rate
            runDSP(buffer);
        }
    }
    
    float processDistortion(float input, float driveValue, int channel)
    {
        switch (distortionMode)
        {
        case 0: return processFuzz(input, driveValue);
        case 1: return processScreamer(input, driveValue);
        case 2: return processRat(input, driveValue);
        case 3: return processBitcrusher(input, channel);
        case 4: return processGlitch(input, channel);
        case 5: return processWavefolder(input, channel);
        default: return input;
        }
    }
    
    float processFuzz(float input, float driveValue)
    {
        float driven = input * driveValue * 0.8f;
        if (driven > 0.0f)
        {
            driven = std::tanh(driven * 1.5f);
        }
        else
        {
            driven = -std::pow(std::abs(driven), 0.7f) * std::copysign(1.0f, driven);
            driven = juce::jlimit(-1.0f, 1.0f, driven * 1.2f);
        }

        float harmonic = std::sin(driven * juce::MathConstants<float>::pi) * 0.1f;
        return juce::jlimit(-1.0f, 1.0f, driven + harmonic);
    }

    float processScreamer(float input, float driveValue)
    {
        float preGain = driveValue * 0.5f;
        float driven = input * preGain;
        float output = (2.0f / juce::MathConstants<float>::pi) * std::atan(driven * 2.0f);
        float harmonic = std::sin(output * juce::MathConstants<float>::pi * 2.0f) * 0.05f;
        output *= (1.0f + std::abs(output) * 0.1f);

        return juce::jlimit(-1.0f, 1.0f, output + harmonic);
    }

    float processRat(float input, float driveValue)
    {
        float driven = input * driveValue * 0.6f;
        if (std::abs(driven) > 0.7f)
        {
            float sign = driven > 0.0f ? 1.0f : -1.0f;
            float excess = std::abs(driven) - 0.7f;
            driven = sign * (0.7f + std::tanh(excess * 3.0f) * 0.3f);
        }

        float harmonic = std::sin(driven * juce::MathConstants<float>::pi * 3.0f) * 0.08f;
        return juce::jlimit(-1.0f, 1.0f, driven + harmonic);
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
    
    // Filter coefficient calculations (same pattern as amp.h)
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
        // Input high-pass filter
        makeHighPass(&inputHPFStates[0], 20.0f);
        
        // Output low-pass filter
        makeLowPass(&outputLPFStates[0], 12000.0f);
        
        // Tone filter (peak filter)
        float gain = Decibels::decibelsToGain(tone);
        makePeakFilter(&toneFilterStates[0], toneFreq, 0.7f, tone);
    }
};
}