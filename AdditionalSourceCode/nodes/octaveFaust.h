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

namespace octaveFaust_impl
{
// ====================| Node & Parameter type declarations |====================

template <int NV>
using octaveFaust_t_ = container::chain<parameter::empty, 
                                        wrap::fix<2, project::octave_faust<NV>>>;

// ========================| Root node initialiser class |========================

template <int NV> struct instance: public octaveFaust_impl::octaveFaust_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(octaveFaust);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(2)
		{
			0x0000, 0x0000
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
		// Node References ------------------------------------------------------
		
		auto& faust1 = this->getT(0); // project::octave_faust<NV>
		
		// Default Values -------------------------------------------------------
		
		faust1.setParameterT(0, 300.);  // core::faust::Freq
		faust1.setParameterT(1, 1.);    // core::faust::Mix
		faust1.setParameterT(2, 1000.); // core::faust::window
		faust1.setParameterT(3, 16.);   // core::faust::xfade
		
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
// =============================| Public Definition |=============================

namespace project
{
// polyphonic template declaration

template <int NV>
using octaveFaust = wrap::node<octaveFaust_impl::instance<NV>>;
}


