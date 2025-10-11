// ==================================| Third Party Node Template |==================================

#pragma once
#include <JuceHeader.h>

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;

// ==========================| The node class with all required callbacks |==========================

template <int NV> struct ringmod: public data::base
{
    SNEX_NODE(ringmod);        
    
    struct MetadataClass
    {
        SN_NODE_ID("ringmod");
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

    ringmod()
    {
        // Initialize oscillator phases and envelope states
        for (int ch = 0; ch < 2; ++ch)
        {
            oscillatorPhase[ch] = 0.0f;
            lfoPhase[ch] = 0.0f;
            envelopeState[ch] = 0.0f;
        }
    }
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate = static_cast<float>(specs.sampleRate);
        numChannels = static_cast<int>(specs.numChannels);
        
        // Initialize filter coefficients
        updateInputFilter();
        updateOutputFilter();
        
        reset();
    }       

    void reset() 
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            oscillatorPhase[ch] = 0.0f;
            lfoPhase[ch] = 0.0f;
            envelopeState[ch] = 0.0f;
            
            // Reset filter states
            inputFilterState[ch].reset();
            outputFilterState[ch].reset();
        }
    }
    
    template <typename T> void process(T& data)
    {
        const int numSamples = data.getNumSamples();
        const float mixValue = juce::jlimit(0.0f, 1.0f, mix);
        const float freqValue = juce::jlimit(1.0f, 2000.0f, frequency);
        const float depthValue = juce::jlimit(0.0f, 1.0f, depth);
        const float lfoRateValue = juce::jlimit(0.1f, 10.0f, lfoRate);
        const float lfoDepthValue = juce::jlimit(0.0f, 1.0f, lfoDepth);
        const int modeValue = static_cast<int>(mode) % 4;
        const bool stereoModeEnabled = stereoMode;
        const bool tempoSyncEnabled = tempoSync;
        
        // Update output filter frequency
        updateOutputFilter();

        for (int s = 0; s < numSamples; ++s)
        {
            int channelIndex = 0;
            for (auto ch : data)
            {
                if (channelIndex >= numChannels || channelIndex >= 2) break;
                
                dyn<float> channelData = data.toChannelData(ch);
                float input = channelData[s];
                float drySignal = input;

                // Apply input high-pass filter
                float filteredInput = inputFilterState[channelIndex].process(input);
                
                float baseFreq = freqValue;
                if (tempoSyncEnabled && currentBPM > 0.0f)
                {
                    float noteValues[] = { 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f };
                    int noteIndex = juce::jlimit(0, 6, static_cast<int>(freqValue * 6.0f / 2000.0f));
                    baseFreq = (currentBPM / 60.0f) * noteValues[noteIndex];
                }

                // LFO modulation
                float& lfoPhaseRef = lfoPhase[channelIndex];
                lfoPhaseRef += (2.0f * juce::MathConstants<float>::pi * lfoRateValue) / sampleRate;
                if (lfoPhaseRef >= 2.0f * juce::MathConstants<float>::pi)
                    lfoPhaseRef -= 2.0f * juce::MathConstants<float>::pi;

                float lfoValue = std::sin(lfoPhaseRef) * lfoDepthValue;
                float modulatedFreq = baseFreq * (1.0f + lfoValue * 0.5f);
                
                // Stereo offset
                if (stereoModeEnabled && channelIndex == 1)
                    modulatedFreq *= 1.05f;

                // Generate modulator signal
                float modulatorSignal = 0.0f;
                float& oscPhaseRef = oscillatorPhase[channelIndex];

                switch (modeValue)
                {
                case 0: // sine
                    modulatorSignal = std::sin(oscPhaseRef);
                    oscPhaseRef += (2.0f * juce::MathConstants<float>::pi * modulatedFreq) / sampleRate;
                    break;

                case 1: // square
                    modulatorSignal = (std::sin(oscPhaseRef) >= 0.0f) ? 1.0f : -1.0f;
                    oscPhaseRef += (2.0f * juce::MathConstants<float>::pi * modulatedFreq) / sampleRate;
                    break;

                case 2: // saw
                    modulatorSignal = (2.0f * (oscPhaseRef / (2.0f * juce::MathConstants<float>::pi))) - 1.0f;
                    oscPhaseRef += (2.0f * juce::MathConstants<float>::pi * modulatedFreq) / sampleRate;
                    break;

                case 3: // envelope follower
                {
                    float inputLevel = std::abs(filteredInput);
                    float& envelopeRef = envelopeState[channelIndex];
                    float attack = 0.001f;  // 1ms
                    float release = 0.1f;   // 100ms

                    if (inputLevel > envelopeRef)
                        envelopeRef += (inputLevel - envelopeRef) * (1.0f - std::exp(-1.0f / (attack * sampleRate)));
                    else
                        envelopeRef += (inputLevel - envelopeRef) * (1.0f - std::exp(-1.0f / (release * sampleRate)));

                    float envModulatedFreq = modulatedFreq * (0.5f + envelopeRef * 2.0f);
                    modulatorSignal = std::sin(oscPhaseRef);
                    oscPhaseRef += (2.0f * juce::MathConstants<float>::pi * envModulatedFreq) / sampleRate;
                    break;
                }
                }

                // Wrap oscillator phase
                if (oscPhaseRef >= 2.0f * juce::MathConstants<float>::pi)
                    oscPhaseRef -= 2.0f * juce::MathConstants<float>::pi;
                
                // Ring modulation
                float ringModOutput = filteredInput * (1.0f + modulatorSignal * depthValue);

                // Apply output low-pass filter
                float filteredOutput = outputFilterState[channelIndex].process(ringModOutput);
                
                // Soft saturation
                filteredOutput = std::tanh(filteredOutput * 0.7f) * 1.2f;
                
                // Mix wet and dry signals
                channelData[s] = drySignal * (1.0f - mixValue) + filteredOutput * mixValue;
                
                channelIndex++;
            }
        }
    }
    
    template <int P> void setParameter(double v)
    {
        switch (P)
        {
        case 0: mix = static_cast<float>(v); break;
        case 1: frequency = static_cast<float>(v); break;
        case 2: depth = static_cast<float>(v); break;
        case 3: mode = static_cast<float>(v); break;
        case 4: lfoRate = static_cast<float>(v); break;
        case 5: lfoDepth = static_cast<float>(v); break;
        case 6: filterFrequency = static_cast<float>(v); break;
        case 7: stereoMode = v > 0.5f; break;
        case 8: tempoSync = v > 0.5f; break;
        }
    }
    
    void createParameters(ParameterDataList& data)
    {       
        {
            parameter::data mix_param("Mix", { 0.0, 1.0 });
            registerCallback<0>(mix_param);
            mix_param.setDefaultValue(0.3);
            data.add(std::move(mix_param));
        }
        {
            parameter::data freq_param("Frequency", { 1.0, 2000.0 });
            registerCallback<1>(freq_param);
            freq_param.setDefaultValue(100.0);
            data.add(std::move(freq_param));
        }
        {
            parameter::data depth_param("Depth", { 0.0, 1.0 });
            registerCallback<2>(depth_param);
            depth_param.setDefaultValue(0.7);
            data.add(std::move(depth_param));
        }
        {
            parameter::data mode_param("Mode", { 0.0, 3.0 });
            registerCallback<3>(mode_param);
            mode_param.setDefaultValue(0.0);
            data.add(std::move(mode_param));
        }
        {
            parameter::data lfo_rate_param("LFO Rate", { 0.1, 10.0 });
            registerCallback<4>(lfo_rate_param);
            lfo_rate_param.setDefaultValue(2.0);
            data.add(std::move(lfo_rate_param));
        }
        {
            parameter::data lfo_depth_param("LFO Depth", { 0.0, 1.0 });
            registerCallback<5>(lfo_depth_param);
            lfo_depth_param.setDefaultValue(0.2);
            data.add(std::move(lfo_depth_param));
        }
        {
            parameter::data filter_param("Filter Frequency", { 200.0, 12000.0 });
            registerCallback<6>(filter_param);
            filter_param.setDefaultValue(8000.0);
            data.add(std::move(filter_param));
        }
        {
            parameter::data stereo_param("Stereo Mode", { 0.0, 1.0 });
            registerCallback<7>(stereo_param);
            stereo_param.setDefaultValue(1.0);
            data.add(std::move(stereo_param));
        }
        {
            parameter::data tempo_param("Tempo Sync", { 0.0, 1.0 });
            registerCallback<8>(tempo_param);
            tempo_param.setDefaultValue(0.0);
            data.add(std::move(tempo_param));
        }
    }

