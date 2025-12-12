#pragma once

#include "transpose.h"
// These will improve the readability of the connection definition

#define getT(Idx) template get<Idx>()
#define connectT(Idx, target) template connect<Idx>(target)
#define getParameterT(Idx) template getParameter<Idx>()
#define setParameterT(Idx, value) template setParameter<Idx>(value)
#define setParameterWT(Idx, value) template setWrapParameter<Idx>(value)
using namespace scriptnode;
using namespace snex;
using namespace snex::Types;

namespace octave_impl
{
// ==============================| Node & Parameter type declarations |==============================

DECLARE_PARAMETER_RANGE_SKEW(xfader_c1Range, 
                             -100., 
                             0., 
                             5.42227);

template <int NV>
using xfader_c1 = parameter::from0To1<core::gain<NV>, 
                                      0, 
                                      xfader_c1Range>;

template <int NV>
using xfader_multimod = parameter::list<parameter::empty, xfader_c1<NV>>;

template <int NV>
using xfader_t = control::xfader<xfader_multimod<NV>, faders::linear>;

using chain_t = container::chain<parameter::empty, 
                                 wrap::fix<2, core::fix_delay>>;

template <int NV>
using chain1_t = container::chain<parameter::empty, 
                                  wrap::fix<2, project::transpose<NV>>, 
                                  filters::svf_eq<NV>, 
                                  core::gain<NV>>;

template <int NV>
using split_t = container::split<parameter::empty, 
                                 wrap::fix<2, chain_t>, 
                                 chain1_t<NV>>;

namespace octave_t_parameters
{
// Parameter list for octave_impl::octave_t --------------------------------------------------------

template <int NV>
using Octave = parameter::plain<octave_impl::xfader_t<NV>, 
                                0>;
template <int NV>
using BlockSamples = parameter::plain<project::transpose<NV>, 1>;
template <int NV>
using IntervalSamples = parameter::plain<project::transpose<NV>, 2>;
template <int NV>
using octave_t_plist = parameter::list<Octave<NV>, 
                                       BlockSamples<NV>, 
                                       IntervalSamples<NV>>;
}

template <int NV>
using octave_t_ = container::chain<octave_t_parameters::octave_t_plist<NV>, 
                                   wrap::fix<2, xfader_t<NV>>, 
                                   split_t<NV>>;

// =================================| Root node initialiser class |=================================

template <int NV> struct instance: public octave_impl::octave_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(octave);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(60)
		{
			0x005C, 0x0000, 0x0000, 0x634F, 0x6174, 0x6576, 0x0000, 0x0000, 
            0x0000, 0x0000, 0x3F80, 0x0000, 0x3F80, 0x0000, 0x3F80, 0x0000, 
            0x0000, 0x005C, 0x0001, 0x0000, 0x6C42, 0x636F, 0x536B, 0x6D61, 
            0x6C70, 0x7365, 0x0000, 0x0000, 0x4280, 0x0000, 0x4600, 0x0000, 
            0x4400, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 0x0002, 0x0000, 
            0x6E49, 0x6574, 0x7672, 0x6C61, 0x6153, 0x706D, 0x656C, 0x0073, 
            0x0000, 0x8000, 0x0042, 0x0000, 0x0046, 0x0000, 0x0044, 0x8000, 
            0x003F, 0x0000, 0x0000, 0x0000
		};
		SNEX_METADATA_ENCODED_MOD_INFO(25)
		{
			0x003A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
            0x0000
		};
	};
	
	instance()
	{
		// Node References -------------------------------------------------------------------------
		
		auto& xfader = this->getT(0);                    // octave_impl::xfader_t<NV>
		auto& split = this->getT(1);                     // octave_impl::split_t<NV>
		auto& chain = this->getT(1).getT(0);             // octave_impl::chain_t
		auto& fix_delay = this->getT(1).getT(0).getT(0); // core::fix_delay
		auto& chain1 = this->getT(1).getT(1);            // octave_impl::chain1_t<NV>
		auto& transpose = this->getT(1).getT(1).getT(0); // project::transpose<NV>
		auto& svf_eq = this->getT(1).getT(1).getT(1);    // filters::svf_eq<NV>
		auto& gain = this->getT(1).getT(1).getT(2);      // core::gain<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, xfader); // Octave -> xfader::Value
		
		this->getParameterT(1).connectT(0, transpose); // BlockSamples -> transpose::BlockSamples
		
		this->getParameterT(2).connectT(0, transpose); // IntervalSamples -> transpose::IntervalSamples
		
		// Modulation Connections ------------------------------------------------------------------
		
		auto& xfader_p = xfader.getWrappedObject().getParameter();
		xfader_p.getParameterT(1).connectT(0, gain); // xfader -> gain::Gain
		
		// Default Values --------------------------------------------------------------------------
		
		; // xfader::Value is automated
		
		fix_delay.setParameterT(0, 10.7); // core::fix_delay::DelayTime
		fix_delay.setParameterT(1, 512.); // core::fix_delay::FadeTime
		
		transpose.setParameterT(0, 0.5); // project::transpose::FreqRatio
		;                                // transpose::BlockSamples is automated
		;                                // transpose::IntervalSamples is automated
		
		svf_eq.setParameterT(0, 1024.75);  // filters::svf_eq::Frequency
		svf_eq.setParameterT(1, 0.519895); // filters::svf_eq::Q
		svf_eq.setParameterT(2, 0.);       // filters::svf_eq::Gain
		svf_eq.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq.setParameterT(4, 0.);       // filters::svf_eq::Mode
		svf_eq.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		;                           // gain::Gain is automated
		gain.setParameterT(1, 20.); // core::gain::Smoothing
		gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		this->setParameterT(0, 1.);
		this->setParameterT(1, 512.);
		this->setParameterT(2, 512.);
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
// ======================================| Public Definition |======================================

namespace project
{
// polyphonic template declaration

template <int NV>
using octave = wrap::node<octave_impl::instance<NV>>;
}


