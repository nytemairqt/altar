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

template <int NV> struct tuner: public data::base, public cable_manager_t, public juce::Timer
{
	// Metadata Definitions ------------------------------------------------------------------------
	
	SNEX_NODE(tuner);    

    
    tuner()
    {
        //startTimerHz(30);

        // maybe this weird ass lambda?

        /*
        this->registerDataCallback<GlobalCables::pitch>([](const var& funky)
        {
            // do stuff
        });
        */
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
        auto numChannels = specs.numChannels;
        auto samplesPerBlock = specs.blockSize;

        // YIN parameters
        //bufferSize = static_cast<int>(sampleRate * 0.2f); // 0.2f seems a good value for low tuning guitar
        bufferSize = static_cast<int>(sampleRate.load() * 0.2f); // 0.2f seems a good value for low tuning guitar
        yinBuffer.resize(bufferSize, 0.0f);
        differenceFunction.resize(bufferSize / 2, 0.0f);
        cumulativeMeanNormalizedDifference.resize(bufferSize / 2, 0.0f);

        circularBuffer.resize(bufferSize * 2, 0.0f); 
        writeIndex = 0;

        // Reset detection state
        lastDetectedPitch.store(0.0f);
        pitchConfidence.store(0.0f);
        isNoteDetected.store(false);

        pitchHistoryIndex = 0;
        std::fill(pitchHistory.begin(), pitchHistory.end(), 0.0f);

        //startTimerHz(30);

        
        //randomGenerator.setSeedRandomly();
        //float randomValue = randomGenerator.nextFloat();
        //setGlobalCableValue<GlobalCables::pitch>(randomValue);
        //if (randomValue >= 0.5) { monitorOutput.store(true); }
        //else { monitorOutput.store(false); }
        
        //startTimerHz(30);
	}

	void reset() 
    {
        //stopTimer();
    }
	
	
		
	template <typename T> void process(T& data)
	{		
        int numSamples = data.getNumSamples();

		for (auto ch : data)
		{
			dyn<float> channelData = data.toChannelData(ch);

	        for (int i = 0; i < numSamples; ++i)
	        {
	            circularBuffer[writeIndex] = channelData[i];
	            writeIndex = (writeIndex + 1) % (bufferSize * 2);
	            if (++samplesSinceLastDetection >= pitchDetectionInterval)            
	                samplesSinceLastDetection = 0; // reset 
	        }

	        // Mute audio through
	        if (!monitorOutput.load())
	        	channelData.clear();	        				        
		}       

        randomGenerator.setSeedRandomly();
        float randomValue = randomGenerator.nextFloat();
        setGlobalCableValue<GlobalCables::pitch>(randomValue);
	}

	struct TuningInfo
    {
        int closestStringIndex = -1;
        juce::String noteName = "";
        float centDeviation = 0.0f;
        bool inTune = false;
    };
	
	TuningInfo getTuningInfo() const
    {
        TuningInfo info;
        float detectedPitch = lastDetectedPitch.load();

        if (detectedPitch > 0.0f && isNoteDetected.load())
        {
            float minDifference = std::numeric_limits<float>::max();

            for (int i = 0; i < standardTuning.size(); ++i)
            {
                float difference = std::abs(detectedPitch - standardTuning[i]);
                if (difference < minDifference)
                {
                    minDifference = difference;
                    info.closestStringIndex = i;
                    info.noteName = tuningNames[i];
                }
            }
            if (info.closestStringIndex >= 0)
            {
                float targetFreq = standardTuning[info.closestStringIndex];
                info.centDeviation = 1200.0f * std::log2(detectedPitch / targetFreq);
                info.inTune = std::abs(info.centDeviation) <= tuningToleranceCents;
            }
        }
        return info;
    }

    juce::Random randomGenerator;

	void timerCallback() override
    {
        //randomGenerator.setSeedRandomly();
        performPitchDetection();

        randomGenerator.setSeedRandomly();
        float randomValue = randomGenerator.nextFloat();
        //setGlobalCableValue<GlobalCables::pitch>(randomValue);
    } 		    

	// Parameter Functions -------------------------------------------------------------------------
	
	template <int P> void setParameter(double v)
	{
		if (P == 0)
		{            
            if (v >= 0.5) { monitorOutput.store(true); }
            else { monitorOutput.store(false); }    
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

private:

	std::atomic<bool> monitorOutput{ false };
	std::atomic<float> sampleRate{ 44100.0f };
    float detectedPitch = 0.0f;    

    // YIN parameters
    float threshold = 0.15f; // sensitivity 
    float minFrequency = 20.0f;
    float maxFrequency = 500.0f; 
    float stabilityFactor = 0.7f; 

    float tuningToleranceCents = 5.0f; 

    int bufferSize = 4096;
    std::vector<float> circularBuffer;
    std::vector<float> yinBuffer;
    std::vector<float> differenceFunction;
    std::vector<float> cumulativeMeanNormalizedDifference;
    int writeIndex = 0;

    int pitchDetectionInterval = 512; 
    int samplesSinceLastDetection = 0;

    static constexpr int pitchHistorySize = 5;
    std::vector<float> pitchHistory;
    int pitchHistoryIndex = 0;

    std::atomic<float> lastDetectedPitch{ 0.0f };
    std::atomic<float> pitchConfidence{ 0.0f };
    std::atomic<bool> isNoteDetected{ false };

    std::vector<float> standardTuning{ 440.0f };
    std::vector<juce::String> tuningNames{ "E2", "A2", "D3", "G3", "B3", "E4" };

    void performPitchDetection()
    {
        // Copy buffer to process
        int startIndex = writeIndex >= bufferSize ? writeIndex - bufferSize :
            (bufferSize * 2) + writeIndex - bufferSize;

        for (int i = 0; i < bufferSize; ++i)
        {
            int index = (startIndex + i) % (bufferSize * 2);
            yinBuffer[i] = circularBuffer[index];
        }

        applyHannWindow(yinBuffer);
        calculateDifferenceFunction();
        calculateCumulativeMeanNormalizedDifference();
        int tau = getAbsoluteThreshold();

        if (tau != -1)
        {
            // YIN Algorithm Step 4: Parabolic interpolation
            float betterTau = parabolicInterpolation(tau);
            //detectedPitch = sampleRate / betterTau;
            detectedPitch = sampleRate.load() / betterTau;

            //DBG("detectedPitch: " << detectedPitch);

            // Validate frequency range
            if (detectedPitch >= minFrequency && detectedPitch <= maxFrequency)
            {
                // Add to pitch history for stability
                pitchHistory[pitchHistoryIndex] = detectedPitch;
                pitchHistoryIndex = (pitchHistoryIndex + 1) % pitchHistorySize;

                // Calculate stable pitch using weighted average
                float stablePitch = getStablePitch();

                // Calculate confidence based on clarity of detection
                float confidence = 1.0f - cumulativeMeanNormalizedDifference[tau];

                lastDetectedPitch.store(stablePitch);
                pitchConfidence.store(confidence);
                isNoteDetected.store(confidence > 0.7f);

            }
            else
            {
                pitchConfidence.store(pitchConfidence.load() * 0.9f);
                isNoteDetected.store(pitchConfidence.load() > 0.5f);
            }
        }
        else
        {
            pitchConfidence.store(pitchConfidence.load() * 0.8f);
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
            for (int j = 0; j < W; ++j)
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
            if (runningSum == 0.0f)
            {
                cumulativeMeanNormalizedDifference[tau] = 1.0f;
            }
            else
            {
                cumulativeMeanNormalizedDifference[tau] =
                    differenceFunction[tau] * tau / runningSum;
            }
        }
    }

    int getAbsoluteThreshold()
    {
        int minTau = static_cast<int>(sampleRate / maxFrequency);
        int maxTau = static_cast<int>(sampleRate / minFrequency);

        minTau = juce::jmax(2, juce::jmin(minTau, static_cast<int>(cumulativeMeanNormalizedDifference.size()) - 1));
        maxTau = juce::jmax(minTau, juce::jmin(maxTau, static_cast<int>(cumulativeMeanNormalizedDifference.size()) - 1));

        for (int tau = minTau; tau < maxTau; ++tau)
        {
            if (cumulativeMeanNormalizedDifference[tau] < threshold)
            {
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

        if (a == 0.0f)
            return static_cast<float>(tau);

        return static_cast<float>(tau) - b / (2.0f * a);
    }

    float getStablePitch()
    {
        float weightedSum = 0.0f;
        float totalWeight = 0.0f;

        for (int i = 0; i < pitchHistorySize; ++i)
        {
            if (pitchHistory[i] > 0.0f)
            {
                float weight = 1.0f; 
                weightedSum += pitchHistory[i] * weight;
                totalWeight += weight;
            }
        }

        if (totalWeight > 0.0f)
        {
            float averagePitch = weightedSum / totalWeight;
            float currentPitch = lastDetectedPitch.load();

            if (currentPitch > 0.0f)
                return currentPitch * (1.0f - stabilityFactor) + averagePitch * stabilityFactor;
            else
                return averagePitch;
        }

        return lastDetectedPitch.load();
    }
};
}


