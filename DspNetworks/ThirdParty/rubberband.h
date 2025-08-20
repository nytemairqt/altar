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

template <int NV> struct rubberband: public data::base
{
	// Metadata Definitions ------------------------------------------------------------------------
	
	SNEX_NODE(rubberband);
	
	struct MetadataClass
	{
		SN_NODE_ID("rubberband");
	};
	
	// set to true if you want this node to have a modulation dragger
	static constexpr bool isModNode() { return false; };
	static constexpr bool isPolyphonic() { return NV > 1; };
	// set to true if your node produces a tail
	static constexpr bool hasTail() { return false; };
	// set to true if your doesn't generate sound from silence and can be suspended when the input signal is silent
	static constexpr bool isSuspendedOnSilence() { return false; };
	// Undefine this method if you want a dynamic channel count
	static constexpr int getFixChannelAmount() { return 2; };
	
	// Define the amount and types of external data slots you want to use
	static constexpr int NumTables = 0;
	static constexpr int NumSliderPacks = 0;
	static constexpr int NumAudioFiles = 0;
	static constexpr int NumFilters = 0;
	static constexpr int NumDisplayBuffers = 0;

	// declare a unique_ptr to store our shifter
	std::unique_ptr<RubberBand::RubberBandStretcher> rb;
	
	// Scriptnode Callbacks ------------------------------------------------------------------------
	
	void prepare(PrepareSpecs specs)
	{		
		// assign the shifter to our pointer
		rb = std::make_unique<RubberBand::RubberBandStretcher> (specs.sampleRate, specs.numChannels, RubberBand::RubberBandStretcher::Option::OptionProcessRealTime);
		rb->reset(); // not sure if this is necessary, but I don't think it hurts		
	}
	
	void reset(){}		
	void handleHiseEvent(HiseEvent& e){}	
	
	template <typename T> void process(T& data)
	{
		if (!rb) return; // safety check in case the rb object isn't constructed yet
		
		int numSamples = data.getNumSamples();
		auto ptrs = data.getRawDataPointers(); // RB needs pointers

		rb->process(ptrs, numSamples, false); // 

		int available = rb->available();

		if (available >= numSamples)
		{
			// We have enough output, retrieve it directly
			rb->retrieve(ptrs, numSamples);
		}			
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
			if (!rb) return; // Safety check in case the Parameter is triggered before the rb object is constructed
			rb->setPitchScale(v); 
		}
		
	}
	
	void createParameters(ParameterDataList& data)
	{
		{
			// Create a parameter like this
			parameter::data p("FreqRatio", { 0.5, 2.0 });
			// The template parameter (<0>) will be forwarded to setParameter<P>()
			registerCallback<0>(p);
			p.setDefaultValue(1.0);
			data.add(std::move(p));
		}
	}
};
}


