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

#include "src/dependencies/signalsmith-stretch/signalsmith-stretch.h"

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

	signalsmith::stretch::SignalsmithStretch<float> stretch;
		
	// still using a ratio because we already have the conversion in the interface logic
	double ratio = 1.0; 
	
	int maxBlockSize = 0; 

	// stretcher needs unique in & out, so we need 
	// a temp buffer to hold our intermediate samples:	
	juce::AudioBuffer<float> tmp; // probably better to use a span here

	const int numChannels = 2;
	int blockSamples = 4096;
    int intervalSamples = 512;
    int numInput = 512;    
	
	void prepare(PrepareSpecs specs)
	{
		maxBlockSize = (int)specs.blockSize; 
		
		stretch.presetDefault(specs.numChannels, specs.sampleRate);
		stretch.setTransposeFactor(ratio);

		tmp.setSize(numChannels, maxBlockSize, false, false, true);
	}
	
	void reset()
	{		
		stretch.reset();
	}
	
	void handleHiseEvent(HiseEvent& e){}	
	
	template <typename T> void process(T& data)
	{
		auto ptrs = data.getRawDataPointers();
		const int numSamples = data.getNumSamples();
		
		if (tmp.getNumSamples() < numSamples)
			tmp.setSize(numChannels, numSamples, false, false, true);
		
		float* in[2]  = { ptrs[0], ptrs[1] };
		float* out[2] = { tmp.getWritePointer(0), tmp.getWritePointer(1) };
		
		// requires the in and out pointers to be unique
		stretch.process(in, numSamples, out, numSamples);
		
		// now write back to the original pointers
		std::memcpy(ptrs[0], out[0], sizeof(float) * numSamples);
		std::memcpy(ptrs[1], out[1], sizeof(float) * numSamples);
	}
			
	template <int P> void setParameter(double v)
	{
		if (P == 0)
		{
			ratio = v;			
			stretch.setTransposeFactor(ratio);
		}
	}
	
	void createParameters(ParameterDataList& data)
	{
		{
			parameter::data p("FreqRatio", { 0.25, 4.0 }); // -24 to 24 semitones
			registerCallback<0>(p);
			p.setDefaultValue(1.0);
			data.add(std::move(p));
		}
	}
};
}