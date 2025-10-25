#pragma once

#include "cabDesigner.h"
#include "cabDesigner.h"
// These will improve the readability of the connection definition

#define getT(Idx) template get<Idx>()
#define connectT(Idx, target) template connect<Idx>(target)
#define getParameterT(Idx) template getParameter<Idx>()
#define setParameterT(Idx, value) template setParameter<Idx>(value)
#define setParameterWT(Idx, value) template setWrapParameter<Idx>(value)
using namespace scriptnode;
using namespace snex;
using namespace snex::Types;

namespace cabDesignerNetwork_impl
{
// ==============================| Node & Parameter type declarations |==============================

struct file_player_t_data: public data::embedded::multichannel_data
{
	// Metadata functions for embedded data --------------------------------------------------------
	
	int    getNumSamples()     const override { return 1024; }
	double getSamplerate()     const override { return 44100.; }
	int    getNumChannels()    const override { return 1; }
	const float* getChannelData(int index) override
	{
		if(index == 0) { return reinterpret_cast<const float*>(audiodata::cabDesignerNetwork_impl_file_player_t_data0); }
		jassertfalse;    return nullptr;
	};
};

template <int NV>
using file_player_t = wrap::data<core::file_player<NV>, 
                                 data::embedded::audiofile<file_player_t_data>>;
using fft_t = wrap::data<analyse::fft, 
                         data::external::displaybuffer<0>>;

template <int NV>
using chain_t = container::chain<parameter::empty, 
                                 wrap::fix<2, math::clear<NV>>, 
                                 file_player_t<NV>, 
                                 core::gain<NV>, 
                                 project::cabDesigner<NV>, 
                                 filters::svf_eq<NV>, 
                                 fft_t, 
                                 math::clear<NV>>;

template <int NV>
using chain1_t = container::chain<parameter::empty, 
                                  wrap::fix<2, project::cabDesigner<NV>>, 
                                  core::gain<NV>>;

template <int NV>
using split_t = container::split<parameter::empty, 
                                 wrap::fix<2, chain_t<NV>>, 
                                 chain1_t<NV>>;

namespace cabDesignerNetwork_t_parameters
{
// Parameter list for cabDesignerNetwork_impl::cabDesignerNetwork_t --------------------------------

template <int NV>
using SpeakerType = parameter::chain<ranges::Identity, 
                                     parameter::plain<project::cabDesigner<NV>, 0>, 
                                     parameter::plain<project::cabDesigner<NV>, 0>>;

template <int NV>
using CustomMod = parameter::chain<ranges::Identity, 
                                   parameter::plain<project::cabDesigner<NV>, 1>, 
                                   parameter::plain<project::cabDesigner<NV>, 1>>;

template <int NV>
using MicrophoneType = parameter::chain<ranges::Identity, 
                                        parameter::plain<project::cabDesigner<NV>, 2>, 
                                        parameter::plain<project::cabDesigner<NV>, 2>>;

template <int NV>
using MojoStrength = parameter::chain<ranges::Identity, 
                                      parameter::plain<project::cabDesigner<NV>, 3>, 
                                      parameter::plain<project::cabDesigner<NV>, 3>>;

template <int NV>
using GenerateMojo = parameter::chain<ranges::Identity, 
                                      parameter::plain<project::cabDesigner<NV>, 4>, 
                                      parameter::plain<project::cabDesigner<NV>, 4>>;

template <int NV>
using CabAge = parameter::chain<ranges::Identity, 
                                parameter::plain<project::cabDesigner<NV>, 5>, 
                                parameter::plain<project::cabDesigner<NV>, 5>>;

template <int NV>
using FFTGain = parameter::plain<core::gain<NV>, 0>;
template <int NV> using Gain = FFTGain<NV>;
template <int NV>
using cabDesignerNetwork_t_plist = parameter::list<SpeakerType<NV>, 
                                                   CustomMod<NV>, 
                                                   MicrophoneType<NV>, 
                                                   MojoStrength<NV>, 
                                                   GenerateMojo<NV>, 
                                                   CabAge<NV>, 
                                                   FFTGain<NV>, 
                                                   Gain<NV>>;
}

template <int NV>
using cabDesignerNetwork_t_ = container::chain<cabDesignerNetwork_t_parameters::cabDesignerNetwork_t_plist<NV>, 
                                               wrap::fix<2, split_t<NV>>>;

// =================================| Root node initialiser class |=================================

template <int NV> struct instance: public cabDesignerNetwork_impl::cabDesignerNetwork_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 1;
		
