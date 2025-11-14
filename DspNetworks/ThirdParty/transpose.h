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

/*

you have to replace every instance of "Rectangle" in HISE/hi_tools/simple_css/Renderer.h to juce::Rectangle

And replace both "copy" variables in that same file with copyRectangle

Then build HISE with HI_ENABLE_CUSTOM_NODES=1 preprocessor!

Don't forget to extract SDK

*/

#pragma once
#include <JuceHeader.h>

#include "src/dependencies/rubberband/single/RubberBandSingle.cpp"
#include "src/dependencies/rubberband/rubberband/RubberBandStretcher.h"

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;

template <int NV> struct transpose: public data::base
{
	
	SNEX_NODE(transpose);
	
	struct MetadataClass
	{
		SN_NODE_ID("transpose");
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

	template <typename T> void processFrame(T& data){}	
	int handleModulation(double& value) { return 0; }	
	void setExternalData(const ExternalData& data, int index) {}	

	std::unique_ptr<RubberBand::RubberBandStretcher> rb;
	const int numChannels = 1; // stretcher is mono
	
	size_t startPad = 0;              
	size_t startDelay = 0;            
	size_t remainingPad = 0;          
	size_t remainingDrop = 0;         
	
	double pitchScale = 1.0;
	
	juce::HeapBlock<float> padBuffer;       
	juce::HeapBlock<float> retrieveBuffer;  
	int maxBlockSize = 0;
	
	void prepare(PrepareSpecs specs)
	{
		maxBlockSize = (int)specs.blockSize; 
		
		rb = std::make_unique<RubberBand::RubberBandStretcher>(
			specs.sampleRate,
			numChannels,
			RubberBand::RubberBandStretcher::Option::OptionProcessRealTime
			| RubberBand::RubberBandStretcher::Option::OptionEngineFaster
			| RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency	
			| RubberBand::RubberBandStretcher::Option::OptionTransientsCrisp		
			| RubberBand::RubberBandStretcher::Option::OptionWindowStandard			
		);
		
		rb->reset();		
		rb->setPitchScale(pitchScale);
		
		startPad = rb->getPreferredStartPad();
		startDelay = rb->getStartDelay();
		remainingPad = startPad;
		remainingDrop = startDelay;
		
		const int padCapacity = jmax((int)startPad, maxBlockSize);
		const int retrieveCapacity = jmax((int)startDelay, maxBlockSize);
		
		padBuffer.allocate((size_t)padCapacity, true);
		retrieveBuffer.allocate((size_t)retrieveCapacity, false);
	}
	
	void reset()
	{
		if (!rb) return;
		rb->reset();
		rb->setPitchScale(pitchScale);
		
		startPad = rb->getPreferredStartPad();
		startDelay = rb->getStartDelay();
		remainingPad = startPad;
		remainingDrop = startDelay;

		const int padCapacity = jmax((int)startPad, maxBlockSize);
		const int retrieveCapacity = jmax((int)startDelay, maxBlockSize);
		padBuffer.allocate((size_t)padCapacity, true);
		retrieveBuffer.allocate((size_t)retrieveCapacity, false);
	}
	
	void handleHiseEvent(HiseEvent& e){}	
	
	template <typename T> void process(T& data)
	{
		if (!rb) return;
		
		const int numSamples = data.getNumSamples();
		auto ptrs = data.getRawDataPointers(); // ptrs[0], ptrs[1]
				
		float* rbInPtrs[1] = { nullptr };
		
		if (remainingPad > 0)
		{
			const int padNow = (int)remainingPad;
			rbInPtrs[0] = padBuffer.getData();
			rb->process(rbInPtrs, padNow, false);
			remainingPad = 0; // pad fully supplied
		}
		
		rbInPtrs[0] = ptrs[0];
		rb->process(rbInPtrs, numSamples, false);
		
		int written = 0;
		while (written < numSamples)
		{
			int avail = rb->available();
			if (avail <= 0) break; // nothing available yet
			
			const int toGet = jmin(avail, numSamples - written);
			
			float* rbOutPtrs[1] = { retrieveBuffer.getData() };
			rb->retrieve(rbOutPtrs, toGet);
			
			if (remainingDrop > 0)
			{
				if ((size_t)toGet <= remainingDrop)
				{
					remainingDrop -= (size_t)toGet;
					continue; // all retrieved samples were dropped
				}
				else
				{
					const int keep = toGet - (int)remainingDrop;
					const int start = (int)remainingDrop;
					std::memcpy(ptrs[0] + written, retrieveBuffer.getData() + start, sizeof(float) * keep);
					written += keep;
					remainingDrop = 0;
				}
			}
			else
			{
				// no more to drop, copy to output
				std::memcpy(ptrs[0] + written, retrieveBuffer.getData(), sizeof(float) * toGet);
				written += toGet;
			}
		}
		
		// if not enough output available, zero-fill the remainder
		if (written < numSamples)
		{
			std::memset(ptrs[0] + written, 0, sizeof(float) * (numSamples - written));
		}
		
		std::memcpy(ptrs[1], ptrs[0], sizeof(float) * numSamples); // back to stereo
	}
			
	template <int P> void setParameter(double v)
	{
		if (P == 0)
		{
			pitchScale = v;
			if (rb) { rb->setPitchScale(pitchScale); }
		}
	}
	
	void createParameters(ParameterDataList& data)
	{
		{
			parameter::data p("FreqRatio", { 0.25, 4.0 });
			registerCallback<0>(p);
			p.setDefaultValue(1.0);
			data.add(std::move(p));
		}
	}
};
}