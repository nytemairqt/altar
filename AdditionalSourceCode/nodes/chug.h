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

namespace chug_impl
{
// ==============================| Node & Parameter type declarations |==============================

template <int NV>
using chain_t = container::chain<parameter::empty, 
                                 wrap::fix<2, filters::svf_eq<NV>>>;

template <int NV>
using pma_unscaled1_t = control::pma_unscaled<NV, 
                                              parameter::plain<filters::svf<NV>, 0>>;

template <int NV> using pma_unscaled_t = pma_unscaled1_t<NV>;

DECLARE_PARAMETER_RANGE_INV(envelope_follower_modRange, 
                            -10., 
                            0.);

template <int NV>
using envelope_follower_mod = parameter::from0To1_inv<filters::svf_eq<NV>, 
                                                      2, 
                                                      envelope_follower_modRange>;

template <int NV>
using envelope_follower_t = wrap::mod<envelope_follower_mod<NV>, 
                                      wrap::no_data<dynamics::envelope_follower<NV>>>;

template <int NV>
using chain1_t = container::chain<parameter::empty, 
                                  wrap::fix<2, pma_unscaled1_t<NV>>, 
                                  pma_unscaled_t<NV>, 
                                  filters::svf<NV>, 
                                  filters::svf<NV>, 
                                  core::gain<NV>, 
                                  envelope_follower_t<NV>, 
                                  math::clear<NV>>;

template <int NV>
using split_t = container::split<parameter::empty, 
                                 wrap::fix<2, chain_t<NV>>, 
                                 chain1_t<NV>>;

namespace chug_t_parameters
{
// Parameter list for chug_impl::chug_t ------------------------------------------------------------

template <int NV>
using Freq = parameter::chain<ranges::Identity, 
                              parameter::plain<filters::svf_eq<NV>, 0>, 
                              parameter::plain<chug_impl::pma_unscaled_t<NV>, 0>, 
                              parameter::plain<chug_impl::pma_unscaled1_t<NV>, 0>>;

template <int NV>
using Threshold = parameter::plain<core::gain<NV>, 0>;
template <int NV>
using chug_t_plist = parameter::list<Freq<NV>, Threshold<NV>>;
}

template <int NV>
using chug_t_ = container::chain<chug_t_parameters::chug_t_plist<NV>, 
                                 wrap::fix<2, split_t<NV>>>;

// =================================| Root node initialiser class |=================================

template <int NV> struct instance: public chug_impl::chug_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(chug);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(34)
		{
			0x015C, 0x0000, 0x0000, 0x7246, 0x7165, 0x0000, 0xA000, 0x0041, 
            0xFA00, 0x0043, 0xC800, 0x1A42, 0x6B6C, 0x003E, 0x0000, 0x5C00, 
            0x0100, 0x0000, 0x5400, 0x7268, 0x7365, 0x6F68, 0x646C, 0x0000, 
            0x0000, 0x0000, 0xC800, 0x0042, 0x0000, 0x0000, 0x8000, 0xCD3F, 
            0xCCCC, 0x003D
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
		
		auto& split = this->getT(0);                             // chug_impl::split_t<NV>
		auto& chain = this->getT(0).getT(0);                     // chug_impl::chain_t<NV>
		auto& svf_eq = this->getT(0).getT(0).getT(0);            // filters::svf_eq<NV>
		auto& chain1 = this->getT(0).getT(1);                    // chug_impl::chain1_t<NV>
		auto& pma_unscaled1 = this->getT(0).getT(1).getT(0);     // chug_impl::pma_unscaled1_t<NV>
		auto& pma_unscaled = this->getT(0).getT(1).getT(1);      // chug_impl::pma_unscaled_t<NV>
		auto& svf = this->getT(0).getT(1).getT(2);               // filters::svf<NV>
		auto& svf1 = this->getT(0).getT(1).getT(3);              // filters::svf<NV>
		auto& gain = this->getT(0).getT(1).getT(4);              // core::gain<NV>
		auto& envelope_follower = this->getT(0).getT(1).getT(5); // chug_impl::envelope_follower_t<NV>
		auto& clear1 = this->getT(0).getT(1).getT(6);            // math::clear<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		auto& Freq_p = this->getParameterT(0);
		Freq_p.connectT(0, svf_eq);        // Freq -> svf_eq::Frequency
		Freq_p.connectT(1, pma_unscaled);  // Freq -> pma_unscaled::Value
		Freq_p.connectT(2, pma_unscaled1); // Freq -> pma_unscaled1::Value
		
		this->getParameterT(1).connectT(0, gain); // Threshold -> gain::Gain
		
		// Modulation Connections ------------------------------------------------------------------
		
		pma_unscaled1.getWrappedObject().getParameter().connectT(0, svf); // pma_unscaled1 -> svf::Frequency
		pma_unscaled.getWrappedObject().getParameter().connectT(0, svf1); // pma_unscaled -> svf1::Frequency
		envelope_follower.getParameter().connectT(0, svf_eq);             // envelope_follower -> svf_eq::Gain
		
		// Default Values --------------------------------------------------------------------------
		
		;                                  // svf_eq::Frequency is automated
		svf_eq.setParameterT(1, 0.985619); // filters::svf_eq::Q
		;                                  // svf_eq::Gain is automated
		svf_eq.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		;                                        // pma_unscaled1::Value is automated
		pma_unscaled1.setParameterT(1, 1.);      // control::pma_unscaled::Multiply
		pma_unscaled1.setParameterT(2, 143.733); // control::pma_unscaled::Add
		
		;                                  // pma_unscaled::Value is automated
		pma_unscaled.setParameterT(1, 1.); // control::pma_unscaled::Multiply
		pma_unscaled.setParameterT(2, 0.); // control::pma_unscaled::Add
		
		;                           // svf::Frequency is automated
		svf.setParameterT(1, 1.);   // filters::svf::Q
		svf.setParameterT(2, 0.);   // filters::svf::Gain
		svf.setParameterT(3, 0.01); // filters::svf::Smoothing
		svf.setParameterT(4, 0.);   // filters::svf::Mode
		svf.setParameterT(5, 1.);   // filters::svf::Enabled
		
		;                            // svf1::Frequency is automated
		svf1.setParameterT(1, 1.);   // filters::svf::Q
		svf1.setParameterT(2, 0.);   // filters::svf::Gain
		svf1.setParameterT(3, 0.01); // filters::svf::Smoothing
		svf1.setParameterT(4, 1.);   // filters::svf::Mode
		svf1.setParameterT(5, 1.);   // filters::svf::Enabled
		
		;                           // gain::Gain is automated
		gain.setParameterT(1, 20.); // core::gain::Smoothing
		gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		envelope_follower.setParameterT(0, 10.5); // dynamics::envelope_follower::Attack
		envelope_follower.setParameterT(1, 10.);  // dynamics::envelope_follower::Release
		envelope_follower.setParameterT(2, 0.);   // dynamics::envelope_follower::ProcessSignal
		
		clear1.setParameterT(0, 0.); // math::clear::Value
		
		this->setParameterT(0, 100.);
		this->setParameterT(1, 0.);
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
		
		this->getT(0).getT(1).getT(5).setExternalData(b, index); // chug_impl::envelope_follower_t<NV>
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
using chug = wrap::node<chug_impl::instance<NV>>;
}