		SNEX_METADATA_ID(cabDesignerNetwork);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(150)
		{
			0x005C, 0x0000, 0x0000, 0x7053, 0x6165, 0x656B, 0x5472, 0x7079, 
            0x0065, 0x0000, 0x0000, 0x0000, 0x8000, 0x0040, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x0000, 0x5C00, 0x0100, 0x0000, 0x4300, 0x7375, 
            0x6F74, 0x4D6D, 0x646F, 0x0000, 0x0000, 0x0000, 0x0000, 0x3F80, 
            0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 0x0002, 
            0x0000, 0x694D, 0x7263, 0x706F, 0x6F68, 0x656E, 0x7954, 0x6570, 
            0x0000, 0x0000, 0x0000, 0x0000, 0x4080, 0x0000, 0x0000, 0x0000, 
            0x3F80, 0x0000, 0x0000, 0x005C, 0x0003, 0x0000, 0x6F4D, 0x6F6A, 
            0x7453, 0x6572, 0x676E, 0x6874, 0x0000, 0x0000, 0x0000, 0x0000, 
            0x3F80, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 
            0x0004, 0x0000, 0x6547, 0x656E, 0x6172, 0x6574, 0x6F4D, 0x6F6A, 
            0x0000, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 0x0000, 0x0000, 
            0x3F80, 0x0000, 0x0000, 0x005C, 0x0005, 0x0000, 0x6143, 0x4162, 
            0x6567, 0x0000, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 0x0000, 
            0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 0x0006, 0x0000, 0x4646, 
            0x4754, 0x6961, 0x006E, 0x0000, 0x0000, 0x0000, 0x7000, 0x0042, 
            0x0000, 0x0000, 0x8000, 0xCD3F, 0xCCCC, 0x5C3D, 0x0700, 0x0000, 
            0x4700, 0x6961, 0x006E, 0x0000, 0x4000, 0x00C1, 0x4000, 0x0041, 
            0x4000, 0x0034, 0x8000, 0xCD3F, 0xCCCC, 0x003D
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
		// Node References -------------------------------------------------------------------------
		
		auto& split = this->getT(0);                        // cabDesignerNetwork_impl::split_t<NV>
		auto& chain = this->getT(0).getT(0);                // cabDesignerNetwork_impl::chain_t<NV>
		auto& clear = this->getT(0).getT(0).getT(0);        // math::clear<NV>
		auto& file_player = this->getT(0).getT(0).getT(1);  // cabDesignerNetwork_impl::file_player_t<NV>
		auto& gain = this->getT(0).getT(0).getT(2);         // core::gain<NV>
		auto& cabDesigner = this->getT(0).getT(0).getT(3);  // project::cabDesigner<NV>
		auto& svf_eq = this->getT(0).getT(0).getT(4);       // filters::svf_eq<NV>
		auto& fft = this->getT(0).getT(0).getT(5);          // cabDesignerNetwork_impl::fft_t
		auto& clear1 = this->getT(0).getT(0).getT(6);       // math::clear<NV>
		auto& chain1 = this->getT(0).getT(1);               // cabDesignerNetwork_impl::chain1_t<NV>
		auto& cabDesigner1 = this->getT(0).getT(1).getT(0); // project::cabDesigner<NV>
		auto& gain1 = this->getT(0).getT(1).getT(1);        // core::gain<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		auto& SpeakerType_p = this->getParameterT(0);
		SpeakerType_p.connectT(0, cabDesigner1); // SpeakerType -> cabDesigner1::SpeakerType
		SpeakerType_p.connectT(1, cabDesigner);  // SpeakerType -> cabDesigner::SpeakerType
		
		auto& CustomMod_p = this->getParameterT(1);
		CustomMod_p.connectT(0, cabDesigner1); // CustomMod -> cabDesigner1::CustomMod
		CustomMod_p.connectT(1, cabDesigner);  // CustomMod -> cabDesigner::CustomMod
		
		auto& MicrophoneType_p = this->getParameterT(2);
		MicrophoneType_p.connectT(0, cabDesigner1); // MicrophoneType -> cabDesigner1::MicrophoneType
		MicrophoneType_p.connectT(1, cabDesigner);  // MicrophoneType -> cabDesigner::MicrophoneType
		
		auto& MojoStrength_p = this->getParameterT(3);
		MojoStrength_p.connectT(0, cabDesigner1); // MojoStrength -> cabDesigner1::MojoStrength
		MojoStrength_p.connectT(1, cabDesigner);  // MojoStrength -> cabDesigner::MojoStrength
		
		auto& GenerateMojo_p = this->getParameterT(4);
		GenerateMojo_p.connectT(0, cabDesigner1); // GenerateMojo -> cabDesigner1::GenerateMojo
		GenerateMojo_p.connectT(1, cabDesigner);  // GenerateMojo -> cabDesigner::GenerateMojo
		
		auto& CabAge_p = this->getParameterT(5);
		CabAge_p.connectT(0, cabDesigner1); // CabAge -> cabDesigner1::CabAge
		CabAge_p.connectT(1, cabDesigner);  // CabAge -> cabDesigner::CabAge
		
		this->getParameterT(6).connectT(0, gain); // FFTGain -> gain::Gain
		
		this->getParameterT(7).connectT(0, gain1); // Gain -> gain1::Gain
		
		// Default Values --------------------------------------------------------------------------
		
		clear.setParameterT(0, 0.); // math::clear::Value
		
		file_player.setParameterT(0, 0.);   // core::file_player::PlaybackMode
		file_player.setParameterT(1, 1.);   // core::file_player::Gate
		file_player.setParameterT(2, 440.); // core::file_player::RootFrequency
		file_player.setParameterT(3, 1);    // core::file_player::FreqRatio
		
		;                           // gain::Gain is automated
		gain.setParameterT(1, 20.); // core::gain::Smoothing
		gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		; // cabDesigner::SpeakerType is automated
		; // cabDesigner::CustomMod is automated
		; // cabDesigner::MicrophoneType is automated
		; // cabDesigner::MojoStrength is automated
		; // cabDesigner::GenerateMojo is automated
		; // cabDesigner::CabAge is automated
		
		svf_eq.setParameterT(0, 166.533);  // filters::svf_eq::Frequency
		svf_eq.setParameterT(1, 0.689054); // filters::svf_eq::Q
		svf_eq.setParameterT(2, 0.);       // filters::svf_eq::Gain
		svf_eq.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq.setParameterT(4, 1.);       // filters::svf_eq::Mode
		svf_eq.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		clear1.setParameterT(0, 0.); // math::clear::Value
		
		; // cabDesigner1::SpeakerType is automated
		; // cabDesigner1::CustomMod is automated
		; // cabDesigner1::MicrophoneType is automated
		; // cabDesigner1::MojoStrength is automated
		; // cabDesigner1::GenerateMojo is automated
		; // cabDesigner1::CabAge is automated
		
		;                            // gain1::Gain is automated
		gain1.setParameterT(1, 20.); // core::gain::Smoothing
		gain1.setParameterT(2, 0.);  // core::gain::ResetValue
		
		this->setParameterT(0, 0.);
		this->setParameterT(1, 0.);
		this->setParameterT(2, 0.);
		this->setParameterT(3, 0.);
		this->setParameterT(4, 0.);
		this->setParameterT(5, 0.);
		this->setParameterT(6, 0.);
		this->setParameterT(7, 1.78814e-07);
		this->setExternalData({}, -1);
	}
	~instance() override
	{
		// Cleanup external data references --------------------------------------------------------
		
		this->setExternalData({}, -1);
	}
	
	static constexpr bool isPolyphonic() { return NV > 1; };
	
	static constexpr bool isProcessingHiseEvent() { return true; };
	
	static constexpr bool hasTail() { return true; };
	
	static constexpr bool isSuspendedOnSilence() { return false; };
	
	void setExternalData(const ExternalData& b, int index)
	{
		// External Data Connections ---------------------------------------------------------------
		
		this->getT(0).getT(0).getT(1).setExternalData(b, index); // cabDesignerNetwork_impl::file_player_t<NV>
		this->getT(0).getT(0).getT(5).setExternalData(b, index); // cabDesignerNetwork_impl::fft_t
	}
};
}

#undef getT
#undef connectT
#undef setParameterT
#undef setParameterWT
#undef getParameterT
// ======================================| Public Definition |======================================

namespace project
{
// polyphonic template declaration

template <int NV>
using cabDesignerNetwork = wrap::node<cabDesignerNetwork_impl::instance<NV>>;
}


