// ==================================| Third Party Node Template |==================================

#pragma once
#include <JuceHeader.h>

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;
using cable_manager_t = routing::global_cable_cpp_manager<SN_GLOBAL_CABLE(106677056)>; 

enum class GlobalCables
{
	pitch = 0
};

// ==========================| The node class with all required callbacks |==========================

template <int NV> struct tuner: public data::base, public cable_manager_t, public hise::DllTimer
{
	// Metadata Definitions ------------------------------------------------------------------------
	
	SNEX_NODE(tuner);    

    tuner()
    {
        startTimer(100); // Faster update for better responsiveness
        pitchHistory.resize(pitchHistorySize, 0.0f);
    }

    ~tuner()
    {
        stopTimer();
    }
	
	struct MetadataClass
	{
		SN_NODE_ID("tuner");
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
	
	// Scriptnode Callbacks ------------------------------------------------------------------------
    	
	void prepare(PrepareSpecs specs) 
	{
		sampleRate.store(static_cast<float>(specs.sampleRate));
        
        // Optimized buffer size for low tuning (drop F ~87Hz needs ~0.3s window)
        bufferSize = static_cast<int>(sampleRate.load() * 0.25f); 
        
        // Initialize YIN buffers
        yinBuffer.resize(bufferSize, 0.0f);
        differenceFunction.resize(bufferSize / 2, 0.0f);
        cumulativeMeanNormalizedDifference.resize(bufferSize / 2, 0.0f);
        
        // Circular buffer for continuous audio capture
        circularBuffer.resize(bufferSize * 2, 0.0f);
        writeIndex = 0;
        
        // Reset detection state
        lastDetectedPitch.store(0.0f);
        pitchConfidence.store(0.0f);
        isNoteDetected.store(false);
        
        // Initialize pitch history
        std::fill(pitchHistory.begin(), pitchHistory.end(), 0.0f);
        pitchHistoryIndex = 0;
        
        samplesSinceLastDetection = 0;
        totalSamplesCollected = 0;
	}    	

    void reset() 
    {
        writeIndex = 0;
        samplesSinceLastDetection = 0;
        std::fill(circularBuffer.begin(), circularBuffer.end(), 0.0f);
        std::fill(pitchHistory.begin(), pitchHistory.end(), 0.0f);
        pitchHistoryIndex = 0;
        lastDetectedPitch.store(0.0f);
        pitchConfidence.store(0.0f);
        isNoteDetected.store(false);
        totalSamplesCollected = 0;

    }
	
    /*
	template <typename T> void process(T& data)
	{		
        int numSamples = data.getNumSamples();

		for (auto ch : data)
		{
			dyn<float> channelData = data.toChannelData(ch);

	        // Copy audio samples to circular buffer for pitch detection
	        for (int i = 0; i < numSamples; ++i)
	        {
	            circularBuffer[writeIndex] = channelData[i];
	            writeIndex = (writeIndex + 1) % (bufferSize * 2);
	            samplesSinceLastDetection++;
	        }

	        // Mute audio through if monitor is off
	        if (!monitorOutput.load())
	        	channelData.clear();
		}       
	}
    */

    template <typename T> void process(T& data)
    {
        const int numSamples = data.getNumSamples();
        const int numChannels = data.getNumChannels(); // or iterate to count

        for (int i = 0; i < numSamples; ++i)
        {
            float mono = 0.0f;
            for (auto ch : data)
                mono += data.toChannelData(ch)[i];

            mono /= (float)numChannels; // simple average

            // Write exactly one sample per frame
            circularBuffer[writeIndex] = mono;
            writeIndex = (writeIndex + 1) % (bufferSize * 2);

            samplesSinceLastDetection++;
            totalSamplesCollected++;
        }

        // Optionally mute output AFTER copying
        if (!monitorOutput.load())
        {
            for (auto ch : data)
                data.toChannelData(ch).clear();
        }
    }

    juce::Random randomGenerator;

    /*
	void timerCallback() override
    {        
        // Perform pitch detection only if we have enough new samples
        if (samplesSinceLastDetection >= pitchDetectionInterval)
        {
            performPitchDetection();
            samplesSinceLastDetection = 0;
                
            // send detected pitch back to cable
            setGlobalCableValue<GlobalCables::pitch>(lastDetectedPitch.load());
        }
    }
    */
    void timerCallback() override
    {
        if (totalSamplesCollected < bufferSize)
            return; // not enough data to analyze yet

        if (samplesSinceLastDetection >= pitchDetectionInterval)
        {
            performPitchDetection();
            samplesSinceLastDetection = 0;
            float normalizedPitch = normalizePitchForCable(lastDetectedPitch.load());
            setGlobalCableValue<GlobalCables::pitch>(normalizedPitch);
        }
    }

	// Parameter Functions -------------------------------------------------------------------------
	
	template <int P> void setParameter(double v)
	{
		if (P == 0) // Monitor parameter
		{            
            monitorOutput.store(v >= 0.5);
		}
	}
	
	void createParameters(ParameterDataList& data)
	{		
        {
            parameter::data monitor("Monitor", { 0.0, 1.0 });
            registerCallback<0>(monitor);
            monitor.setDefaultValue(0.0);            
            data.add(std::move(monitor));
        }        
	}

    // Public API for getting tuning information
    struct TuningInfo
    {
        int closestStringIndex = -1;
        juce::String noteName = "";
        float centDeviation = 0.0f;
        bool inTune = false;
        float frequency = 0.0f;
        float confidence = 0.0f;
    };
    
    TuningInfo getTuningInfo() const
    {
        TuningInfo info;
        info.frequency = lastDetectedPitch.load();
        info.confidence = pitchConfidence.load();
        
        if (info.frequency > 0.0f && isNoteDetected.load())
        {
            float minDifference = std::numeric_limits<float>::max();
            
            // Check against 7-string drop F tuning
            for (int i = 0; i < sevenStringDropF.size(); ++i)
            {
                float difference = std::abs(info.frequency - sevenStringDropF[i]);
                if (difference < minDifference)
                {
                    minDifference = difference;
                    info.closestStringIndex = i;
                    info.noteName = dropFNames[i];
                }
            }
            
            if (info.closestStringIndex >= 0)
            {
                float targetFreq = sevenStringDropF[info.closestStringIndex];
                info.centDeviation = 1200.0f * std::log2(info.frequency / targetFreq);
                info.inTune = std::abs(info.centDeviation) <= tuningToleranceCents;
            }
        }
        return info;
    }

private:

	std::atomic<bool> monitorOutput{ false };
	std::atomic<float> sampleRate{ 44100.0f };
    int totalSamplesCollected = 0;   // total mono frames written
    int numChannelsCached = 0;       // (optional) store channel count if needed
    
    // YIN Algorithm parameters - optimized for low tuning
    float threshold = 0.12f; // Lower threshold for better low frequency detection
    float minFrequency = 40.0f; // Lower minimum for drop F (87Hz)
    float maxFrequency = 400.0f; // Upper range for guitar fundamentals
    float stabilityFactor = 0.75f; // Stability filtering
    float tuningToleranceCents = 8.0f; // Tolerance for "in tune"

    // Buffer management
    int bufferSize = 8192;
    std::vector<float> circularBuffer;
    std::vector<float> yinBuffer;
    std::vector<float> differenceFunction;
    std::vector<float> cumulativeMeanNormalizedDifference;
    int writeIndex = 0;

    // Detection timing
    int pitchDetectionInterval = 1024; // Samples between detections
    int samplesSinceLastDetection = 0;

    // Pitch history for stability
    static constexpr int pitchHistorySize = 7;
    std::vector<float> pitchHistory;
    int pitchHistoryIndex = 0;

    // Detection results
    std::atomic<float> lastDetectedPitch{ 0.0f };
    std::atomic<float> pitchConfidence{ 0.0f };
    std::atomic<bool> isNoteDetected{ false };

    // 7-string drop F tuning: F1, C2, F2, Bb2, Eb3, G3, C4
    std::vector<float> sevenStringDropF{ 87.31f, 130.81f, 174.61f, 233.08f, 311.13f, 392.0f, 523.25f };
    std::vector<juce::String> dropFNames{ "F1", "C2", "F2", "Bb2", "Eb3", "G3", "C4" };

    void performPitchDetection()
    {
        // Copy circular buffer to YIN buffer
        int startIndex = (writeIndex + bufferSize) % (bufferSize * 2);
        for (int i = 0; i < bufferSize; ++i)
        {
            int index = (startIndex + i) % (bufferSize * 2);
            yinBuffer[i] = circularBuffer[index];
        }

        // Apply window function to reduce spectral leakage
        applyHannWindow(yinBuffer);
        
        // YIN Algorithm steps
        calculateDifferenceFunction();
        calculateCumulativeMeanNormalizedDifference();
        int tau = getAbsoluteThreshold();

        if (tau != -1)
        {
            // Parabolic interpolation for sub-sample precision
            float betterTau = parabolicInterpolation(tau);
            float detectedFreq = sampleRate.load() / betterTau;

            // Validate frequency range
            if (detectedFreq >= minFrequency && detectedFreq <= maxFrequency)
            {
                // Add to pitch history for stability
                pitchHistory[pitchHistoryIndex] = detectedFreq;
                pitchHistoryIndex = (pitchHistoryIndex + 1) % pitchHistorySize;

                // Get stable pitch using weighted average
                float stablePitch = getStablePitch();
                float confidence = 1.0f - cumulativeMeanNormalizedDifference[tau];

                lastDetectedPitch.store(stablePitch);
                pitchConfidence.store(confidence);
                isNoteDetected.store(confidence > 0.5f); // Lower threshold for low frequencies
            }
            else
            {
                // Decay confidence if out of range
                pitchConfidence.store(pitchConfidence.load() * 0.9f);
                isNoteDetected.store(pitchConfidence.load() > 0.4f);
            }
        }
        else
        {
            // Decay confidence if no pitch detected
            pitchConfidence.store(pitchConfidence.load() * 0.85f);
            isNoteDetected.store(pitchConfidence.load() > 0.3f);
        }
    }

    void applyHannWindow(std::vector<float>& buffer)
    {
        int N = static_cast<int>(buffer.size());
        for (int i = 0; i < N; ++i)
        {
            float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (N - 1)));
            buffer[i] *= window;
        }
    }

    void calculateDifferenceFunction()
    {
        int W = bufferSize / 2;
        for (int tau = 0; tau < W; ++tau)
        {
            float sum = 0.0f;
            for (int j = 0; j < W - tau; ++j)
            {
                float diff = yinBuffer[j] - yinBuffer[j + tau];
                sum += diff * diff;
            }
            differenceFunction[tau] = sum;
        }
    }

    void calculateCumulativeMeanNormalizedDifference()
    {
        cumulativeMeanNormalizedDifference[0] = 1.0f;
        float runningSum = 0.0f;

        for (int tau = 1; tau < differenceFunction.size(); ++tau)
        {
            runningSum += differenceFunction[tau];
            cumulativeMeanNormalizedDifference[tau] = 
                (runningSum == 0.0f) ? 1.0f : differenceFunction[tau] * tau / runningSum;
        }
    }

    int getAbsoluteThreshold()
    {
        const float sr = sampleRate.load();
        int minTau = juce::jmax(2, static_cast<int>(sr / maxFrequency));
        int maxTau = juce::jmin(static_cast<int>(sr / minFrequency), 
                               static_cast<int>(cumulativeMeanNormalizedDifference.size()) - 1);

        for (int tau = minTau; tau < maxTau; ++tau)
        {
            if (cumulativeMeanNormalizedDifference[tau] < threshold)
            {
                // Find local minimum
                while (tau + 1 < maxTau &&
                       cumulativeMeanNormalizedDifference[tau + 1] < cumulativeMeanNormalizedDifference[tau])
                {
                    tau++;
                }
                return tau;
            }
        }
        return -1; // No pitch found
    }

    float parabolicInterpolation(int tau)
    {
        if (tau < 1 || tau >= static_cast<int>(cumulativeMeanNormalizedDifference.size()) - 1)
            return static_cast<float>(tau);

        float s0 = cumulativeMeanNormalizedDifference[tau - 1];
        float s1 = cumulativeMeanNormalizedDifference[tau];
        float s2 = cumulativeMeanNormalizedDifference[tau + 1];

        float a = (s0 - 2.0f * s1 + s2) / 2.0f;
        float b = (s2 - s0) / 2.0f;

        return (a == 0.0f) ? static_cast<float>(tau) : static_cast<float>(tau) - b / (2.0f * a);
    }

    float getStablePitch()
    {
        float weightedSum = 0.0f;
        float totalWeight = 0.0f;

        // Use recent history with higher weights for newer samples
        for (int i = 0; i < pitchHistorySize; ++i)
        {
            if (pitchHistory[i] > 0.0f)
            {
                int age = (pitchHistoryIndex - i + pitchHistorySize) % pitchHistorySize;
                float weight = 1.0f / (1.0f + age * 0.1f); // Newer samples have higher weight
                weightedSum += pitchHistory[i] * weight;
                totalWeight += weight;
            }
        }

        if (totalWeight > 0.0f)
        {
            float averagePitch = weightedSum / totalWeight;
            float currentPitch = lastDetectedPitch.load();
            
            return (currentPitch > 0.0f) ? 
                currentPitch * (1.0f - stabilityFactor) + averagePitch * stabilityFactor :
                averagePitch;
        }

        return lastDetectedPitch.load();
    }
    
    float normalizePitchForCable(float frequency)
    {
        if (frequency <= 0.0f) return 0.0f;

        // Map frequency to 0.0-1.0 range (natural log scale from 60Hz to 400Hz)
        float logFreq = std::log(juce::jlimit(minFrequency, maxFrequency, frequency));
        float logMin = std::log(minFrequency);
        float logMax = std::log(maxFrequency);

        return (logFreq - logMin) / (logMax - logMin);
    }
};
}