// ==================================| Third Party Node Template |==================================

/*

Don't forget, you have to replace every instance of "Rectangle" in HISE/hi_tools/simple_css/Renderer.h to juce::Rectangle

And replace both "copy" variables in that same file with copyRectangle

Then build HISE with HI_ENABLE_CUSTOM_NODES=1 preprocessor!

*/

#pragma once
#include <JuceHeader.h>

#include "src/rubberband/single/RubberBandSingle.cpp"
#include "src/rubberband/rubberband/RubberBandStretcher.h"

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;

// ==========================| The node class with all required callbacks |==========================

template <int NV> struct transpose: public data::base
{
	// Metadata Definitions ------------------------------------------------------------------------
	
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

	// Rubber Band (mono) — we will mirror to right channel at the end
	std::unique_ptr<RubberBand::RubberBandStretcher> rb;
	const int numChannels = 1; // mono in the stretcher
	
	// Startup pad/drop management for RealTime mode
	size_t startPad = 0;              // value from getPreferredStartPad()
	size_t startDelay = 0;            // value from getStartDelay()
	size_t remainingPad = 0;          // how many pad samples we still need to feed
	size_t remainingDrop = 0;         // how many output samples we still need to drop
	
	// Current pitch scale (so we can recompute pad/delay on reset consistently)
	double pitchScale = 1.0;
	
	// Scratch buffers (pre-allocated in prepare)
	juce::HeapBlock<float> padBuffer;       // zeros for priming
	juce::HeapBlock<float> retrieveBuffer;  // temporary output retrieval before drop/copy
	int maxBlockSize = 0;

	// Scriptnode Callbacks ------------------------------------------------------------------------
	
	void prepare(PrepareSpecs specs)
	{
		maxBlockSize = (int)specs.blockSize; // make sure PrepareSpecs provides this; HISE does
		
		// Create stretcher with low-latency options (your existing choices are good)
		rb = std::make_unique<RubberBand::RubberBandStretcher>(
			specs.sampleRate,
			numChannels,
			RubberBand::RubberBandStretcher::Option::OptionProcessRealTime
			| RubberBand::RubberBandStretcher::Option::OptionEngineFaster
			| RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency
			| RubberBand::RubberBandStretcher::Option::OptionWindowShort
		);
		
		rb->reset();
		
		// Ensure initial pitch/time are set BEFORE querying pad/delay
		rb->setPitchScale(pitchScale);
		
		// Query startup pad and output delay for RealTime mode
		startPad = rb->getPreferredStartPad();
		startDelay = rb->getStartDelay();
		remainingPad = startPad;
		remainingDrop = startDelay;
		
		// Preallocate scratch buffers for RT safety
		const int padCapacity = jmax((int)startPad, maxBlockSize);
		const int retrieveCapacity = jmax((int)startDelay, maxBlockSize);
		
		padBuffer.allocate((size_t)padCapacity, true);         // zero-initialized
		retrieveBuffer.allocate((size_t)retrieveCapacity, false);
	}
	
	void reset()
	{
		if (!rb) return;
		rb->reset();
		rb->setPitchScale(pitchScale);
		
		// Re-query pad/delay on reset
		startPad = rb->getPreferredStartPad();
		startDelay = rb->getStartDelay();
		remainingPad = startPad;
		remainingDrop = startDelay;

		// Ensure scratch buffers are large enough for new values
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
		
		// 1) Prime the stretcher with silent input once (preferred pad), then process the real input.
		// We pass only 1 channel to RB because rb was constructed as mono.
		float* rbInPtrs[1] = { nullptr };
		
		// Feed any remaining silent pad before first "true" process
		if (remainingPad > 0)
		{
			// We can push the entire remainingPad at once using our preallocated zero buffer
			const int padNow = (int)remainingPad;
			rbInPtrs[0] = padBuffer.getData();
			rb->process(rbInPtrs, padNow, false);
			remainingPad = 0; // pad fully supplied
		}
		
		// Now feed the actual input block to RB (mono: take left channel)
		rbInPtrs[0] = ptrs[0];
		rb->process(rbInPtrs, numSamples, false);
		
		// 2) Retrieve output, dropping the initial startDelay samples, and fill the host buffer fully wet.
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
				// Discard the first remainingDrop samples from the stream
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
				// No more to drop, copy straight to output
				std::memcpy(ptrs[0] + written, retrieveBuffer.getData(), sizeof(float) * toGet);
				written += toGet;
			}
		}
		
		// If not enough output was available this cycle, zero-fill the remainder to stay 100% wet
		if (written < numSamples)
		{
			std::memset(ptrs[0] + written, 0, sizeof(float) * (numSamples - written));
		}
		
		// Mirror mono to right channel to keep 2ch node contract
		std::memcpy(ptrs[1], ptrs[0], sizeof(float) * numSamples);
	}
	
	template <typename T> void processFrame(T& data){}
	
	int handleModulation(double& value)
	{	
		return 0;		
	}
	
	void setExternalData(const ExternalData& data, int index)
	{
		
	}
	
	// Parameter Functions -------------------------------------------------------------------------
	
	template <int P> void setParameter(double v)
	{
		if (P == 0)
		{
			pitchScale = v;
			if (rb)
			{
				rb->setPitchScale(pitchScale);
				// Note: We do NOT reapply startPad/startDelay mid-stream.
				// For scheduled ratio changes, you may want to apply them early by the current getStartDelay().
				// If you call reset(), pad/drop will be recomputed and applied again from stream start.
			}
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