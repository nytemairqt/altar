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

namespace cab_impl
{
// ============================| Node & Parameter type declarations |============================

DECLARE_PARAMETER_RANGE_SKEW(xfader_c0Range, 
                             -100., 
                             0., 
                             11.2023);

template <int NV>
using xfader_c0 = parameter::from0To1<core::gain<NV>, 
                                      0, 
                                      xfader_c0Range>;

template <int NV> using xfader_c1 = xfader_c0<NV>;

template <int NV>
using xfader_multimod = parameter::list<xfader_c0<NV>, xfader_c1<NV>>;

template <int NV>
using xfader_t = control::xfader<xfader_multimod<NV>, faders::linear>;
using convolution2_t = wrap::data<filters::convolution, 
                                  data::external::audiofile<0>>;

template <int NV>
using soft_bypass1_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, math::inv<NV>>>;

template <int NV>
using soft_bypass1_t = bypass::smoothed<20, soft_bypass1_t_<NV>>;

template <int NV>
using soft_bypass_t_ = container::chain<parameter::empty, 
                                        wrap::fix<2, core::fix_delay>, 
                                        convolution2_t, 
                                        filters::svf_eq<NV>, 
                                        filters::svf_eq<NV>, 
                                        filters::svf_eq<NV>, 
                                        filters::svf_eq<NV>, 
                                        filters::svf_eq<NV>, 
                                        filters::svf_eq<NV>, 
                                        jdsp::jpanner<NV>, 
                                        soft_bypass1_t<NV>, 
                                        core::gain<NV>, 
                                        core::gain<NV>>;

template <int NV>
using soft_bypass_t = bypass::smoothed<20, soft_bypass_t_<NV>>;
using convolution1_t = wrap::data<filters::convolution, 
                                  data::external::audiofile<1>>;

template <int NV> using soft_bypass3_t_ = soft_bypass1_t_<NV>;

template <int NV>
using soft_bypass3_t = bypass::smoothed<20, soft_bypass3_t_<NV>>;

template <int NV>
using soft_bypass2_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, core::fix_delay>, 
                                         convolution1_t, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         jdsp::jpanner<NV>, 
                                         soft_bypass3_t<NV>, 
                                         core::gain<NV>, 
                                         core::gain<NV>>;

template <int NV>
using soft_bypass2_t = bypass::smoothed<20, soft_bypass2_t_<NV>>;

template <int NV>
using split_t = container::split<parameter::empty, 
                                 wrap::fix<2, soft_bypass_t<NV>>, 
                                 soft_bypass2_t<NV>>;

namespace cab_t_parameters
{
// Parameter list for cab_impl::cab_t -----------------------------------------------------------

template <int NV>
using CabAEnable = parameter::bypass<cab_impl::soft_bypass_t<NV>>;

DECLARE_PARAMETER_RANGE_SKEW(CabADelay_InputRange, 
                             0., 
                             50., 
                             0.30103);
DECLARE_PARAMETER_RANGE_SKEW(CabADelay_0Range, 
                             0., 
                             50., 
                             0.30103);

using CabADelay_0 = parameter::from0To1<core::fix_delay, 
                                        0, 
                                        CabADelay_0Range>;

using CabADelay = parameter::chain<CabADelay_InputRange, CabADelay_0>;

DECLARE_PARAMETER_RANGE_INV(CabAAxis_0Range, 
                            0., 
                            6.);

template <int NV>
using CabAAxis_0 = parameter::from0To1_inv<filters::svf_eq<NV>, 
                                           2, 
                                           CabAAxis_0Range>;

DECLARE_PARAMETER_RANGE(CabAAxis_1Range, 
                        -6., 
                        0.);

template <int NV>
using CabAAxis_1 = parameter::from0To1<filters::svf_eq<NV>, 
                                       2, 
                                       CabAAxis_1Range>;

template <int NV>
using CabAAxis = parameter::chain<ranges::Identity, 
                                  CabAAxis_0<NV>, 
                                  CabAAxis_1<NV>>;

DECLARE_PARAMETER_RANGE(CabAAngle_0Range, 
                        0., 
                        3.);

template <int NV>
using CabAAngle_0 = parameter::from0To1<filters::svf_eq<NV>, 
                                        2, 
                                        CabAAngle_0Range>;

DECLARE_PARAMETER_RANGE(CabAAngle_1Range, 
                        0., 
                        2.);

template <int NV>
using CabAAngle_1 = parameter::from0To1<filters::svf_eq<NV>, 
                                        2, 
                                        CabAAngle_1Range>;

template <int NV>
using CabAAngle = parameter::chain<ranges::Identity, 
                                   CabAAngle_0<NV>, 
                                   CabAAngle_1<NV>>;

template <int NV> using CabADistance_0 = CabAAngle_0<NV>;

DECLARE_PARAMETER_RANGE_INV(CabADistance_1Range, 
                            -4., 
                            0.);

template <int NV>
using CabADistance_1 = parameter::from0To1_inv<filters::svf_eq<NV>, 
                                               2, 
                                               CabADistance_1Range>;

template <int NV>
using CabADistance = parameter::chain<ranges::Identity, 
                                      CabADistance_0<NV>, 
                                      CabADistance_1<NV>>;

template <int NV>
using CabAPhase = parameter::bypass<cab_impl::soft_bypass1_t<NV>>;

template <int NV>
using CabBEnable = parameter::bypass<cab_impl::soft_bypass2_t<NV>>;

DECLARE_PARAMETER_RANGE_SKEW(CabBDelay_InputRange, 
                             0., 
                             50., 
                             0.30103);
using CabBDelay_0 = CabADelay_0;

using CabBDelay = parameter::chain<CabBDelay_InputRange, CabBDelay_0>;

template <int NV> using CabBAxis_0 = CabAAxis_0<NV>;

template <int NV> using CabBAxis_1 = CabAAxis_1<NV>;

template <int NV>
using CabBAxis = parameter::chain<ranges::Identity, 
                                  CabBAxis_0<NV>, 
                                  CabBAxis_1<NV>>;

template <int NV> using CabBAngle_0 = CabAAngle_0<NV>;

template <int NV> using CabBAngle_1 = CabAAngle_1<NV>;

template <int NV>
using CabBAngle = parameter::chain<ranges::Identity, 
                                   CabBAngle_0<NV>, 
                                   CabBAngle_1<NV>>;

template <int NV> using CabBDistance_0 = CabAAngle_0<NV>;

template <int NV> using CabBDistance_1 = CabADistance_1<NV>;

template <int NV>
using CabBDistance = parameter::chain<ranges::Identity, 
                                      CabBDistance_0<NV>, 
                                      CabBDistance_1<NV>>;

template <int NV>
using CabBPhase = parameter::bypass<cab_impl::soft_bypass3_t<NV>>;

template <int NV>
using Mix = parameter::plain<cab_impl::xfader_t<NV>, 0>;
template <int NV>
using CabAPan = parameter::plain<jdsp::jpanner<NV>, 0>;
template <int NV>
using CabAGain = parameter::plain<core::gain<NV>, 0>;
template <int NV> using CabBPan = CabAPan<NV>;
template <int NV> using CabBGain = CabAGain<NV>;
template <int NV>
using cab_t_plist = parameter::list<Mix<NV>, 
                                    CabAEnable<NV>, 
                                    CabADelay, 
                                    CabAAxis<NV>, 
                                    CabAAngle<NV>, 
                                    CabADistance<NV>, 
                                    CabAPan<NV>, 
                                    CabAPhase<NV>, 
                                    CabAGain<NV>, 
                                    CabBEnable<NV>, 
                                    CabBDelay, 
                                    CabBAxis<NV>, 
                                    CabBAngle<NV>, 
                                    CabBDistance<NV>, 
                                    CabBPan<NV>, 
                                    CabBPhase<NV>, 
                                    CabBGain<NV>>;
}

template <int NV>
using cab_t_ = container::chain<cab_t_parameters::cab_t_plist<NV>, 
                                wrap::fix<2, xfader_t<NV>>, 
                                split_t<NV>>;

// ================================| Root node initialiser class |================================

template <int NV> struct instance: public cab_impl::cab_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 2;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(cab);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(304)
		{
			0x005C, 0x0000, 0x0000, 0x694D, 0x0078, 0x0000, 0x0000, 0x0000, 
            0x3F80, 0x0000, 0x3F00, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 
            0x0001, 0x0000, 0x6143, 0x4162, 0x6E45, 0x6261, 0x656C, 0x0000, 
            0x0000, 0x0000, 0x8000, 0x003F, 0x8000, 0x003F, 0x8000, 0x003F, 
            0x8000, 0x5C3F, 0x0200, 0x0000, 0x4300, 0x6261, 0x4441, 0x6C65, 
            0x7961, 0x0000, 0x0000, 0x0000, 0x4800, 0x0042, 0x0000, 0x9B00, 
            0x9A20, 0x003E, 0x0000, 0x5C00, 0x0300, 0x0000, 0x4300, 0x6261, 
            0x4141, 0x6978, 0x0073, 0x0000, 0x0000, 0x0000, 0x3F80, 0xB852, 
            0x3E9E, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 0x0004, 0x0000, 
            0x6143, 0x4162, 0x6E41, 0x6C67, 0x0065, 0x0000, 0x0000, 0x0000, 
            0x3F80, 0x0000, 0x3F00, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 
            0x0005, 0x0000, 0x6143, 0x4162, 0x6944, 0x7473, 0x6E61, 0x6563, 
            0x0000, 0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 0x0000, 0x8000, 
            0x003F, 0x0000, 0x5C00, 0x0600, 0x0000, 0x4300, 0x6261, 0x5041, 
            0x6E61, 0x0000, 0x8000, 0x00BF, 0x8000, 0x003F, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x0000, 0x5C00, 0x0700, 0x0000, 0x4300, 0x6261, 
            0x5041, 0x6168, 0x6573, 0x0000, 0x0000, 0x0000, 0x8000, 0x003F, 
            0x0000, 0x0000, 0x8000, 0x003F, 0x8000, 0x5C3F, 0x0800, 0x0000, 
            0x4300, 0x6261, 0x4741, 0x6961, 0x006E, 0x0000, 0xC2C8, 0x0000, 
            0x0000, 0x0000, 0x0000, 0x833E, 0x40AD, 0xCCCD, 0x3DCC, 0x005C, 
            0x0009, 0x0000, 0x6143, 0x4262, 0x6E45, 0x6261, 0x656C, 0x0000, 
            0x0000, 0x0000, 0x8000, 0x003F, 0x8000, 0x003F, 0x8000, 0x003F, 
            0x8000, 0x5C3F, 0x0A00, 0x0000, 0x4300, 0x6261, 0x4442, 0x6C65, 
            0x7961, 0x0000, 0x0000, 0x0000, 0x4800, 0x0042, 0x0000, 0x9B00, 
            0x9A20, 0x003E, 0x0000, 0x5C00, 0x0B00, 0x0000, 0x4300, 0x6261, 
            0x4142, 0x6978, 0x0073, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 
            0x3F80, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 0x000C, 0x0000, 
            0x6143, 0x4262, 0x6E41, 0x6C67, 0x0065, 0x0000, 0x0000, 0x0000, 
            0x3F80, 0x3333, 0x3F73, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 
            0x000D, 0x0000, 0x6143, 0x4262, 0x6944, 0x7473, 0x6E61, 0x6563, 
            0x0000, 0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 0x0000, 0x8000, 
            0x003F, 0x0000, 0x5C00, 0x0E00, 0x0000, 0x4300, 0x6261, 0x5042, 
            0x6E61, 0x0000, 0x8000, 0x00BF, 0x8000, 0x003F, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x0000, 0x5C00, 0x0F00, 0x0000, 0x4300, 0x6261, 
            0x5042, 0x6168, 0x6573, 0x0000, 0x0000, 0x0000, 0x8000, 0x003F, 
            0x0000, 0x0000, 0x8000, 0x003F, 0x8000, 0x5C3F, 0x1000, 0x0000, 
            0x4300, 0x6261, 0x4742, 0x6961, 0x006E, 0x0000, 0xC2C8, 0x0000, 
            0x0000, 0x0000, 0x0000, 0x833E, 0x40AD, 0xCCCD, 0x3DCC, 0x0000
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
		// Node References ----------------------------------------------------------------------
		
		auto& xfader = this->getT(0);                       // cab_impl::xfader_t<NV>
		auto& split = this->getT(1);                        // cab_impl::split_t<NV>
		auto& soft_bypass = this->getT(1).getT(0);          // cab_impl::soft_bypass_t<NV>
		auto& fix_delay = this->getT(1).getT(0).getT(0);    // core::fix_delay
		auto& convolution2 = this->getT(1).getT(0).getT(1); // cab_impl::convolution2_t
		auto& svf_eq = this->getT(1).getT(0).getT(2);       // filters::svf_eq<NV>
		auto& svf_eq1 = this->getT(1).getT(0).getT(3);      // filters::svf_eq<NV>
		auto& svf_eq2 = this->getT(1).getT(0).getT(4);      // filters::svf_eq<NV>
		auto& svf_eq3 = this->getT(1).getT(0).getT(5);      // filters::svf_eq<NV>
		auto& svf_eq4 = this->getT(1).getT(0).getT(6);      // filters::svf_eq<NV>
		auto& svf_eq6 = this->getT(1).getT(0).getT(7);      // filters::svf_eq<NV>
		auto& jpanner = this->getT(1).getT(0).getT(8);      // jdsp::jpanner<NV>
		auto& soft_bypass1 = this->getT(1).getT(0).getT(9); // cab_impl::soft_bypass1_t<NV>
		auto& inv = this->getT(1).getT(0).getT(9).getT(0);  // math::inv<NV>
		auto& gain = this->getT(1).getT(0).getT(10);        // core::gain<NV>
		auto& gain7 = this->getT(1).getT(0).getT(11);       // core::gain<NV>
		auto& soft_bypass2 = this->getT(1).getT(1);         // cab_impl::soft_bypass2_t<NV>
		auto& fix_delay1 = this->getT(1).getT(1).getT(0);   // core::fix_delay
		auto& convolution1 = this->getT(1).getT(1).getT(1); // cab_impl::convolution1_t
		auto& svf_eq7 = this->getT(1).getT(1).getT(2);      // filters::svf_eq<NV>
		auto& svf_eq8 = this->getT(1).getT(1).getT(3);      // filters::svf_eq<NV>
		auto& svf_eq9 = this->getT(1).getT(1).getT(4);      // filters::svf_eq<NV>
		auto& svf_eq10 = this->getT(1).getT(1).getT(5);     // filters::svf_eq<NV>
		auto& svf_eq11 = this->getT(1).getT(1).getT(6);     // filters::svf_eq<NV>
		auto& svf_eq12 = this->getT(1).getT(1).getT(7);     // filters::svf_eq<NV>
		auto& jpanner2 = this->getT(1).getT(1).getT(8);     // jdsp::jpanner<NV>
		auto& soft_bypass3 = this->getT(1).getT(1).getT(9); // cab_impl::soft_bypass3_t<NV>
		auto& inv1 = this->getT(1).getT(1).getT(9).getT(0); // math::inv<NV>
		auto& gain1 = this->getT(1).getT(1).getT(10);       // core::gain<NV>
		auto& gain8 = this->getT(1).getT(1).getT(11);       // core::gain<NV>
		
		// Parameter Connections ----------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, xfader); // Mix -> xfader::Value
		
		this->getParameterT(1).connectT(0, soft_bypass); // CabAEnable -> soft_bypass::Bypassed
		
		this->getParameterT(2).connectT(0, fix_delay); // CabADelay -> fix_delay::DelayTime
		
		auto& CabAAxis_p = this->getParameterT(3);
		CabAAxis_p.connectT(0, svf_eq);  // CabAAxis -> svf_eq::Gain
		CabAAxis_p.connectT(1, svf_eq1); // CabAAxis -> svf_eq1::Gain
		
		auto& CabAAngle_p = this->getParameterT(4);
		CabAAngle_p.connectT(0, svf_eq2); // CabAAngle -> svf_eq2::Gain
		CabAAngle_p.connectT(1, svf_eq3); // CabAAngle -> svf_eq3::Gain
		
		auto& CabADistance_p = this->getParameterT(5);
		CabADistance_p.connectT(0, svf_eq4); // CabADistance -> svf_eq4::Gain
		CabADistance_p.connectT(1, svf_eq6); // CabADistance -> svf_eq6::Gain
		
		this->getParameterT(6).connectT(0, jpanner); // CabAPan -> jpanner::Pan
		
		this->getParameterT(7).connectT(0, soft_bypass1); // CabAPhase -> soft_bypass1::Bypassed
		
		this->getParameterT(8).connectT(0, gain7); // CabAGain -> gain7::Gain
		
		this->getParameterT(9).connectT(0, soft_bypass2); // CabBEnable -> soft_bypass2::Bypassed
		
		this->getParameterT(10).connectT(0, fix_delay1); // CabBDelay -> fix_delay1::DelayTime
		
		auto& CabBAxis_p = this->getParameterT(11);
		CabBAxis_p.connectT(0, svf_eq7); // CabBAxis -> svf_eq7::Gain
		CabBAxis_p.connectT(1, svf_eq8); // CabBAxis -> svf_eq8::Gain
		
		auto& CabBAngle_p = this->getParameterT(12);
		CabBAngle_p.connectT(0, svf_eq9);  // CabBAngle -> svf_eq9::Gain
		CabBAngle_p.connectT(1, svf_eq10); // CabBAngle -> svf_eq10::Gain
		
		auto& CabBDistance_p = this->getParameterT(13);
		CabBDistance_p.connectT(0, svf_eq11); // CabBDistance -> svf_eq11::Gain
		CabBDistance_p.connectT(1, svf_eq12); // CabBDistance -> svf_eq12::Gain
		
		this->getParameterT(14).connectT(0, jpanner2); // CabBPan -> jpanner2::Pan
		
		this->getParameterT(15).connectT(0, soft_bypass3); // CabBPhase -> soft_bypass3::Bypassed
		
		this->getParameterT(16).connectT(0, gain8); // CabBGain -> gain8::Gain
		
		// Modulation Connections ---------------------------------------------------------------
		
		auto& xfader_p = xfader.getWrappedObject().getParameter();
		xfader_p.getParameterT(0).connectT(0, gain);  // xfader -> gain::Gain
		xfader_p.getParameterT(1).connectT(0, gain1); // xfader -> gain1::Gain
		
		// Default Values -----------------------------------------------------------------------
		
		; // xfader::Value is automated
		
		;                                 // fix_delay::DelayTime is automated
		fix_delay.setParameterT(1, 256.); // core::fix_delay::FadeTime
		
		convolution2.setParameterT(0, 1.);     // filters::convolution::Gate
		convolution2.setParameterT(1, 0.);     // filters::convolution::Predelay
		convolution2.setParameterT(2, 0.);     // filters::convolution::Damping
		convolution2.setParameterT(3, 20000.); // filters::convolution::HiCut
		convolution2.setParameterT(4, 0.);     // filters::convolution::Multithread
		
		svf_eq.setParameterT(0, 150.); // filters::svf_eq::Frequency
		svf_eq.setParameterT(1, 0.3);  // filters::svf_eq::Q
		;                              // svf_eq::Gain is automated
		svf_eq.setParameterT(3, 0.01); // filters::svf_eq::Smoothing
		svf_eq.setParameterT(4, 2.);   // filters::svf_eq::Mode
		svf_eq.setParameterT(5, 1.);   // filters::svf_eq::Enabled
		
		svf_eq1.setParameterT(0, 2300.); // filters::svf_eq::Frequency
		svf_eq1.setParameterT(1, 0.3);   // filters::svf_eq::Q
		;                                // svf_eq1::Gain is automated
		svf_eq1.setParameterT(3, 0.01);  // filters::svf_eq::Smoothing
		svf_eq1.setParameterT(4, 3.);    // filters::svf_eq::Mode
		svf_eq1.setParameterT(5, 1.);    // filters::svf_eq::Enabled
		
		svf_eq2.setParameterT(0, 2003.68);  // filters::svf_eq::Frequency
		svf_eq2.setParameterT(1, 0.624712); // filters::svf_eq::Q
		;                                   // svf_eq2::Gain is automated
		svf_eq2.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq2.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq2.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		svf_eq3.setParameterT(0, 4048.25);  // filters::svf_eq::Frequency
		svf_eq3.setParameterT(1, 0.349516); // filters::svf_eq::Q
		;                                   // svf_eq3::Gain is automated
		svf_eq3.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq3.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq3.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		svf_eq4.setParameterT(0, 1000.);    // filters::svf_eq::Frequency
		svf_eq4.setParameterT(1, 0.417865); // filters::svf_eq::Q
		;                                   // svf_eq4::Gain is automated
		svf_eq4.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq4.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq4.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		svf_eq6.setParameterT(0, 4000.);    // filters::svf_eq::Frequency
		svf_eq6.setParameterT(1, 0.548232); // filters::svf_eq::Q
		;                                   // svf_eq6::Gain is automated
		svf_eq6.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq6.setParameterT(4, 3.);       // filters::svf_eq::Mode
		svf_eq6.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		;                             // jpanner::Pan is automated
		jpanner.setParameterT(1, 1.); // jdsp::jpanner::Rule
		
		inv.setParameterT(0, 0.); // math::inv::Value
		
		;                           // gain::Gain is automated
		gain.setParameterT(1, 20.); // core::gain::Smoothing
		gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		;                            // gain7::Gain is automated
		gain7.setParameterT(1, 20.); // core::gain::Smoothing
		gain7.setParameterT(2, 0.);  // core::gain::ResetValue
		
		;                                  // fix_delay1::DelayTime is automated
		fix_delay1.setParameterT(1, 256.); // core::fix_delay::FadeTime
		
		convolution1.setParameterT(0, 1.);     // filters::convolution::Gate
		convolution1.setParameterT(1, 0.);     // filters::convolution::Predelay
		convolution1.setParameterT(2, 0.);     // filters::convolution::Damping
		convolution1.setParameterT(3, 20000.); // filters::convolution::HiCut
		convolution1.setParameterT(4, 0.);     // filters::convolution::Multithread
		
		svf_eq7.setParameterT(0, 150.); // filters::svf_eq::Frequency
		svf_eq7.setParameterT(1, 0.3);  // filters::svf_eq::Q
		;                               // svf_eq7::Gain is automated
		svf_eq7.setParameterT(3, 0.01); // filters::svf_eq::Smoothing
		svf_eq7.setParameterT(4, 2.);   // filters::svf_eq::Mode
		svf_eq7.setParameterT(5, 1.);   // filters::svf_eq::Enabled
		
		svf_eq8.setParameterT(0, 2300.); // filters::svf_eq::Frequency
		svf_eq8.setParameterT(1, 0.3);   // filters::svf_eq::Q
		;                                // svf_eq8::Gain is automated
		svf_eq8.setParameterT(3, 0.01);  // filters::svf_eq::Smoothing
		svf_eq8.setParameterT(4, 3.);    // filters::svf_eq::Mode
		svf_eq8.setParameterT(5, 1.);    // filters::svf_eq::Enabled
		
		svf_eq9.setParameterT(0, 2003.68);  // filters::svf_eq::Frequency
		svf_eq9.setParameterT(1, 0.624712); // filters::svf_eq::Q
		;                                   // svf_eq9::Gain is automated
		svf_eq9.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq9.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq9.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		svf_eq10.setParameterT(0, 4048.25);  // filters::svf_eq::Frequency
		svf_eq10.setParameterT(1, 0.349516); // filters::svf_eq::Q
		;                                    // svf_eq10::Gain is automated
		svf_eq10.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq10.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq10.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		svf_eq11.setParameterT(0, 1000.);    // filters::svf_eq::Frequency
		svf_eq11.setParameterT(1, 0.417865); // filters::svf_eq::Q
		;                                    // svf_eq11::Gain is automated
		svf_eq11.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq11.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq11.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		svf_eq12.setParameterT(0, 4000.);    // filters::svf_eq::Frequency
		svf_eq12.setParameterT(1, 0.548232); // filters::svf_eq::Q
		;                                    // svf_eq12::Gain is automated
		svf_eq12.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq12.setParameterT(4, 3.);       // filters::svf_eq::Mode
		svf_eq12.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		;                              // jpanner2::Pan is automated
		jpanner2.setParameterT(1, 1.); // jdsp::jpanner::Rule
		
		inv1.setParameterT(0, 0.); // math::inv::Value
		
		;                            // gain1::Gain is automated
		gain1.setParameterT(1, 20.); // core::gain::Smoothing
		gain1.setParameterT(2, 0.);  // core::gain::ResetValue
		
		;                            // gain8::Gain is automated
		gain8.setParameterT(1, 20.); // core::gain::Smoothing
		gain8.setParameterT(2, 0.);  // core::gain::ResetValue
		
		this->setParameterT(0, 0.5);
		this->setParameterT(1, 1.);
		this->setParameterT(2, 0.);
		this->setParameterT(3, 0.31);
		this->setParameterT(4, 0.5);
		this->setParameterT(5, 0.);
		this->setParameterT(6, 0.);
		this->setParameterT(7, 0.);
		this->setParameterT(8, 0.);
		this->setParameterT(9, 1.);
		this->setParameterT(10, 0.);
		this->setParameterT(11, 1.);
		this->setParameterT(12, 0.95);
		this->setParameterT(13, 0.);
		this->setParameterT(14, 0.);
		this->setParameterT(15, 0.);
		this->setParameterT(16, 0.);
		this->setExternalData({}, -1);
	}
	~instance() override
	{
		// Cleanup external data references -----------------------------------------------------
		
		this->setExternalData({}, -1);
	}
	
	static constexpr bool isPolyphonic() { return NV > 1; };
	
	static constexpr bool hasTail() { return true; };
	
	static constexpr bool isSuspendedOnSilence() { return false; };
	
	void setExternalData(const ExternalData& b, int index)
	{
		// External Data Connections ------------------------------------------------------------
		
		this->getT(1).getT(0).getT(1).setExternalData(b, index); // cab_impl::convolution2_t
		this->getT(1).getT(1).getT(1).setExternalData(b, index); // cab_impl::convolution1_t
	}
};
}

#undef getT
#undef connectT
#undef setParameterT
#undef setParameterWT
#undef getParameterT
// =====================================| Public Definition |=====================================

namespace project
{
// polyphonic template declaration

template <int NV>
using cab = wrap::node<cab_impl::instance<NV>>;
}


