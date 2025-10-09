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
        
        float fastRmsTimeMs = 2.5f;
        fastRmsCoefficient = 1.0f - exp(-1.0f / (fastRmsTimeMs * 0.001f * sampleRate));

        float slowRmsTimeMs = 8.0f;
        slowRmsCoefficient = 1.0f - exp(-1.0f / (slowRmsTimeMs * 0.001f * sampleRate));

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
        int numSamples = data.getNumSamples();
        float attackCoefficient = exp(-1.0f / (attack * 0.001f * sampleRate));
        float releaseCoefficient = exp(-1.0f / (release * 0.001f * sampleRate));

        for (auto ch : data)
        {
            dyn<float> channelData = data.toChannelData(ch);

            for (int s = 0; s < numSamples; ++s)
            {
                float inputSquared = channelData[s] * channelData[s];
                float currentPeak = abs(channelData[s]);

                peakLevel = currentPeak * 0.1f + peakLevel * 0.9f;

                fastRmsLevel = fastRmsLevel * (1.0f - fastRmsCoefficient) +
                    inputSquared * fastRmsCoefficient;
                float fastRms = sqrt(fastRmsLevel);

                slowRmsLevel = slowRmsLevel * (1.0f - slowRmsCoefficient) +
                    inputSquared * slowRmsCoefficient;
                float slowRms = sqrt(slowRmsLevel);

                float detectionLevel = calculateDetectionLevel(peakLevel, fastRms, slowRms);

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
                        targetGain = 0.0f;
                }

                if (targetGain > envelope)
                    envelope += (targetGain - envelope) * (1.0f - attackCoefficient);
                else
                    envelope += (targetGain - envelope) * (1.0f - releaseCoefficient);               

                channelData[s] *= envelope;
            }
        }
    }
	
	template <int P> void setParameter(double v)
	{
		if (P == 0)
		{            
            thresholdParameter = static_cast<float>(v);
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
    float release = 30.0f;
    float thresholdParameter = -24.0f;
    float sensitivity = 0.7f;

    float fastRmsLevel = 0.0f;
    float slowRmsLevel = 0.0f;
    float peakLevel = 0.0f;
    float fastRmsCoefficient = 0.0f;
    float slowRmsCoefficient = 0.0f;

    float openThreshold = 0.0f;
    float closeThreshold = 0.0f;

    float calculateDetectionLevel(float peak, float fastRms, float slowRms)
    {
        float peakWeight = 0.3f + (sensitivity * 0.4f);
        float fastRmsWeight = 0.5f - (sensitivity * 0.2f);
        float slowRmsWeight = 0.2f - (sensitivity * 0.1f);

        return (peak * peakWeight) + (fastRms * fastRmsWeight) + (slowRms * slowRmsWeight);
    }

    void updateThresholds()
    {
        float baseThreshold = juce::Decibels::decibelsToGain(thresholdParameter);
        openThreshold = baseThreshold * 0.85f;
        closeThreshold = baseThreshold * 1.15f;
    }
};
}