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
// ==============================| Node & Parameter type declarations |==============================

DECLARE_PARAMETER_RANGE_SKEW(xfader_c0Range, 
                             -100., 
                             0., 
                             10.8445);

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
using xfader1_c0 = parameter::chain<ranges::Identity, 
                                    parameter::plain<math::mul<NV>, 0>, 
                                    parameter::plain<math::mul<NV>, 0>>;

template <int NV>
using xfader1_multimod = parameter::list<xfader1_c0<NV>, 
                                         parameter::plain<math::mul<NV>, 0>>;

template <int NV>
using xfader1_t = control::xfader<xfader1_multimod<NV>, faders::linear>;
using convolution_t = wrap::data<filters::convolution, 
                                 data::external::audiofile<0>>;

template <int NV>
using soft_bypass2_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, math::inv<NV>>>;

template <int NV>
using soft_bypass2_t = bypass::smoothed<20, soft_bypass2_t_<NV>>;

template <int NV>
using soft_bypass_t_ = container::chain<parameter::empty, 
                                        wrap::fix<2, core::fix_delay>, 
                                        convolution_t, 
                                        filters::svf_eq<NV>, 
                                        filters::svf_eq<NV>, 
                                        soft_bypass2_t<NV>, 
                                        jdsp::jpanner<NV>>;

template <int NV>
using soft_bypass_t = bypass::smoothed<20, soft_bypass_t_<NV>>;

template <int NV>
using chain_t = container::chain<parameter::empty, 
                                 wrap::fix<2, soft_bypass_t<NV>>, 
                                 core::gain<NV>, 
                                 core::gain<NV>, 
                                 math::mul<NV>>;
using convolution2_t = wrap::data<filters::convolution, 
                                  data::external::audiofile<1>>;

template <int NV> using soft_bypass3_t_ = soft_bypass2_t_<NV>;

template <int NV>
using soft_bypass3_t = bypass::smoothed<20, soft_bypass3_t_<NV>>;

template <int NV>
using soft_bypass1_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, core::fix_delay>, 
                                         convolution2_t, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         soft_bypass3_t<NV>, 
                                         jdsp::jpanner<NV>>;

template <int NV>
using soft_bypass1_t = bypass::smoothed<20, soft_bypass1_t_<NV>>;

template <int NV>
using chain1_t = container::chain<parameter::empty, 
                                  wrap::fix<2, soft_bypass1_t<NV>>, 
                                  core::gain<NV>, 
                                  core::gain<NV>, 
                                  math::mul<NV>>;

template <int NV>
using chain2_t = container::chain<parameter::empty, 
                                  wrap::fix<2, math::mul<NV>>>;

template <int NV>
using split_t = container::split<parameter::empty, 
                                 wrap::fix<2, chain_t<NV>>, 
                                 chain1_t<NV>, 
                                 chain2_t<NV>>;

namespace cab_t_parameters
{
// Parameter list for cab_impl::cab_t --------------------------------------------------------------

template <int NV>
using CabAEnable = parameter::bypass<cab_impl::soft_bypass_t<NV>>;

template <int NV>
using CabAPhase = parameter::bypass<cab_impl::soft_bypass2_t<NV>>;

template <int NV>
using CabBEnable = parameter::bypass<cab_impl::soft_bypass1_t<NV>>;

template <int NV>
using CabBPhase = parameter::bypass<cab_impl::soft_bypass3_t<NV>>;

template <int NV>
using Mix = parameter::plain<cab_impl::xfader_t<NV>, 0>;
using CabADelay = parameter::plain<core::fix_delay, 0>;
template <int NV>
using CabAAxis = parameter::plain<filters::svf_eq<NV>, 2>;
template <int NV>
using CabADistance = parameter::plain<filters::svf_eq<NV>, 0>;
template <int NV>
using CabAPan = parameter::plain<jdsp::jpanner<NV>, 0>;
template <int NV>
using CabAGain = parameter::plain<core::gain<NV>, 0>;
using CabBDelay = CabADelay;
template <int NV> using CabBAxis = CabAAxis<NV>;
template <int NV> using CabBDistance = CabADistance<NV>;
template <int NV> using CabBPan = CabAPan<NV>;
template <int NV> using CabBGain = CabAGain<NV>;
template <int NV>
using CabDesignerBypass = parameter::plain<cab_impl::xfader1_t<NV>, 
                                           0>;
template <int NV>
using cab_t_plist = parameter::list<Mix<NV>, 
                                    CabAEnable<NV>, 
                                    CabADelay, 
                                    CabAAxis<NV>, 
                                    CabADistance<NV>, 
                                    CabAPhase<NV>, 
                                    CabAPan<NV>, 
                                    CabAGain<NV>, 
                                    CabBEnable<NV>, 
                                    CabBDelay, 
                                    CabBAxis<NV>, 
                                    CabBDistance<NV>, 
                                    CabBPhase<NV>, 
                                    CabBPan<NV>, 
                                    CabBGain<NV>, 
                                    CabDesignerBypass<NV>>;
}