private:
    float sampleRate = 44100.0f;
    int numChannels = 2;
    float currentBPM = 120.0f;

    // Parameters
    float mix = 0.3f;
    float frequency = 100.0f;
    float depth = 0.7f;
    float mode = 0.0f;
    float lfoRate = 2.0f;
    float lfoDepth = 0.2f;
    float filterFrequency = 8000.0f;
    bool stereoMode = true;
    bool tempoSync = false;
    
    // Oscillator and LFO phases
    float oscillatorPhase[2] = { 0.0f, 0.0f };
    float lfoPhase[2] = { 0.0f, 0.0f };
    float envelopeState[2] = { 0.0f, 0.0f };

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

    BiquadState inputFilterState[2];
    BiquadState outputFilterState[2];

    void updateInputFilter()
    {
        // High-pass filter at 80 Hz
        float freq = 80.0f;
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float alpha = sinOmega / (2.0f * 0.707f); // Q = 0.707
        
        float b0 = (1.0f + cosOmega) / 2.0f;
        float b1 = -(1.0f + cosOmega);
        float b2 = (1.0f + cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;
        
        for (int ch = 0; ch < 2; ++ch)
        {
            inputFilterState[ch].b0 = b0 / a0;
            inputFilterState[ch].b1 = b1 / a0;
            inputFilterState[ch].b2 = b2 / a0;
            inputFilterState[ch].a1 = a1 / a0;
            inputFilterState[ch].a2 = a2 / a0;
        }
    }

    void updateOutputFilter()
    {
        // Low-pass filter with variable frequency
        float freq = juce::jlimit(200.0f, 12000.0f, filterFrequency);
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float alpha = sinOmega / (2.0f * 0.707f); // Q = 0.707
        
        float b0 = (1.0f - cosOmega) / 2.0f;
        float b1 = 1.0f - cosOmega;
        float b2 = (1.0f - cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;
        
        for (int ch = 0; ch < 2; ++ch)
        {
            outputFilterState[ch].b0 = b0 / a0;
            outputFilterState[ch].b1 = b1 / a0;
            outputFilterState[ch].b2 = b2 / a0;
            outputFilterState[ch].a1 = a1 / a0;
            outputFilterState[ch].a2 = a2 / a0;
        }
    }
};
}