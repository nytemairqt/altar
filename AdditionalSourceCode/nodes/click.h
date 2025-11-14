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

namespace click_impl
{
// ==============================| Node & Parameter type declarations |==============================

template <int NV>
using oscillator_t = wrap::no_data<core::oscillator<NV>>;
template <int NV>
using converter_t = control::converter<parameter::plain<oscillator_t<NV>, 1>, 
                                       conversion_logic::ms2freq>;
template <int NV>
using tempo_sync1_t = wrap::mod<parameter::plain<converter_t<NV>, 0>, 
                                control::tempo_sync<NV>>;

struct file_player_t_data: public data::embedded::multichannel_data
{
	// Metadata functions for embedded data --------------------------------------------------------
	
	int    getNumSamples()     const override { return 132300; }
	double getSamplerate()     const override { return 44100.; }
	int    getNumChannels()    const override { return 2; }
	const float* getChannelData(int index) override
	{
		if(index == 0) { return reinterpret_cast<const float*>(audiodata::click_impl_file_player_t_data0); }
		if(index == 1) { return reinterpret_cast<const float*>(audiodata::click_impl_file_player_t_data1); }
		jassertfalse;    return nullptr;
	};
};

template <int NV>
using file_player_t = wrap::data<core::file_player<NV>, 
                                 data::embedded::audiofile<file_player_t_data>>;
template <int NV>
using ahdsr1_multimod = parameter::list<parameter::empty, 
                                        parameter::plain<file_player_t<NV>, 1>>;

template <int NV>
using ahdsr1_t = wrap::no_data<envelope::ahdsr<NV, ahdsr1_multimod<NV>>>;
template <int NV>
using peak_unscaled_t = wrap::mod<parameter::plain<ahdsr1_t<NV>, 8>, 
                                  wrap::no_data<core::peak_unscaled>>;

template <int NV>
using modchain_t_ = container::chain<parameter::empty, 
                                     wrap::fix<1, tempo_sync1_t<NV>>, 
                                     converter_t<NV>, 
                                     oscillator_t<NV>, 
                                     peak_unscaled_t<NV>>;

template <int NV>
using modchain_t = wrap::control_rate<modchain_t_<NV>>;

template <int NV>
using soft_bypass_t_ = container::chain<parameter::empty, 
                                        wrap::fix<2, math::clear<NV>>, 
                                        modchain_t<NV>, 
                                        ahdsr1_t<NV>, 
                                        file_player_t<NV>, 
                                        core::gain<NV>>;

template <int NV>
using soft_bypass_t = bypass::smoothed<20, soft_bypass_t_<NV>>;

using chain_t = container::chain<parameter::empty, 
                                 wrap::fix<2, core::empty>>;

template <int NV>
using split_t = container::split<parameter::empty, 
                                 wrap::fix<2, soft_bypass_t<NV>>, 
                                 chain_t>;

namespace click_t_parameters
{
}

template <int NV>
using click_t_ = container::chain<parameter::plain<core::gain<NV>, 0>, 
                                  wrap::fix<2, split_t<NV>>>;

// =================================| Root node initialiser class |=================================

template <int NV> struct instance: public click_impl::click_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(click);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(18)
		{
			0x005C, 0x0000, 0x0000, 0x6147, 0x6E69, 0x0000, 0x0000, 0xC2C8, 
            0x0000, 0x4140, 0x0000, 0x35C8, 0x833E, 0x40AD, 0xCCCD, 0x3DCC, 
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
		// Node References -------------------------------------------------------------------------
		
		auto& split = this->getT(0);                                 // click_impl::split_t<NV>
		auto& soft_bypass = this->getT(0).getT(0);                   // click_impl::soft_bypass_t<NV>
		auto& clear = this->getT(0).getT(0).getT(0);                 // math::clear<NV>
		auto& modchain = this->getT(0).getT(0).getT(1);              // click_impl::modchain_t<NV>
		auto& tempo_sync1 = this->getT(0).getT(0).getT(1).getT(0);   // click_impl::tempo_sync1_t<NV>
		auto& converter = this->getT(0).getT(0).getT(1).getT(1);     // click_impl::converter_t<NV>
		auto& oscillator = this->getT(0).getT(0).getT(1).getT(2);    // click_impl::oscillator_t<NV>
		auto& peak_unscaled = this->getT(0).getT(0).getT(1).getT(3); // click_impl::peak_unscaled_t<NV>
		auto& ahdsr1 = this->getT(0).getT(0).getT(2);                // click_impl::ahdsr1_t<NV>
		auto& file_player = this->getT(0).getT(0).getT(3);           // click_impl::file_player_t<NV>
		auto& gain = this->getT(0).getT(0).getT(4);                  // core::gain<NV>
		auto& chain = this->getT(0).getT(1);                         // click_impl::chain_t
		
		// Parameter Connections -------------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, gain); // Gain -> gain::Gain
		
		// Modulation Connections ------------------------------------------------------------------
		
		converter.getWrappedObject().getParameter().connectT(0, oscillator); // converter -> oscillator::Frequency
		tempo_sync1.getParameter().connectT(0, converter);                   // tempo_sync1 -> converter::Value
		auto& ahdsr1_p = ahdsr1.getWrappedObject().getParameter();
		ahdsr1_p.getParameterT(1).connectT(0, file_player); // ahdsr1 -> file_player::Gate
		peak_unscaled.getParameter().connectT(0, ahdsr1);   // peak_unscaled -> ahdsr1::Gate
		
		// Default Values --------------------------------------------------------------------------
		
		clear.setParameterT(0, 0.); // math::clear::Value
		
		tempo_sync1.setParameterT(0, 5.);   // control::tempo_sync::Tempo
		tempo_sync1.setParameterT(1, 1.);   // control::tempo_sync::Multiplier
		tempo_sync1.setParameterT(2, 1.);   // control::tempo_sync::Enabled
		tempo_sync1.setParameterT(3, 200.); // control::tempo_sync::UnsyncedTime
		
		; // converter::Value is automated
		
		oscillator.setParameterT(0, 3.); // core::oscillator::Mode
		;                                // oscillator::Frequency is automated
		oscillator.setParameterT(2, 1.); // core::oscillator::FreqRatio
		oscillator.setParameterT(3, 1.); // core::oscillator::Gate
		oscillator.setParameterT(4, 0.); // core::oscillator::Phase
		oscillator.setParameterT(5, 1.); // core::oscillator::Gain
		
		ahdsr1.setParameterT(0, 3.4);  // envelope::ahdsr::Attack
		ahdsr1.setParameterT(1, 1.);   // envelope::ahdsr::AttackLevel
		ahdsr1.setParameterT(2, 0.4);  // envelope::ahdsr::Hold
		ahdsr1.setParameterT(3, 19.7); // envelope::ahdsr::Decay
		ahdsr1.setParameterT(4, 0.);   // envelope::ahdsr::Sustain
		ahdsr1.setParameterT(5, 0.);   // envelope::ahdsr::Release
		ahdsr1.setParameterT(6, 0.5);  // envelope::ahdsr::AttackCurve
		ahdsr1.setParameterT(7, 0.);   // envelope::ahdsr::Retrigger
		;                              // ahdsr1::Gate is automated
		
		file_player.setParameterT(0, 0.);   // core::file_player::PlaybackMode
		;                                   // file_player::Gate is automated
		file_player.setParameterT(2, 440.); // core::file_player::RootFrequency
		file_player.setParameterT(3, 1.);   // core::file_player::FreqRatio
		
		;                           // gain::Gain is automated
		gain.setParameterT(1, 20.); // core::gain::Smoothing
		gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		this->setParameterT(0, 1.49012e-06);
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
		
		this->getT(0).getT(0).getT(1).getT(2).setExternalData(b, index); // click_impl::oscillator_t<NV>
		this->getT(0).getT(0).getT(1).getT(3).setExternalData(b, index); // click_impl::peak_unscaled_t<NV>
		this->getT(0).getT(0).getT(2).setExternalData(b, index);         // click_impl::ahdsr1_t<NV>
		this->getT(0).getT(0).getT(3).setExternalData(b, index);         // click_impl::file_player_t<NV>
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
using click = wrap::node<click_impl::instance<NV>>;
}


