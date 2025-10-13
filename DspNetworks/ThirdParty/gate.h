// ==================================| Third Party Node Template |==================================

#pragma once
#include <JuceHeader.h>

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;

// ==========================| The node class with all required callbacks |==========================

template <int NV> struct gate: public data::base
{
    SNEX_NODE(gate);        
    
    struct MetadataClass
    {
        SN_NODE_ID("gate");
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
        
        // Slightly longer detector windows for smoother gating
        float fastRmsTimeMs = 10.0f;
        fastRmsCoefficient = 1.0f - std::exp(-1.0f / (fastRmsTimeMs * 0.001f * sampleRate));

        float slowRmsTimeMs = 120.0f;
        slowRmsCoefficient = 1.0f - std::exp(-1.0f / (slowRmsTimeMs * 0.001f * sampleRate));

        updateThresholds();
    }       

    void reset() 
    {
        envelope = 0.0f;
        gateState = false;
        fastRmsLevel = 0.0f;
        slowRmsLevel = 0.0f;
        peakLevel = 0.0f;
    }
    
    template <typename T> void process(T& data)
    {
        const int numSamples = data.getNumSamples();

        // Obtain dyn<float> handles for the first two channels via ChannelPtr iteration
        dyn<float> ch0;
        dyn<float> ch1;

        int ci = 0;
        for (auto ch : data)
        {
            if (ci == 0) ch0 = data.toChannelData(ch);
            else if (ci == 1) ch1 = data.toChannelData(ch);
            ++ci;
        }

        // If only one channel is present for some reason, mirror it to avoid null access
        if (ci < 2) ch1 = ch0;

        const float attackCoefficient  = std::exp(-1.0f / (attack  * 0.001f * sampleRate));
        const float releaseCoefficient = std::exp(-1.0f / (release * 0.001f * sampleRate));

        for (int s = 0; s < numSamples; ++s)
        {
            const float in0 = ch0[s];
            const float in1 = ch1[s];

            // Stereo-aware detector
            const float absMax = std::max(std::abs(in0), std::abs(in1));
            peakLevel = absMax * 0.1f + peakLevel * 0.9f;

            const float e0 = in0 * in0;
            const float e1 = in1 * in1;
            const float monoEnergy = 0.5f * (e0 + e1); // average of L/R energy

            fastRmsLevel = fastRmsLevel * (1.0f - fastRmsCoefficient) + monoEnergy * fastRmsCoefficient;
            const float fastRms = std::sqrt(fastRmsLevel);

            slowRmsLevel = slowRmsLevel * (1.0f - slowRmsCoefficient) + monoEnergy * slowRmsCoefficient;
            const float slowRms = std::sqrt(slowRmsLevel);

            const float detectionLevel = calculateDetectionLevel(peakLevel, fastRms, slowRms);

            float targetGain = 0.0f;
            if (gateState)
            {
                if (detectionLevel > closeThreshold)
                    targetGain = 1.0f;
                else
                {
                    gateState = false;
                    targetGain = 0.0f;
                }
            }
            else
            {
                if (detectionLevel > openThreshold)
                {
                    gateState = true;
                    targetGain = 1.0f;
                }
                else
                {
                    targetGain = 0.0f;
                }
            }

            if (targetGain > envelope)
                envelope += (targetGain - envelope) * (1.0f - attackCoefficient);
            else
                envelope += (targetGain - envelope) * (1.0f - releaseCoefficient);

            ch0[s] = in0 * envelope;
            ch1[s] = in1 * envelope;
        }
    }
    
    template <int P> void setParameter(double v)
    {
        if (P == 0)
        {            
            thresholdParameter = static_cast<float>(v);
            updateThresholds();
        }
    }
    
    void createParameters(ParameterDataList& data)
    {       
        {
            parameter::data threshold("Threshold", { -100.0, 0.0 });
            registerCallback<0>(threshold);
            threshold.setDefaultValue(-24.0);            
            data.add(std::move(threshold));
        }        
    }

private:
    float sampleRate = 44100.0f;   
    float envelope = 0.0f;
    bool gateState = false;

    float attack = 2.0f;
    float release = 120.0f;

    float thresholdParameter = -24.0f;
    float sensitivity = 0.7f;

    float fastRmsLevel = 0.0f;
    float slowRmsLevel = 0.0f;
    float peakLevel = 0.0f;
    float fastRmsCoefficient = 0.0f;
    float slowRmsCoefficient = 0.0f;

    float openThreshold = 0.0f;
    float closeThreshold = 0.0f;

    // Hysteresis span in dB: open at threshold, close lower than threshold
    float hysteresisDb = 6.0f;

    float calculateDetectionLevel(float peak, float fastRms, float slowRms)
    {
        float peakWeight = 0.3f + (sensitivity * 0.4f);
        float fastRmsWeight = 0.5f - (sensitivity * 0.2f);
        float slowRmsWeight = 0.2f - (sensitivity * 0.1f);

        // Normalize weights so their sum is ~1.0
        float wSum = peakWeight + fastRmsWeight + slowRmsWeight;
        if (wSum <= 0.0f) wSum = 1.0f;
        const float inv = 1.0f / wSum;

        return (peak * peakWeight + fastRms * fastRmsWeight + slowRms * slowRmsWeight) * inv;
    }

    void updateThresholds()
    {
        const float openDb  = thresholdParameter;
        const float closeDb = thresholdParameter - hysteresisDb;

        openThreshold  = juce::Decibels::decibelsToGain(openDb);
        closeThreshold = juce::Decibels::decibelsToGain(closeDb);
    }
};
}