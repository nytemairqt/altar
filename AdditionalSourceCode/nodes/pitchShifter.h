#pragma once

// These will improve the readability of the connection definition

#define getT(Idx) template get<Idx>()
#define connectT(Idx, target) template connect<Idx>(target)
#define getParameterT(Idx) template getParameter<Idx>()
#define setParameterT(Idx, value) template setParameter<Idx>(value)
#define setParameterWT(Idx, value) template setWrapParameter<Idx>(value)
using namespace scriptnode;
using namespace snex;
using namespace snex::Types;

namespace pitchShifter_impl
{
// ===========================| Node & Parameter type declarations |===========================

namespace pitchShifter_t_parameters
{
// Parameter list for pitchShifter_impl::pitchShifter_t --------------------------------------

using FreqRatio = parameter::empty;
template <int NV>
using ShiftSemitones = parameter::plain<project::pitch_shifter_faust<NV>, 
                                        0>;
template <int NV>
using pitchShifter_t_plist = parameter::list<FreqRatio, ShiftSemitones<NV>>;
}

template <int NV>
using pitchShifter_t_ = container::chain<pitchShifter_t_parameters::pitchShifter_t_plist<NV>, 
                                         wrap::fix<2, project::pitch_shifter_faust<NV>>>;

// ==============================| Root node initialiser class |==============================

template <int NV> struct instance: public pitchShifter_impl::pitchShifter_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(pitchShifter);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(40)
		{
			0x005C, 0x0000, 0x0000, 0x7246, 0x7165, 0x6152, 0x6974, 0x006F, 
            0x0000, 0x3F00, 0x0000, 0x4000, 0x0000, 0x3F00, 0x0000, 0x3F80, 
            0xD70A, 0x3C23, 0x005C, 0x0001, 0x0000, 0x6853, 0x6669, 0x5374, 
            0x6D65, 0x7469, 0x6E6F, 0x7365, 0x0000, 0x4000, 0x00C1, 0x4000, 
            0x0041, 0x4000, 0x00C1, 0x8000, 0x003F, 0x8000, 0x003F, 0x0000
		};
		SNEX_METADATA_ENCODED_MOD_INFO(17)
		{
			0x003A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
            0x0000
		};
	};
	
	instance()
	{
		// Node References -------------------------------------------------------------------
		
		auto& faust = this->getT(0); // project::pitch_shifter_faust<NV>
		
		// Parameter Connections -------------------------------------------------------------
		
		this->getParameterT(1).connectT(0, faust); // ShiftSemitones -> faust::shiftsemitones
		
		// Default Values --------------------------------------------------------------------
		
		;                              // faust::shiftsemitones is automated
		faust.setParameterT(1, 2048.); // core::faust::windowsamples
		faust.setParameterT(2, 128.);  // core::faust::xfadesamples
		
		this->setParameterT(0, 0.5);
		this->setParameterT(1, -12.);
	}
	
	static constexpr bool isPolyphonic() { return NV > 1; };
	
	static constexpr bool hasTail() { return true; };
	
	static constexpr bool isSuspendedOnSilence() { return false; };
};
}

#undef getT
#undef connectT
#undef setParameterT
#undef setParameterWT
#undef getParameterT
// ===================================| Public Definition |===================================

namespace project
{
// polyphonic template declaration

template <int NV>
using pitchShifter = wrap::node<pitchShifter_impl::instance<NV>>;
}


