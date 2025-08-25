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

namespace pick_impl
{
// ==============================| Node & Parameter type declarations |==============================

template <int NV>
using chain_t = container::chain<parameter::empty, 
                                 wrap::fix<2, filters::svf_eq<NV>>>;

template <int NV>
using pma_unscaled1_mod = parameter::chain<ranges::Identity, 
                                           parameter::plain<filters::svf<NV>, 0>, 
                                           parameter::plain<filters::svf<NV>, 0>>;

template <int NV>
using pma_unscaled1_t = control::pma_unscaled<NV, pma_unscaled1_mod<NV>>;

DECLARE_PARAMETER_RANGE(envelope_follower_modRange, 
                        0., 
                        12.);

template <int NV>
using envelope_follower_mod = parameter::from0To1<filters::svf_eq<NV>, 
                                                  2, 
                                                  envelope_follower_modRange>;

template <int NV>
using envelope_follower_t = wrap::mod<envelope_follower_mod<NV>, 
                                      wrap::no_data<dynamics::envelope_follower<NV>>>;

template <int NV>
using chain1_t = container::chain<parameter::empty, 
                                  wrap::fix<2, pma_unscaled1_t<NV>>, 
                                  control::pma_unscaled<NV, parameter::empty>, 
                                  filters::svf<NV>, 
                                  filters::svf<NV>, 
                                  core::gain<NV>, 
                                  envelope_follower_t<NV>, 
                                  math::clear<NV>>;

template <int NV>
using split_t = container::split<parameter::empty, 
                                 wrap::fix<2, chain_t<NV>>, 
                                 chain1_t<NV>>;

namespace pick_t_parameters
{
// Parameter list for pick_impl::pick_t ------------------------------------------------------------

template <int NV>
using Freq = parameter::chain<ranges::Identity, 
                              parameter::plain<filters::svf_eq<NV>, 0>, 
                              parameter::plain<pick_impl::pma_unscaled1_t<NV>, 0>, 
                              parameter::plain<control::pma_unscaled<NV, parameter::empty>, 0>>;

template <int NV>
using Threshold = parameter::plain<core::gain<NV>, 0>;
template <int NV>
using pick_t_plist = parameter::list<Threshold<NV>, Freq<NV>>;
}

template <int NV>
using pick_t_ = container::chain<pick_t_parameters::pick_t_plist<NV>, 
                                 wrap::fix<2, split_t<NV>>>;

// =================================| Root node initialiser class |=================================

template <int NV> struct instance: public pick_impl::pick_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(pick);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(34)
		{
			0x005C, 0x0000, 0x0000, 0x6854, 0x6572, 0x6873, 0x6C6F, 0x0064, 
            0x0000, 0x0000, 0x0000, 0x42C8, 0x0000, 0x42C8, 0x0000, 0x3F80, 
            0xCCCD, 0x3DCC, 0x015C, 0x0001, 0x0000, 0x7246, 0x7165, 0x0000, 
            0xFA00, 0x0044, 0x7A00, 0x0045, 0x28C0, 0x0045, 0x8000, 0x003F, 
            0x8000, 0x003F
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
		
		auto& split = this->getT(0);                             // pick_impl::split_t<NV>
		auto& chain = this->getT(0).getT(0);                     // pick_impl::chain_t<NV>
		auto& svf_eq1 = this->getT(0).getT(0).getT(0);           // filters::svf_eq<NV>
		auto& chain1 = this->getT(0).getT(1);                    // pick_impl::chain1_t<NV>
		auto& pma_unscaled1 = this->getT(0).getT(1).getT(0);     // pick_impl::pma_unscaled1_t<NV>
		auto& pma_unscaled = this->getT(0).getT(1).getT(1);      // control::pma_unscaled<NV, parameter::empty>
		auto& svf2 = this->getT(0).getT(1).getT(2);              // filters::svf<NV>
		auto& svf3 = this->getT(0).getT(1).getT(3);              // filters::svf<NV>
		auto& gain = this->getT(0).getT(1).getT(4);              // core::gain<NV>
		auto& envelope_follower = this->getT(0).getT(1).getT(5); // pick_impl::envelope_follower_t<NV>
		auto& clear1 = this->getT(0).getT(1).getT(6);            // math::clear<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, gain); // Threshold -> gain::Gain
		
		auto& Freq_p = this->getParameterT(1);
		Freq_p.connectT(0, svf_eq1);       // Freq -> svf_eq1::Frequency
		Freq_p.connectT(1, pma_unscaled1); // Freq -> pma_unscaled1::Value
		Freq_p.connectT(2, pma_unscaled);  // Freq -> pma_unscaled::Value
		
		// Modulation Connections ------------------------------------------------------------------
		
		pma_unscaled1.getWrappedObject().getParameter().connectT(0, svf2); // pma_unscaled1 -> svf2::Frequency
		pma_unscaled1.getWrappedObject().getParameter().connectT(1, svf3); // pma_unscaled1 -> svf3::Frequency
		envelope_follower.getParameter().connectT(0, svf_eq1);             // envelope_follower -> svf_eq1::Gain
		
		// Default Values --------------------------------------------------------------------------
		
		;                                   // svf_eq1::Frequency is automated
		svf_eq1.setParameterT(1, 0.881295); // filters::svf_eq::Q
		;                                   // svf_eq1::Gain is automated
		svf_eq1.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq1.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq1.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		;                                        // pma_unscaled1::Value is automated
		pma_unscaled1.setParameterT(1, 1.);      // control::pma_unscaled::Multiply
		pma_unscaled1.setParameterT(2, 143.733); // control::pma_unscaled::Add
		
		;                                  // pma_unscaled::Value is automated
		pma_unscaled.setParameterT(1, 1.); // control::pma_unscaled::Multiply
		pma_unscaled.setParameterT(2, 0.); // control::pma_unscaled::Add
		
		;                                // svf2::Frequency is automated
		svf2.setParameterT(1, 0.810861); // filters::svf::Q
		svf2.setParameterT(2, 0.);       // filters::svf::Gain
		svf2.setParameterT(3, 0.01);     // filters::svf::Smoothing
		svf2.setParameterT(4, 0.);       // filters::svf::Mode
		svf2.setParameterT(5, 1.);       // filters::svf::Enabled
		
		;                                // svf3::Frequency is automated
		svf3.setParameterT(1, 0.810861); // filters::svf::Q
		svf3.setParameterT(2, 0.);       // filters::svf::Gain
		svf3.setParameterT(3, 0.01);     // filters::svf::Smoothing
		svf3.setParameterT(4, 1.);       // filters::svf::Mode
		svf3.setParameterT(5, 1.);       // filters::svf::Enabled
		
		;                           // gain::Gain is automated
		gain.setParameterT(1, 20.); // core::gain::Smoothing
		gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		envelope_follower.setParameterT(0, 10.5); // dynamics::envelope_follower::Attack
		envelope_follower.setParameterT(1, 10.);  // dynamics::envelope_follower::Release
		envelope_follower.setParameterT(2, 0.);   // dynamics::envelope_follower::ProcessSignal
		
		clear1.setParameterT(0, 0.); // math::clear::Value
		
		this->setParameterT(0, 100.);
		this->setParameterT(1, 2700.);
		this->setExternalData({}, -1);
	}
	~instance() override
	{
		// Cleanup external data references --------------------------------------------------------
		
		this->setExternalData({}, -1);
	}
	
	static constexpr bool isPolyphonic() { return NV > 1; };
	
	static constexpr bool hasTail() { return true; };
	
	static constexpr bool isSuspendedOnSilence() { return false; };
	
	void setExternalData(const ExternalData& b, int index)
	{
		// External Data Connections ---------------------------------------------------------------
		
		this->getT(0).getT(1).getT(5).setExternalData(b, index); // pick_impl::envelope_follower_t<NV>
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
using pick = wrap::node<pick_impl::instance<NV>>;
}