template <int NV>
using cab_t_ = container::chain<cab_t_parameters::cab_t_plist<NV>, 
                                wrap::fix<2, xfader_t<NV>>, 
                                xfader1_t<NV>, 
                                split_t<NV>>;

// =================================| Root node initialiser class |=================================

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
		SNEX_METADATA_ENCODED_PARAMETERS(298)
		{
			0x005C, 0x0000, 0x0000, 0x694D, 0x0078, 0x0000, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x0000, 0x003F, 0x8000, 0x003F, 0x0000, 0x5C00, 
            0x0100, 0x0000, 0x4300, 0x6261, 0x4541, 0x616E, 0x6C62, 0x0065, 
            0x0000, 0x0000, 0x0000, 0x8000, 0x003F, 0x8000, 0x003F, 0x8000, 
            0x003F, 0x8000, 0x5C3F, 0x0200, 0x0000, 0x4300, 0x6261, 0x4441, 
            0x6C65, 0x7961, 0x0000, 0x0000, 0x0000, 0x0000, 0x42C8, 0x0000, 
            0x0000, 0x209B, 0x3E9A, 0xCCCD, 0x3DCC, 0x005C, 0x0003, 0x0000, 
            0x6143, 0x4162, 0x7841, 0x7369, 0x0000, 0x0000, 0xC0C0, 0x0000, 
            0x0000, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 0x0000, 0x015C, 
            0x0004, 0x0000, 0x6143, 0x4162, 0x6944, 0x7473, 0x6E61, 0x6563, 
            0x0000, 0x8000, 0x45BB, 0x4000, 0x469C, 0x8000, 0x45BB, 0x6C1A, 
            0x3E6B, 0x0000, 0x0000, 0x005C, 0x0005, 0x0000, 0x6143, 0x4162, 
            0x6850, 0x7361, 0x0065, 0x0000, 0x0000, 0x0000, 0x8000, 0x003F, 
            0x0000, 0x0000, 0x8000, 0x003F, 0x8000, 0x5C3F, 0x0600, 0x0000, 
            0x4300, 0x6261, 0x5041, 0x6E61, 0x0000, 0x0000, 0xBF80, 0x0000, 
            0x3F80, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 
            0x0007, 0x0000, 0x6143, 0x4162, 0x6147, 0x6E69, 0x0000, 0x0000, 
            0xC2C8, 0x0000, 0x4140, 0x0000, 0x35C8, 0x833E, 0x40AD, 0xCCCD, 
            0x3DCC, 0x005C, 0x0008, 0x0000, 0x6143, 0x4262, 0x6E45, 0x6261, 
            0x656C, 0x0000, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 0x0000, 
            0x0000, 0x3F80, 0x0000, 0x3F80, 0x005C, 0x0009, 0x0000, 0x6143, 
            0x4262, 0x6544, 0x616C, 0x0079, 0x0000, 0x0000, 0x0000, 0xC800, 
            0x0042, 0x0000, 0x9B00, 0x9A20, 0xCD3E, 0xCCCC, 0x5C3D, 0x0A00, 
            0x0000, 0x4300, 0x6261, 0x4142, 0x6978, 0x0073, 0x0000, 0xC000, 
            0x00C0, 0x0000, 0x0000, 0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 
            0x5C00, 0x0B01, 0x0000, 0x4300, 0x6261, 0x4442, 0x7369, 0x6174, 
            0x636E, 0x0065, 0x0000, 0xBB80, 0x0045, 0x9C40, 0x0046, 0xBB80, 
            0x1A45, 0x6B6C, 0x003E, 0x0000, 0x5C00, 0x0C00, 0x0000, 0x4300, 
            0x6261, 0x5042, 0x6168, 0x6573, 0x0000, 0x0000, 0x0000, 0x0000, 
            0x3F80, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 0x3F80, 0x005C, 
            0x000D, 0x0000, 0x6143, 0x4262, 0x6150, 0x006E, 0x0000, 0x8000, 
            0x00BF, 0x8000, 0x003F, 0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 
            0x5C00, 0x0E00, 0x0000, 0x4300, 0x6261, 0x4742, 0x6961, 0x006E, 
            0x0000, 0xC800, 0x00C2, 0x4000, 0x0041, 0xC800, 0x3E35, 0xAD83, 
            0xCD40, 0xCCCC, 0x5C3D, 0x0F00, 0x0000, 0x4300, 0x6261, 0x6544, 
            0x6973, 0x6E67, 0x7265, 0x7942, 0x6170, 0x7373, 0x0000, 0x0000, 
            0x0000, 0x0000, 0x3F80, 0x0000, 0x3F80, 0x0000, 0x3F80, 0xD70A, 
            0x3C23, 0x0000
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
		
		auto& xfader = this->getT(0);                               // cab_impl::xfader_t<NV>
		auto& xfader1 = this->getT(1);                              // cab_impl::xfader1_t<NV>
		auto& split = this->getT(2);                                // cab_impl::split_t<NV>
		auto& chain = this->getT(2).getT(0);                        // cab_impl::chain_t<NV>
		auto& soft_bypass = this->getT(2).getT(0).getT(0);          // cab_impl::soft_bypass_t<NV>
		auto& fix_delay = this->getT(2).getT(0).getT(0).getT(0);    // core::fix_delay
		auto& convolution = this->getT(2).getT(0).getT(0).getT(1);  // cab_impl::convolution_t
		auto& svf_eq3 = this->getT(2).getT(0).getT(0).getT(2);      // filters::svf_eq<NV>
		auto& svf_eq1 = this->getT(2).getT(0).getT(0).getT(3);      // filters::svf_eq<NV>
		auto& soft_bypass2 = this->getT(2).getT(0).getT(0).getT(4); // cab_impl::soft_bypass2_t<NV>
		auto& inv = this->getT(2).getT(0).getT(0).getT(4).getT(0);  // math::inv<NV>
		auto& jpanner = this->getT(2).getT(0).getT(0).getT(5);      // jdsp::jpanner<NV>
		auto& gain2 = this->getT(2).getT(0).getT(1);                // core::gain<NV>
		auto& gain = this->getT(2).getT(0).getT(2);                 // core::gain<NV>
		auto& mul = this->getT(2).getT(0).getT(3);                  // math::mul<NV>
		auto& chain1 = this->getT(2).getT(1);                       // cab_impl::chain1_t<NV>
		auto& soft_bypass1 = this->getT(2).getT(1).getT(0);         // cab_impl::soft_bypass1_t<NV>
		auto& fix_delay2 = this->getT(2).getT(1).getT(0).getT(0);   // core::fix_delay
		auto& convolution2 = this->getT(2).getT(1).getT(0).getT(1); // cab_impl::convolution2_t
		auto& svf_eq4 = this->getT(2).getT(1).getT(0).getT(2);      // filters::svf_eq<NV>
		auto& svf_eq5 = this->getT(2).getT(1).getT(0).getT(3);      // filters::svf_eq<NV>
		auto& soft_bypass3 = this->getT(2).getT(1).getT(0).getT(4); // cab_impl::soft_bypass3_t<NV>
		auto& inv1 = this->getT(2).getT(1).getT(0).getT(4).getT(0); // math::inv<NV>
		auto& jpanner1 = this->getT(2).getT(1).getT(0).getT(5);     // jdsp::jpanner<NV>
		auto& gain4 = this->getT(2).getT(1).getT(1);                // core::gain<NV>
		auto& gain5 = this->getT(2).getT(1).getT(2);                // core::gain<NV>
		auto& mul1 = this->getT(2).getT(1).getT(3);                 // math::mul<NV>
		auto& chain2 = this->getT(2).getT(2);                       // cab_impl::chain2_t<NV>
		auto& mul2 = this->getT(2).getT(2).getT(0);                 // math::mul<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, xfader); // Mix -> xfader::Value
		
		this->getParameterT(1).connectT(0, soft_bypass); // CabAEnable -> soft_bypass::Bypassed
		
		this->getParameterT(2).connectT(0, fix_delay); // CabADelay -> fix_delay::DelayTime
		
		this->getParameterT(3).connectT(0, svf_eq3); // CabAAxis -> svf_eq3::Gain
		
		this->getParameterT(4).connectT(0, svf_eq1); // CabADistance -> svf_eq1::Frequency
		
		this->getParameterT(5).connectT(0, soft_bypass2); // CabAPhase -> soft_bypass2::Bypassed
		
		this->getParameterT(6).connectT(0, jpanner); // CabAPan -> jpanner::Pan
		
		this->getParameterT(7).connectT(0, gain2); // CabAGain -> gain2::Gain
		
		this->getParameterT(8).connectT(0, soft_bypass1); // CabBEnable -> soft_bypass1::Bypassed
		
		this->getParameterT(9).connectT(0, fix_delay2); // CabBDelay -> fix_delay2::DelayTime
		
		this->getParameterT(10).connectT(0, svf_eq4); // CabBAxis -> svf_eq4::Gain
		
		this->getParameterT(11).connectT(0, svf_eq5); // CabBDistance -> svf_eq5::Frequency
		
		this->getParameterT(12).connectT(0, soft_bypass3); // CabBPhase -> soft_bypass3::Bypassed
		
		this->getParameterT(13).connectT(0, jpanner1); // CabBPan -> jpanner1::Pan
		
		this->getParameterT(14).connectT(0, gain4); // CabBGain -> gain4::Gain
		
		this->getParameterT(15).connectT(0, xfader1); // CabDesignerBypass -> xfader1::Value
		
		// Modulation Connections ------------------------------------------------------------------
		
		auto& xfader_p = xfader.getWrappedObject().getParameter();
		xfader_p.getParameterT(0).connectT(0, gain);  // xfader -> gain::Gain
		xfader_p.getParameterT(1).connectT(0, gain5); // xfader -> gain5::Gain
		auto& xfader1_p = xfader1.getWrappedObject().getParameter();
		xfader1_p.getParameterT(0).connectT(0, mul);  // xfader1 -> mul::Value
		xfader1_p.getParameterT(0).connectT(1, mul1); // xfader1 -> mul1::Value
		xfader1_p.getParameterT(1).connectT(0, mul2); // xfader1 -> mul2::Value
		
		// Default Values --------------------------------------------------------------------------
		
		; // xfader::Value is automated
		
		; // xfader1::Value is automated
		
		;                                 // fix_delay::DelayTime is automated
		fix_delay.setParameterT(1, 512.); // core::fix_delay::FadeTime
		
		convolution.setParameterT(0, 1.);     // filters::convolution::Gate
		convolution.setParameterT(1, 0.);     // filters::convolution::Predelay
		convolution.setParameterT(2, 0.);     // filters::convolution::Damping
		convolution.setParameterT(3, 20000.); // filters::convolution::HiCut
		convolution.setParameterT(4, 1.);     // filters::convolution::Multithread
		
		svf_eq3.setParameterT(0, 3000.); // filters::svf_eq::Frequency
		svf_eq3.setParameterT(1, 0.4);   // filters::svf_eq::Q
		;                                // svf_eq3::Gain is automated
		svf_eq3.setParameterT(3, 0.01);  // filters::svf_eq::Smoothing
		svf_eq3.setParameterT(4, 3.);    // filters::svf_eq::Mode
		svf_eq3.setParameterT(5, 1.);    // filters::svf_eq::Enabled
		
		;                                   // svf_eq1::Frequency is automated
		svf_eq1.setParameterT(1, 0.426814); // filters::svf_eq::Q
		svf_eq1.setParameterT(2, 0.);       // filters::svf_eq::Gain
		svf_eq1.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq1.setParameterT(4, 0.);       // filters::svf_eq::Mode
		svf_eq1.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		inv.setParameterT(0, 1.); // math::inv::Value
		
		;                             // jpanner::Pan is automated
		jpanner.setParameterT(1, 1.); // jdsp::jpanner::Rule
		
		;                            // gain2::Gain is automated
		gain2.setParameterT(1, 20.); // core::gain::Smoothing
		gain2.setParameterT(2, 0.);  // core::gain::ResetValue
		
		;                           // gain::Gain is automated
		gain.setParameterT(1, 20.); // core::gain::Smoothing
		gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		; // mul::Value is automated
		
		;                                  // fix_delay2::DelayTime is automated
		fix_delay2.setParameterT(1, 512.); // core::fix_delay::FadeTime
		
		convolution2.setParameterT(0, 1.);     // filters::convolution::Gate
		convolution2.setParameterT(1, 0.);     // filters::convolution::Predelay
		convolution2.setParameterT(2, 0.);     // filters::convolution::Damping
		convolution2.setParameterT(3, 20000.); // filters::convolution::HiCut
		convolution2.setParameterT(4, 1.);     // filters::convolution::Multithread
		
		svf_eq4.setParameterT(0, 3000.); // filters::svf_eq::Frequency
		svf_eq4.setParameterT(1, 0.4);   // filters::svf_eq::Q
		;                                // svf_eq4::Gain is automated
		svf_eq4.setParameterT(3, 0.01);  // filters::svf_eq::Smoothing
		svf_eq4.setParameterT(4, 3.);    // filters::svf_eq::Mode
		svf_eq4.setParameterT(5, 1.);    // filters::svf_eq::Enabled
		
		;                                   // svf_eq5::Frequency is automated
		svf_eq5.setParameterT(1, 0.426814); // filters::svf_eq::Q
		svf_eq5.setParameterT(2, 0.);       // filters::svf_eq::Gain
		svf_eq5.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq5.setParameterT(4, 0.);       // filters::svf_eq::Mode
		svf_eq5.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		inv1.setParameterT(0, 1.); // math::inv::Value
		
		;                              // jpanner1::Pan is automated
		jpanner1.setParameterT(1, 1.); // jdsp::jpanner::Rule
		
		;                            // gain4::Gain is automated
		gain4.setParameterT(1, 20.); // core::gain::Smoothing
		gain4.setParameterT(2, 0.);  // core::gain::ResetValue
		
		;                            // gain5::Gain is automated
		gain5.setParameterT(1, 20.); // core::gain::Smoothing
		gain5.setParameterT(2, 0.);  // core::gain::ResetValue
		
		; // mul1::Value is automated
		
		; // mul2::Value is automated
		
		this->setParameterT(0, 0.5);
		this->setParameterT(1, 1.);
		this->setParameterT(2, 0.);
		this->setParameterT(3, 0.);
		this->setParameterT(4, 6000.);
		this->setParameterT(5, 0.);
		this->setParameterT(6, 0.);
		this->setParameterT(7, 1.49012e-06);
		this->setParameterT(8, 0.);
		this->setParameterT(9, 0.);
		this->setParameterT(10, 0.);
		this->setParameterT(11, 6000.);
		this->setParameterT(12, 0.);
		this->setParameterT(13, 0.);
		this->setParameterT(14, 1.49012e-06);
		this->setParameterT(15, 1.);
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
		
		this->getT(2).getT(0).getT(0).getT(1).setExternalData(b, index); // cab_impl::convolution_t
		this->getT(2).getT(1).getT(0).getT(1).setExternalData(b, index); // cab_impl::convolution2_t
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
using cab = wrap::node<cab_impl::instance<NV>>;
}


