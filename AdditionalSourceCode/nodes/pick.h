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
using chain4_t = container::chain<parameter::empty, 
                                  wrap::fix<2, filters::svf_eq<NV>>>;

DECLARE_PARAMETER_RANGE(pma2_modRange, 
                        0., 
                        24.);

template <int NV>
using pma2_mod = parameter::from0To1<filters::svf_eq<NV>, 
                                     2, 
                                     pma2_modRange>;

template <int NV>
using pma2_t = control::pma<NV, pma2_mod<NV>>;
template <int NV>
using pma3_t = control::pma<NV, 
                            parameter::plain<pma2_t<NV>, 0>>;
template <int NV>
using envelope_follower1_t = wrap::mod<parameter::plain<pma3_t<NV>, 0>, 
                                       wrap::no_data<dynamics::envelope_follower<NV>>>;

template <int NV>
using chain5_t = container::chain<parameter::empty, 
                                  wrap::fix<2, filters::svf_eq<NV>>, 
                                  filters::svf_eq<NV>, 
                                  envelope_follower1_t<NV>, 
                                  pma3_t<NV>, 
                                  pma2_t<NV>, 
                                  math::clear<NV>>;

template <int NV>
using split_t = container::split<parameter::empty, 
                                 wrap::fix<2, chain4_t<NV>>, 
                                 chain5_t<NV>>;

namespace pick_t_parameters
{
}

template <int NV>
using pick_t_ = container::chain<parameter::plain<pick_impl::pma2_t<NV>, 1>, 
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
		SNEX_METADATA_ENCODED_PARAMETERS(20)
		{
			0x005C, 0x0000, 0x0000, 0x7453, 0x6572, 0x676E, 0x6874, 0x0000, 
            0x0000, 0x0000, 0x0000, 0x4120, 0x0000, 0x0000, 0x0000, 0x3F80, 
            0xCCCD, 0x3DCC, 0x0000, 0x0000
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
		
		auto& split = this->getT(0);                              // pick_impl::split_t<NV>
		auto& chain4 = this->getT(0).getT(0);                     // pick_impl::chain4_t<NV>
		auto& svf_eq = this->getT(0).getT(0).getT(0);             // filters::svf_eq<NV>
		auto& chain5 = this->getT(0).getT(1);                     // pick_impl::chain5_t<NV>
		auto& svf_eq1 = this->getT(0).getT(1).getT(0);            // filters::svf_eq<NV>
		auto& svf_eq2 = this->getT(0).getT(1).getT(1);            // filters::svf_eq<NV>
		auto& envelope_follower1 = this->getT(0).getT(1).getT(2); // pick_impl::envelope_follower1_t<NV>
		auto& pma3 = this->getT(0).getT(1).getT(3);               // pick_impl::pma3_t<NV>
		auto& pma2 = this->getT(0).getT(1).getT(4);               // pick_impl::pma2_t<NV>
		auto& clear = this->getT(0).getT(1).getT(5);              // math::clear<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, pma2); // Strength -> pma2::Multiply
		
		// Modulation Connections ------------------------------------------------------------------
		
		pma2.getWrappedObject().getParameter().connectT(0, svf_eq); // pma2 -> svf_eq::Gain
		pma3.getWrappedObject().getParameter().connectT(0, pma2);   // pma3 -> pma2::Value
		envelope_follower1.getParameter().connectT(0, pma3);        // envelope_follower1 -> pma3::Value
		
		// Default Values --------------------------------------------------------------------------
		
		svf_eq.setParameterT(0, 3344.3);   // filters::svf_eq::Frequency
		svf_eq.setParameterT(1, 0.541716); // filters::svf_eq::Q
		;                                  // svf_eq::Gain is automated
		svf_eq.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		svf_eq1.setParameterT(0, 2426.95); // filters::svf_eq::Frequency
		svf_eq1.setParameterT(1, 1.);      // filters::svf_eq::Q
		svf_eq1.setParameterT(2, 0.);      // filters::svf_eq::Gain
		svf_eq1.setParameterT(3, 0.01);    // filters::svf_eq::Smoothing
		svf_eq1.setParameterT(4, 1.);      // filters::svf_eq::Mode
		svf_eq1.setParameterT(5, 1.);      // filters::svf_eq::Enabled
		
		svf_eq2.setParameterT(0, 4694.13); // filters::svf_eq::Frequency
		svf_eq2.setParameterT(1, 1.);      // filters::svf_eq::Q
		svf_eq2.setParameterT(2, 0.);      // filters::svf_eq::Gain
		svf_eq2.setParameterT(3, 0.01);    // filters::svf_eq::Smoothing
		svf_eq2.setParameterT(4, 0.);      // filters::svf_eq::Mode
		svf_eq2.setParameterT(5, 1.);      // filters::svf_eq::Enabled
		
		envelope_follower1.setParameterT(0, 10.); // dynamics::envelope_follower::Attack
		envelope_follower1.setParameterT(1, 20.); // dynamics::envelope_follower::Release
		envelope_follower1.setParameterT(2, 0.);  // dynamics::envelope_follower::ProcessSignal
		
		;                            // pma3::Value is automated
		pma3.setParameterT(1, 100.); // control::pma::Multiply
		pma3.setParameterT(2, 0.);   // control::pma::Add
		
		;                          // pma2::Value is automated
		;                          // pma2::Multiply is automated
		pma2.setParameterT(2, 0.); // control::pma::Add
		
		clear.setParameterT(0, 0.); // math::clear::Value
		
		this->setParameterT(0, 0.);
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
		
		this->getT(0).getT(1).getT(2).setExternalData(b, index); // pick_impl::envelope_follower1_t<NV>
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


