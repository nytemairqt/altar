// ==================================| Third Party Node Template |==================================

#pragma once
#include <JuceHeader.h>

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;

// ==========================| The node class with all required callbacks |==========================

template <int NV> struct chorus: public data::base
{
    SNEX_NODE(chorus);        
    
    struct MetadataClass
    {
        SN_NODE_ID("chorus");
    };
    
    static constexpr bool isModNode() { return false; };
    static constexpr bool isPolyphonic() { return NV > 1; };
    static constexpr bool hasTail() { return true; };
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

    chorus()
    {
        // Initialize vectors with proper sizes
        delayLines.resize(2);
        delayIndices.assign(2, 0);
        
        // Initialize LFO phases for multiple voices per channel
        for (int i = 0; i < 8; ++i)
        {
            lfoPhase[i] = (i % 2) * juce::MathConstants<float>::pi * 0.25f;
        }
    }
    
    void prepare(PrepareSpecs specs) 
    {
        sampleRate = static_cast<float>(specs.sampleRate);
        numChannels = static_cast<int>(specs.numChannels);
        numChannels = juce::jlimit(1, 2, numChannels); // node is fixed 2-ch

        // Maximum delay time for chorus (typically 20-50ms)
        const float maxDelayTime = 0.05f; // 50ms
        int maxDelayInSamples = static_cast<int>(maxDelayTime * sampleRate);

        // Resize delay lines
        delayLines.resize(numChannels);
        delayIndices.resize(numChannels);
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            delayLines[ch].assign(maxDelayInSamples, 0.0f);
            delayIndices[ch] = 0;
        }

        // Initialize filters
        updateInputFilter();
        updateOutputFilter();

        currentBPM = 120.0f;
        reset();
    }       

    void reset() 
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            if (ch < delayLines.size())
            {
                std::fill(delayLines[ch].begin(), delayLines[ch].end(), 0.0f);
                delayIndices[ch] = 0;
            }
        }

        // Reset LFO phases
        for (int i = 0; i < 8; ++i)
        {
            lfoPhase[i] = (i % 2) * juce::MathConstants<float>::pi * 0.25f;
        }

        // Reset filter states
        for (int ch = 0; ch < 2; ++ch)
        {
            inputFilterState[ch].reset();
            outputFilterState[ch].reset();
        }
    }
    
    template <typename T> void process(T& data)
    {
        const int numSamples = data.getNumSamples();
        const float mixValue = juce::jlimit(0.0f, 1.0f, mix);
        const float rateValue = juce::jlimit(0.1f, 10.0f, rate);
        const float depthValue = juce::jlimit(0.0f, 1.0f, depth);
        const float feedbackValue = juce::jlimit(0.0f, 0.8f, feedback);
        const float delayTimeValue = juce::jlimit(5.0f, 30.0f, delayTime);
        const float preDelayValue = juce::jlimit(0.0f, 10.0f, preDelay);
        const int voicesValue = juce::jlimit(1, 4, static_cast<int>(voices));
        const bool tempoSyncEnabled = tempoSync;

        // Update output filter
        updateOutputFilter();

        for (int s = 0; s < numSamples; ++s)
        {
            // Handle tempo sync
            float actualRate = rateValue;
            if (tempoSyncEnabled && currentBPM > 0.0f)
            {
                float noteValues[] = { 1.0f/8.0f, 1.0f/4.0f, 1.0f/2.0f, 1.0f, 2.0f, 4.0f, 8.0f };
                int noteIndex = juce::jlimit(0, 6, static_cast<int>(rateValue * 6.0f / 10.0f));
                actualRate = (currentBPM / 60.0f) * noteValues[noteIndex];
            }

            int channelIndex = 0;
            for (auto ch : data)
            {
                if (channelIndex >= numChannels || channelIndex >= delayLines.size()) break;
                
                dyn<float> channelData = data.toChannelData(ch);
                float input = channelData[s];
                float drySignal = input;

                // Apply input high-pass filter for analog character
                float filteredInput = inputFilterState[channelIndex].process(input);

                // Calculate base delay time with pre-delay
                float baseDelayMs = delayTimeValue + preDelayValue;
                float baseDelaySamples = (baseDelayMs / 1000.0f) * sampleRate;

                float wetOutput = 0.0f;

                // Generate multiple voices for richer chorus effect
                for (int voice = 0; voice < voicesValue; ++voice)
                {
                    // Calculate LFO phase for this voice (with stereo and voice offsets)
                    float phaseOffset = voice * juce::MathConstants<float>::pi * 0.5f;
                    if (channelIndex == 1) // Right channel offset for stereo width
                        phaseOffset += juce::MathConstants<float>::pi * 0.3f;

                    int lfoIndex = channelIndex * 4 + voice; // Store per channel/voice
                    if (lfoIndex < 8)
                    {
                        float& lfoPhaseRef = lfoPhase[lfoIndex];
                        lfoPhaseRef += (2.0f * juce::MathConstants<float>::pi * actualRate) / sampleRate;
                        if (lfoPhaseRef >= 2.0f * juce::MathConstants<float>::pi)
                            lfoPhaseRef -= 2.0f * juce::MathConstants<float>::pi;

                        // Generate LFO waveform (triangle wave for more analog sound)
                        float lfoValue;
                        float normalizedPhase = (lfoPhaseRef + phaseOffset);
                        while (normalizedPhase >= 2.0f * juce::MathConstants<float>::pi)
                            normalizedPhase -= 2.0f * juce::MathConstants<float>::pi;
                        while (normalizedPhase < 0.0f)
                            normalizedPhase += 2.0f * juce::MathConstants<float>::pi;

                        normalizedPhase /= (2.0f * juce::MathConstants<float>::pi);

                        // Triangle wave
                        if (normalizedPhase < 0.5f)
                            lfoValue = 4.0f * normalizedPhase - 1.0f;
                        else
                            lfoValue = 3.0f - 4.0f * normalizedPhase;

                        // Add some subtle sine wave harmonic for smoothness
                        lfoValue = lfoValue * 0.8f + std::sin(normalizedPhase * 2.0f * juce::MathConstants<float>::pi) * 0.2f;

                        // Calculate modulated delay time
                        float modulationRange = 5.0f; // ±5ms modulation range
                        float modulatedDelayMs = baseDelayMs + (lfoValue * depthValue * modulationRange);
                        float modulatedDelaySamples = juce::jlimit(1.0f, static_cast<float>(delayLines[channelIndex].size() - 1),
                                                                  (modulatedDelayMs / 1000.0f) * sampleRate);

                        // Read from delay line with linear interpolation
                        float delayIndex = static_cast<float>(delayIndices[channelIndex]) - modulatedDelaySamples;
                        if (delayIndex < 0.0f)
                            delayIndex += static_cast<float>(delayLines[channelIndex].size());

                        int index1 = static_cast<int>(delayIndex);
                        int index2 = (index1 + 1) % delayLines[channelIndex].size();
                        float fraction = delayIndex - static_cast<float>(index1);

                        float delayedSample = delayLines[channelIndex][index1] * (1.0f - fraction) +
                                             delayLines[channelIndex][index2] * fraction;

                        // Add slight detuning per voice for richer sound
                        float detuning = 1.0f + (voice - voicesValue * 0.5f) * 0.001f;
                        delayedSample *= detuning;

                        wetOutput += delayedSample / voicesValue; // Average multiple voices
                    }
                }

                // Apply feedback with saturation for analog warmth
                float feedbackSignal = filteredInput + (wetOutput * feedbackValue);
                
                // Soft clipping for analog saturation
                feedbackSignal = std::tanh(feedbackSignal * 0.8f) * 1.2f;

                // Write to delay line
                delayLines[channelIndex][delayIndices[channelIndex]] = feedbackSignal;
                delayIndices[channelIndex] = (delayIndices[channelIndex] + 1) % delayLines[channelIndex].size();

                // Apply output low-pass filter
                float filteredOutput = outputFilterState[channelIndex].process(wetOutput);

                // Add subtle analog-style noise and warmth
                float noise = (juce::Random::getSystemRandom().nextFloat() - 0.5f) * 0.0002f;
                filteredOutput += noise;

                // Final mix with phase inversion option for more chorus variations
                float finalWet = filteredOutput;
                if (channelIndex == 1 && voicesValue > 2) // Subtle phase variation on right channel
                    finalWet *= -0.95f; // Slight phase inversion with level reduction

                channelData[s] = drySignal * (1.0f - mixValue) + finalWet * mixValue * 0.7f; // Scale wet signal
                
                channelIndex++;
            }
        }
    }
    
    template <int P> void setParameter(double v)
    {
        switch (P)
        {
        case 0: mix = static_cast<float>(v); break;
        case 1: rate = static_cast<float>(v); break;
        case 2: depth = static_cast<float>(v); break;
        case 3: feedback = static_cast<float>(v); break;
        case 4: delayTime = static_cast<float>(v); break;
        case 5: preDelay = static_cast<float>(v); break;
        case 6: tone = static_cast<float>(v); break;
        case 7: voices = static_cast<float>(v); break;
        case 8: tempoSync = v > 0.5f; break;
        }
    }
    
    void createParameters(ParameterDataList& data)
    {       
        {
            parameter::data mix_param("Mix", { 0.0, 1.0 });
            registerCallback<0>(mix_param);
            mix_param.setDefaultValue(0.5);
            data.add(std::move(mix_param));
        }
        {
            parameter::data rate_param("Rate", { 0.1, 10.0 });
            registerCallback<1>(rate_param);
            rate_param.setDefaultValue(0.8);
            data.add(std::move(rate_param));
        }
        {
            parameter::data depth_param("Depth", { 0.0, 1.0 });
            registerCallback<2>(depth_param);
            depth_param.setDefaultValue(0.7);
            data.add(std::move(depth_param));
        }
        {
            parameter::data feedback_param("Feedback", { 0.0, 0.8 });
            registerCallback<3>(feedback_param);
            feedback_param.setDefaultValue(0.2);
            data.add(std::move(feedback_param));
        }
        {
            parameter::data delaytime_param("DelayTime", { 5.0, 30.0 });
            registerCallback<4>(delaytime_param);
            delaytime_param.setDefaultValue(15.0);
            data.add(std::move(delaytime_param));
        }
        {
            parameter::data predelay_param("PreDelay", { 0.0, 10.0 });
            registerCallback<5>(predelay_param);
            predelay_param.setDefaultValue(2.0);
            data.add(std::move(predelay_param));
        }
        {
            parameter::data tone_param("Tone", { 200.0, 10000.0 });
            registerCallback<6>(tone_param);
            tone_param.setDefaultValue(5000.0);
            data.add(std::move(tone_param));
        }
        {
            parameter::data voices_param("Voices", { 1.0, 4.0 });
            registerCallback<7>(voices_param);
            voices_param.setDefaultValue(2.0);
            data.add(std::move(voices_param));
        }
        {
            parameter::data sync_param("TempoSync", { 0.0, 1.0 });
            registerCallback<8>(sync_param);
            sync_param.setDefaultValue(0.0);
            data.add(std::move(sync_param));
        }
    }

private:
    float sampleRate = 44100.0f;
    int numChannels = 2;
    float currentBPM = 120.0f;

    // Parameters
    float mix = 0.5f;
    float rate = 0.8f;
    float depth = 0.7f;
    float feedback = 0.2f;
    float delayTime = 15.0f;
    float preDelay = 2.0f;
    float tone = 5000.0f;
    float voices = 2.0f;
    bool tempoSync = false;

    // Delay lines for chorus effect
    std::vector<std::vector<float>> delayLines;
    std::vector<int> delayIndices;

    // LFO phases (per channel and voice)
    float lfoPhase[8] = { 0.0f }; // Support up to 2 channels * 4 voices

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
        // High-pass filter at 40 Hz
        float freq = 40.0f;
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
        // Low-pass filter with variable frequency based on tone parameter
        float freq = juce::jlimit(200.0f, 10000.0f, tone);
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