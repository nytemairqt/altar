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

enum class GlobalCablesTuner
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


template <int NV> struct tuner: public data::base, public cable_manager_t, public hise::DllTimer
{
	
	SNEX_NODE(tuner);    

    tuner()
    {
        startTimer(100); 
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
        
        bufferSize = static_cast<int>(sampleRate.load() * 0.25f); 
        
        yinBuffer.resize(bufferSize, 0.0f);
        differenceFunction.resize(bufferSize / 2, 0.0f);
        cumulativeMeanNormalizedDifference.resize(bufferSize / 2, 0.0f);
        
        circularBuffer.resize(bufferSize * 2, 0.0f);
        writeIndex = 0;
        
        lastDetectedPitch.store(0.0f);
        pitchConfidence.store(0.0f);
        isNoteDetected.store(false);
        
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

    template <typename T> void process(T& data)
    {
        const int numSamples = data.getNumSamples();
        const int numChannels = data.getNumChannels(); 

        for (int i = 0; i < numSamples; ++i)
        {
            float mono = 0.0f;
            for (auto ch : data)
                mono += data.toChannelData(ch)[i];

            mono /= (float)numChannels; 

            circularBuffer[writeIndex] = mono;
            writeIndex = (writeIndex + 1) % (bufferSize * 2);

            samplesSinceLastDetection++;
            totalSamplesCollected++;
        }

        if (!monitorOutput.load())
        {
            for (auto ch : data)
                data.toChannelData(ch).clear();
        }
    }

    void timerCallback() override
    {
        if (totalSamplesCollected < bufferSize)
            return; // not enough data to analyze yet

        if (samplesSinceLastDetection >= pitchDetectionInterval)
        {
            performPitchDetection();
            samplesSinceLastDetection = 0;
            float normalizedPitch = normalizePitchForCable(lastDetectedPitch.load());
            setGlobalCableValue<GlobalCablesTuner::pitch>(normalizedPitch);
        }
    }

	// Parameter Functions -------------------------------------------------------------------------
	
	template <int P> void setParameter(double v)
	{
		if (P == 0) { monitorOutput.store(v >= 0.5); }
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
    int totalSamplesCollected = 0;   
    int numChannelsCached = 0;       
    
    float threshold = 0.12f;
    float minFrequency = 40.0f;
    float maxFrequency = 400.0f;
    float stabilityFactor = 0.75f;
    float tuningToleranceCents = 8.0f;
    
    int bufferSize = 8192;
    std::vector<float> circularBuffer;
    std::vector<float> yinBuffer;
    std::vector<float> differenceFunction;
    std::vector<float> cumulativeMeanNormalizedDifference;
    int writeIndex = 0;

    int pitchDetectionInterval = 1024; 
    int samplesSinceLastDetection = 0;

    static constexpr int pitchHistorySize = 7;
    std::vector<float> pitchHistory;
    int pitchHistoryIndex = 0;

    std::atomic<float> lastDetectedPitch{ 0.0f };
    std::atomic<float> pitchConfidence{ 0.0f };
    std::atomic<bool> isNoteDetected{ false };

    // default to 7 string drop F
    std::vector<float> sevenStringDropF{ 87.31f, 130.81f, 174.61f, 233.08f, 311.13f, 392.0f, 523.25f };
    std::vector<juce::String> dropFNames{ "F1", "C2", "F2", "Bb2", "Eb3", "G3", "C4" };

    void performPitchDetection()
    {
        int startIndex = (writeIndex + bufferSize) % (bufferSize * 2);
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
            float betterTau = parabolicInterpolation(tau);
            float detectedFreq = sampleRate.load() / betterTau;

            if (detectedFreq >= minFrequency && detectedFreq <= maxFrequency)
            {
                pitchHistory[pitchHistoryIndex] = detectedFreq;
                pitchHistoryIndex = (pitchHistoryIndex + 1) % pitchHistorySize;

                float stablePitch = getStablePitch();
                float confidence = 1.0f - cumulativeMeanNormalizedDifference[tau];

                lastDetectedPitch.store(stablePitch);
                pitchConfidence.store(confidence);
                isNoteDetected.store(confidence > 0.5f);
            }
            else
            {
                pitchConfidence.store(pitchConfidence.load() * 0.9f);
                isNoteDetected.store(pitchConfidence.load() > 0.4f);
            }
        }
        else
        {
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
                while (tau + 1 < maxTau &&
                       cumulativeMeanNormalizedDifference[tau + 1] < cumulativeMeanNormalizedDifference[tau])
                {
                    tau++;
                }
                return tau;
            }
        }
        return -1; // none detected
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

        for (int i = 0; i < pitchHistorySize; ++i)
        {
            if (pitchHistory[i] > 0.0f)
            {
                int age = (pitchHistoryIndex - i + pitchHistorySize) % pitchHistorySize;
                float weight = 1.0f / (1.0f + age * 0.1f); 
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

        float logFreq = std::log(juce::jlimit(minFrequency, maxFrequency, frequency));
        float logMin = std::log(minFrequency);
        float logMax = std::log(maxFrequency);

        return (logFreq - logMin) / (logMax - logMin);
    }
};
}