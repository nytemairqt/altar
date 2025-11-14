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
                                 filters::one_pole<NV>, 
                                 filters::one_pole<NV>, 
                                 filters::one_pole<NV>, 
                                 filters::one_pole<NV>, 
                                 filters::one_pole<NV>, 
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
using MixReady = parameter::chain<ranges::Identity, 
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
using DiracGate = parameter::empty;
template <int NV>
using cabDesignerNetwork_t_plist = parameter::list<SpeakerType<NV>, 
                                                   MixReady<NV>, 
                                                   MicrophoneType<NV>, 
                                                   MojoStrength<NV>, 
                                                   GenerateMojo<NV>, 
                                                   CabAge<NV>, 
                                                   FFTGain<NV>, 
                                                   Gain<NV>, 
                                                   DiracGate>;
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
		SNEX_METADATA_ENCODED_PARAMETERS(168)
		{
			0x005C, 0x0000, 0x0000, 0x7053, 0x6165, 0x656B, 0x5472, 0x7079, 
            0x0065, 0x0000, 0x0000, 0x0000, 0xA000, 0x0040, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x0000, 0x5C00, 0x0100, 0x0000, 0x4D00, 0x7869, 
            0x6552, 0x6461, 0x0079, 0x0000, 0x0000, 0x0000, 0x8000, 0x003F, 
            0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 0x5C00, 0x0200, 0x0000, 
            0x4D00, 0x6369, 0x6F72, 0x6870, 0x6E6F, 0x5465, 0x7079, 0x0065, 
            0x0000, 0x0000, 0x0000, 0xA000, 0x0040, 0x0000, 0x0000, 0x8000, 
            0x003F, 0x0000, 0x5C00, 0x0300, 0x0000, 0x4D00, 0x6A6F, 0x536F, 
            0x7274, 0x6E65, 0x7467, 0x0068, 0x0000, 0x0000, 0x0000, 0x8000, 
            0x003F, 0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 0x5C00, 0x0400, 
            0x0000, 0x4700, 0x6E65, 0x7265, 0x7461, 0x4D65, 0x6A6F, 0x006F, 
            0x0000, 0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 0x0000, 0x8000, 
            0x003F, 0x0000, 0x5C00, 0x0500, 0x0000, 0x4300, 0x6261, 0x6741, 
            0x0065, 0x0000, 0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x0000, 0x5C00, 0x0600, 0x0000, 0x4600, 0x5446, 
            0x6147, 0x6E69, 0x0000, 0x0000, 0x0000, 0x0000, 0x4270, 0x0000, 
            0x0000, 0x0000, 0x3F80, 0xCCCD, 0x3DCC, 0x005C, 0x0007, 0x0000, 
            0x6147, 0x6E69, 0x0000, 0x0000, 0xC140, 0x0000, 0x4140, 0x0000, 
            0x3440, 0x0000, 0x3F80, 0xCCCD, 0x3DCC, 0x005C, 0x0008, 0x0000, 
            0x6944, 0x6172, 0x4763, 0x7461, 0x0065, 0x0000, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 0x0000
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
		auto& cabDesigner2 = this->getT(0).getT(0).getT(3); // project::cabDesigner<NV>
		auto& one_pole = this->getT(0).getT(0).getT(4);     // filters::one_pole<NV>
		auto& one_pole1 = this->getT(0).getT(0).getT(5);    // filters::one_pole<NV>
		auto& one_pole2 = this->getT(0).getT(0).getT(6);    // filters::one_pole<NV>
		auto& one_pole3 = this->getT(0).getT(0).getT(7);    // filters::one_pole<NV>
		auto& one_pole4 = this->getT(0).getT(0).getT(8);    // filters::one_pole<NV>
		auto& fft = this->getT(0).getT(0).getT(9);          // cabDesignerNetwork_impl::fft_t
		auto& clear1 = this->getT(0).getT(0).getT(10);      // math::clear<NV>
		auto& chain1 = this->getT(0).getT(1);               // cabDesignerNetwork_impl::chain1_t<NV>
		auto& cabDesigner3 = this->getT(0).getT(1).getT(0); // project::cabDesigner<NV>
		auto& gain1 = this->getT(0).getT(1).getT(1);        // core::gain<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		auto& SpeakerType_p = this->getParameterT(0);
		SpeakerType_p.connectT(0, cabDesigner2); // SpeakerType -> cabDesigner2::SpeakerType
		SpeakerType_p.connectT(1, cabDesigner3); // SpeakerType -> cabDesigner3::SpeakerType
		
		auto& MixReady_p = this->getParameterT(1);
		MixReady_p.connectT(0, cabDesigner2); // MixReady -> cabDesigner2::MixReady
		MixReady_p.connectT(1, cabDesigner3); // MixReady -> cabDesigner3::MixReady
		
		auto& MicrophoneType_p = this->getParameterT(2);
		MicrophoneType_p.connectT(0, cabDesigner2); // MicrophoneType -> cabDesigner2::MicrophoneType
		MicrophoneType_p.connectT(1, cabDesigner3); // MicrophoneType -> cabDesigner3::MicrophoneType
		
		auto& MojoStrength_p = this->getParameterT(3);
		MojoStrength_p.connectT(0, cabDesigner2); // MojoStrength -> cabDesigner2::MojoStrength
		MojoStrength_p.connectT(1, cabDesigner3); // MojoStrength -> cabDesigner3::MojoStrength
		
		auto& GenerateMojo_p = this->getParameterT(4);
		GenerateMojo_p.connectT(0, cabDesigner2); // GenerateMojo -> cabDesigner2::GenerateMojo
		GenerateMojo_p.connectT(1, cabDesigner3); // GenerateMojo -> cabDesigner3::GenerateMojo
		
		auto& CabAge_p = this->getParameterT(5);
		CabAge_p.connectT(0, cabDesigner2); // CabAge -> cabDesigner2::CabAge
		CabAge_p.connectT(1, cabDesigner3); // CabAge -> cabDesigner3::CabAge
		
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
		
		; // cabDesigner2::SpeakerType is automated
		; // cabDesigner2::MixReady is automated
		; // cabDesigner2::MicrophoneType is automated
		; // cabDesigner2::MojoStrength is automated
		; // cabDesigner2::GenerateMojo is automated
		; // cabDesigner2::CabAge is automated
		
		one_pole.setParameterT(0, 50.);  // filters::one_pole::Frequency
		one_pole.setParameterT(1, 1.);   // filters::one_pole::Q
		one_pole.setParameterT(2, 0.);   // filters::one_pole::Gain
		one_pole.setParameterT(3, 0.01); // filters::one_pole::Smoothing
		one_pole.setParameterT(4, 1.);   // filters::one_pole::Mode
		one_pole.setParameterT(5, 1.);   // filters::one_pole::Enabled
		
		one_pole1.setParameterT(0, 50.);  // filters::one_pole::Frequency
		one_pole1.setParameterT(1, 1.);   // filters::one_pole::Q
		one_pole1.setParameterT(2, 0.);   // filters::one_pole::Gain
		one_pole1.setParameterT(3, 0.01); // filters::one_pole::Smoothing
		one_pole1.setParameterT(4, 1.);   // filters::one_pole::Mode
		one_pole1.setParameterT(5, 1.);   // filters::one_pole::Enabled
		
		one_pole2.setParameterT(0, 50.);  // filters::one_pole::Frequency
		one_pole2.setParameterT(1, 1.);   // filters::one_pole::Q
		one_pole2.setParameterT(2, 0.);   // filters::one_pole::Gain
		one_pole2.setParameterT(3, 0.01); // filters::one_pole::Smoothing
		one_pole2.setParameterT(4, 1.);   // filters::one_pole::Mode
		one_pole2.setParameterT(5, 1.);   // filters::one_pole::Enabled
		
		one_pole3.setParameterT(0, 50.);  // filters::one_pole::Frequency
		one_pole3.setParameterT(1, 1.);   // filters::one_pole::Q
		one_pole3.setParameterT(2, 0.);   // filters::one_pole::Gain
		one_pole3.setParameterT(3, 0.01); // filters::one_pole::Smoothing
		one_pole3.setParameterT(4, 1.);   // filters::one_pole::Mode
		one_pole3.setParameterT(5, 1.);   // filters::one_pole::Enabled
		
		one_pole4.setParameterT(0, 50.);  // filters::one_pole::Frequency
		one_pole4.setParameterT(1, 1.);   // filters::one_pole::Q
		one_pole4.setParameterT(2, 0.);   // filters::one_pole::Gain
		one_pole4.setParameterT(3, 0.01); // filters::one_pole::Smoothing
		one_pole4.setParameterT(4, 1.);   // filters::one_pole::Mode
		one_pole4.setParameterT(5, 1.);   // filters::one_pole::Enabled
		
		clear1.setParameterT(0, 0.); // math::clear::Value
		
		; // cabDesigner3::SpeakerType is automated
		; // cabDesigner3::MixReady is automated
		; // cabDesigner3::MicrophoneType is automated
		; // cabDesigner3::MojoStrength is automated
		; // cabDesigner3::GenerateMojo is automated
		; // cabDesigner3::CabAge is automated
		
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
		this->setParameterT(8, 0.);
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
		this->getT(0).getT(0).getT(9).setExternalData(b, index); // cabDesignerNetwork_impl::fft_t
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


