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

namespace reverb_impl
{
// =====================| Node & Parameter type declarations |=====================

DECLARE_PARAMETER_RANGE_SKEW(xfader_c0Range, 
                             -100., 
                             0., 
                             5.42227);

template <int NV>
using xfader_c0 = parameter::from0To1<core::gain<NV>, 
                                      0, 
                                      xfader_c0Range>;

template <int NV> using xfader_c1 = xfader_c0<NV>;

template <int NV>
using xfader_multimod = parameter::list<xfader_c0<NV>, xfader_c1<NV>>;

template <int NV>
using xfader_t = control::xfader<xfader_multimod<NV>, faders::linear>;

template <int NV>
using chain1_t = container::chain<parameter::empty, 
                                  wrap::fix<2, core::gain<NV>>>;

template <int NV>
using chain_t = container::chain<parameter::empty, 
                                 wrap::fix<2, project::reverb_faust<NV>>, 
                                 filters::svf_eq<NV>, 
                                 jdsp::jchorus, 
                                 core::gain<NV>>;

template <int NV>
using split_t = container::split<parameter::empty, 
                                 wrap::fix<2, chain1_t<NV>>, 
                                 chain_t<NV>>;

namespace reverb_t_parameters
{
// Parameter list for reverb_impl::reverb_t ---------------------------------------

DECLARE_PARAMETER_RANGE(BrightnessRange, 
                        -12., 
                        12.);

template <int NV>
using Brightness = parameter::from0To1<filters::svf_eq<NV>, 
                                       2, 
                                       BrightnessRange>;

DECLARE_PARAMETER_RANGE_STEP(FeedbackRange, 
                             0., 
                             0.95, 
                             0.01);

template <int NV>
using Feedback = parameter::from0To1<project::reverb_faust<NV>, 
                                     4, 
                                     FeedbackRange>;

template <int NV>
using Mix = parameter::plain<reverb_impl::xfader_t<NV>, 
                             0>;
template <int NV>
using reverb_t_plist = parameter::list<Mix<NV>, 
                                       Brightness<NV>, 
                                       Feedback<NV>>;
}

template <int NV>
using reverb_t_ = container::chain<reverb_t_parameters::reverb_t_plist<NV>, 
                                   wrap::fix<2, xfader_t<NV>>, 
                                   split_t<NV>>;

// =========================| Root node initialiser class |=========================

template <int NV> struct instance: public reverb_impl::reverb_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(reverb);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(52)
		{
			0x005C, 0x0000, 0x0000, 0x694D, 0x0078, 0x0000, 0x0000, 0x0000, 
            0x3F80, 0x0000, 0x3F00, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 
            0x0001, 0x0000, 0x7242, 0x6769, 0x7468, 0x656E, 0x7373, 0x0000, 
            0x0000, 0x0000, 0x8000, 0x543F, 0x6474, 0x003F, 0x8000, 0x003F, 
            0x0000, 0x5C00, 0x0200, 0x0000, 0x4600, 0x6565, 0x6264, 0x6361, 
            0x006B, 0x0000, 0x0000, 0x0000, 0x3F80, 0x9446, 0x3F76, 0x0000, 
            0x3F80, 0x0000, 0x0000, 0x0000
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
		// Node References --------------------------------------------------------
		
		auto& xfader = this->getT(0);                  // reverb_impl::xfader_t<NV>
		auto& split = this->getT(1);                   // reverb_impl::split_t<NV>
		auto& chain1 = this->getT(1).getT(0);          // reverb_impl::chain1_t<NV>
		auto& gain = this->getT(1).getT(0).getT(0);    // core::gain<NV>
		auto& chain = this->getT(1).getT(1);           // reverb_impl::chain_t<NV>
		auto& faust = this->getT(1).getT(1).getT(0);   // project::reverb_faust<NV>
		auto& svf_eq = this->getT(1).getT(1).getT(1);  // filters::svf_eq<NV>
		auto& jchorus = this->getT(1).getT(1).getT(2); // jdsp::jchorus
		auto& gain1 = this->getT(1).getT(1).getT(3);   // core::gain<NV>
		
		// Parameter Connections --------------------------------------------------
		
		this->getParameterT(0).connectT(0, xfader); // Mix -> xfader::Value
		
		this->getParameterT(1).connectT(0, svf_eq); // Brightness -> svf_eq::Gain
		
		this->getParameterT(2).connectT(0, faust); // Feedback -> faust::feedback
		
		// Modulation Connections -------------------------------------------------
		
		auto& xfader_p = xfader.getWrappedObject().getParameter();
		xfader_p.getParameterT(0).connectT(0, gain);  // xfader -> gain::Gain
		xfader_p.getParameterT(1).connectT(0, gain1); // xfader -> gain1::Gain
		
		// Default Values ---------------------------------------------------------
		
		; // xfader::Value is automated
		
		;                           // gain::Gain is automated
		gain.setParameterT(1, 20.); // core::gain::Smoothing
		gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		faust.setParameterT(0, 0.0957); // core::faust::delayTime
		faust.setParameterT(1, 0.436);  // core::faust::damping
		faust.setParameterT(2, 1.);     // core::faust::size
		faust.setParameterT(3, 0.5);    // core::faust::diffusion
		;                               // faust::feedback is automated
		faust.setParameterT(5, 0.1);    // core::faust::modDepth
		faust.setParameterT(6, 2.);     // core::faust::modFreq
		
		svf_eq.setParameterT(0, 3129.15);  // filters::svf_eq::Frequency
		svf_eq.setParameterT(1, 0.417865); // filters::svf_eq::Q
		;                                  // svf_eq::Gain is automated
		svf_eq.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq.setParameterT(4, 3.);       // filters::svf_eq::Mode
		svf_eq.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		jchorus.setParameterT(0, 7.);     // jdsp::jchorus::CentreDelay
		jchorus.setParameterT(1, 0.058);  // jdsp::jchorus::Depth
		jchorus.setParameterT(2, 0.);     // jdsp::jchorus::Feedback
		jchorus.setParameterT(3, 2.3181); // jdsp::jchorus::Rate
		jchorus.setParameterT(4, 0.112);  // jdsp::jchorus::Mix
		
		;                            // gain1::Gain is automated
		gain1.setParameterT(1, 20.); // core::gain::Smoothing
		gain1.setParameterT(2, 0.);  // core::gain::ResetValue
		
		this->setParameterT(0, 0.5);
		this->setParameterT(1, 0.8924);
		this->setParameterT(2, 0.9632);
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
// ==============================| Public Definition |==============================

namespace project
{
// polyphonic template declaration

template <int NV>
using reverb = wrap::node<reverb_impl::instance<NV>>;
}


