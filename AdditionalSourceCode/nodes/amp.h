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

namespace amp_impl
{
// ==============================| Node & Parameter type declarations |==============================

template <int NumVoices> struct clean_channel
{
	SNEX_NODE(clean_channel);
	float pi = 3.1415926536f; // ¯\_(ツ)_/¯
	float warmth = 0.3f;
	float threshold = 0.8f;
	float scaledInput = 0.0f;
	float abs_input = 0.0f;
	float excess = 0.0f;
	float compressed = 0.0f;
	float y = 0.0f;
	float cleanTubeWarmth(float input)
	{
		// Scale Input
		scaledInput = input * (0.3f + warmth * 0.5f);
		abs_input = Math.abs(scaledInput);
		// Squish a little bit
		if (abs_input > threshold)
		{
			excess = abs_input - threshold;
			compressed = threshold + Math.tanh(excess * 2.0f) * 0.2f;
			if (scaledInput > 0.0f) // positive
			{
				scaledInput = compressed;
			}
			else // negative
			{
				scaledInput = 0.0f-compressed;
			}
		}
		// Even harmonics
		float harmonic = Math.sin(scaledInput * 4.0f) * warmth * 0.02f; // maybe should be pow or exp
		y = scaledInput + harmonic;
		return y;
	}
	// Implement the Waveshaper here...
	float getSample(float input)
	{
		return cleanTubeWarmth(input);
	}
	// These functions are the glue code that call the function above
	template <typename T> void process(T& data)
	{
		for(auto ch: data)
		{
			for(auto& s: data.toChannelData(ch))
			{
				s = getSample(s);
			}
		}
	}
	template <typename T> void processFrame(T& data)
	{
		for(auto& s: data)
			s = getSample(s);
	}
	void reset()
	{
	}
	void prepare(PrepareSpecs ps)
	{
	}
	void setExternalData(const ExternalData& d, int index)
	{
	}
	template <int P> void setParameter(double v)
	{
	}
};

template <int NV>
using snex_shaper2_t = wrap::no_data<core::snex_shaper<clean_channel<NV>>>;

template <int NV>
using soft_bypass5_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, snex_shaper2_t<NV>>>;

template <int NV>
using soft_bypass5_t = bypass::smoothed<20, soft_bypass5_t_<NV>>;
template <int NV> using snex_shaper3_t = snex_shaper2_t<NV>;

template <int NV>
using oversample4x1_t_ = container::chain<parameter::empty, 
                                          wrap::fix<2, snex_shaper3_t<NV>>>;

template <int NV>
using oversample4x1_t = wrap::oversample<4, oversample4x1_t_<NV>>;

template <int NV>
using soft_bypass6_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, oversample4x1_t<NV>>>;

template <int NV>
using soft_bypass6_t = bypass::smoothed<20, soft_bypass6_t_<NV>>;

template <int NV>
using soft_bypass7_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, core::gain<NV>>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         soft_bypass5_t<NV>, 
                                         soft_bypass6_t<NV>, 
                                         filters::svf_eq<NV>, 
                                         core::gain<NV>>;

template <int NV>
using soft_bypass7_t = bypass::smoothed<20, soft_bypass7_t_<NV>>;
template <int NV>
using xfader1_c0 = parameter::bypass<soft_bypass7_t<NV>>;

template <int NumVoices> struct dirty_channel
{
	SNEX_NODE(dirty_channel);
	// Add coefficients etc here
	float pi = 3.1415926536f;
	float threshold = 0.7f;
	float y = 0.0f;
	float modernTube(float input)
	{
		// Stage 1
		if (input > 0.0f)
		{
			y = (2.0f / pi) * Math.atan(input * 1.5f);
		}
		else
		{
			y = Math.tanh(input * 2.2f);
			y *= 0.85f;
		}
		// Stage 2
		if (Math.abs(y) > threshold)
		{
			float sign = (y > 0.0f) ? 1.0f : -1.0f;
			float excess = Math.abs(y) - threshold;
			float clipped = threshold + Math.tanh(excess * 8.0f) * 0.2f;
			y = sign * clipped;
		}
		return y;
	}
	// Implement the Waveshaper here...
	float getSample(float input)
	{
		return modernTube(input);
	}
	// These functions are the glue code that call the function above
	template <typename T> void process(T& data)
	{
		for(auto ch: data)
		{
			for(auto& s: data.toChannelData(ch))
			{
				s = getSample(s);
			}
		}
	}
	template <typename T> void processFrame(T& data)
	{
		for(auto& s: data)
			s = getSample(s);
	}
	void reset()
	{
	}
	void prepare(PrepareSpecs ps)
	{
	}
	void setExternalData(const ExternalData& d, int index)
	{
	}
	template <int P> void setParameter(double v)
	{
	}
};

template <int NV>
using snex_shaper_t = wrap::no_data<core::snex_shaper<dirty_channel<NV>>>;

template <int NV>
using soft_bypass2_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, snex_shaper_t<NV>>>;

template <int NV>
using soft_bypass2_t = bypass::smoothed<20, soft_bypass2_t_<NV>>;
template <int NV> using snex_shaper1_t = snex_shaper_t<NV>;

template <int NV>
using oversample4x_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, snex_shaper1_t<NV>>>;

template <int NV>
using oversample4x_t = wrap::oversample<4, oversample4x_t_<NV>>;

template <int NV>
using soft_bypass4_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, oversample4x_t<NV>>>;

template <int NV>
using soft_bypass4_t = bypass::smoothed<20, soft_bypass4_t_<NV>>;

template <int NV>
using soft_bypass3_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, core::gain<NV>>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         soft_bypass2_t<NV>, 
                                         soft_bypass4_t<NV>, 
                                         filters::svf_eq<NV>, 
                                         core::gain<NV>>;

template <int NV>
using soft_bypass3_t = bypass::smoothed<20, soft_bypass3_t_<NV>>;
template <int NV>
using xfader1_c1 = parameter::bypass<soft_bypass3_t<NV>>;

template <int NV>
using xfader1_multimod = parameter::list<xfader1_c0<NV>, xfader1_c1<NV>>;

template <int NV>
using xfader1_t = control::xfader<xfader1_multimod<NV>, faders::switcher>;

template <int NV>
using xfader2_c0_0 = parameter::bypass<soft_bypass5_t<NV>>;

template <int NV>
using xfader2_c0_1 = parameter::bypass<soft_bypass2_t<NV>>;

template <int NV>
using xfader2_c0 = parameter::chain<ranges::Identity, 
                                    xfader2_c0_0<NV>, 
                                    xfader2_c0_1<NV>>;

template <int NV>
using xfader2_c1_0 = parameter::bypass<soft_bypass6_t<NV>>;

template <int NV>
using xfader2_c1_1 = parameter::bypass<soft_bypass4_t<NV>>;

template <int NV>
using xfader2_c1 = parameter::chain<ranges::Identity, 
                                    xfader2_c1_0<NV>, 
                                    xfader2_c1_1<NV>>;

template <int NV>
using xfader2_multimod = parameter::list<xfader2_c0<NV>, xfader2_c1<NV>>;

template <int NV>
using xfader2_t = control::xfader<xfader2_multimod<NV>, faders::switcher>;

template <int NV>
using soft_bypass_t_ = container::chain<parameter::empty, 
                                        wrap::fix<2, filters::svf_eq<NV>>, 
                                        filters::svf_eq<NV>, 
                                        filters::svf_eq<NV>, 
                                        filters::svf_eq<NV>>;

template <int NV>
using soft_bypass_t = bypass::smoothed<20, soft_bypass_t_<NV>>;

template <int NV>
using chain2_t = container::chain<parameter::empty, 
                                  wrap::fix<2, soft_bypass7_t<NV>>, 
                                  soft_bypass3_t<NV>>;

template <int NV>
using soft_bypass1_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, filters::svf_eq<NV>>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>>;

template <int NV>
using soft_bypass1_t = bypass::smoothed<20, soft_bypass1_t_<NV>>;

namespace amp_t_parameters
{
// Parameter list for amp_impl::amp_t --------------------------------------------------------------

template <int NV>
using Channel = parameter::plain<amp_impl::xfader1_t<NV>, 
                                 0>;
template <int NV>
using InputGainClean = parameter::plain<core::gain<NV>, 0>;
template <int NV> using InputGainDirty = InputGainClean<NV>;
template <int NV> using OutputGainClean = InputGainClean<NV>;
template <int NV> using OutputGainDirty = InputGainClean<NV>;
template <int NV>
using Oversampling = parameter::plain<amp_impl::xfader2_t<NV>, 
                                      0>;
template <int NV>
using CleanLow = parameter::plain<filters::svf_eq<NV>, 2>;
template <int NV> using CleanMid = CleanLow<NV>;
template <int NV> using CleanHigh = CleanLow<NV>;
template <int NV> using CleanPresence = CleanLow<NV>;
template <int NV> using DirtyLow = CleanLow<NV>;
template <int NV> using DirtyMid = CleanLow<NV>;
template <int NV> using DirtyHigh = CleanLow<NV>;
template <int NV> using DirtyPresence = CleanLow<NV>;
template <int NV>
using amp_t_plist = parameter::list<Channel<NV>, 
                                    InputGainClean<NV>, 
                                    InputGainDirty<NV>, 
                                    OutputGainClean<NV>, 
                                    OutputGainDirty<NV>, 
                                    Oversampling<NV>, 
                                    CleanLow<NV>, 
                                    CleanMid<NV>, 
                                    CleanHigh<NV>, 
                                    CleanPresence<NV>, 
                                    DirtyLow<NV>, 
                                    DirtyMid<NV>, 
                                    DirtyHigh<NV>, 
                                    DirtyPresence<NV>>;
}

template <int NV>
using amp_t_ = container::chain<amp_t_parameters::amp_t_plist<NV>, 
                                wrap::fix<2, xfader1_t<NV>>, 
                                xfader2_t<NV>, 
                                soft_bypass_t<NV>, 
                                chain2_t<NV>, 
                                soft_bypass1_t<NV>>;

// =================================| Root node initialiser class |=================================

template <int NV> struct instance: public amp_impl::amp_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(amp);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(266)
		{
			0x005C, 0x0000, 0x0000, 0x6843, 0x6E61, 0x656E, 0x006C, 0x0000, 
            0x0000, 0x0000, 0x3F80, 0x0000, 0x3F80, 0x0000, 0x3F80, 0x0000, 
            0x3F80, 0x005C, 0x0001, 0x0000, 0x6E49, 0x7570, 0x4774, 0x6961, 
            0x436E, 0x656C, 0x6E61, 0x0000, 0xC800, 0x00C2, 0xC800, 0x0042, 
            0x0000, 0x0000, 0x8000, 0xCD3F, 0xCCCC, 0x5C3D, 0x0200, 0x0000, 
            0x4900, 0x706E, 0x7475, 0x6147, 0x6E69, 0x6944, 0x7472, 0x0079, 
            0x0000, 0xC2C8, 0x0000, 0x42C8, 0x0000, 0x4296, 0x0000, 0x3F80, 
            0xCCCD, 0x3DCC, 0x005C, 0x0003, 0x0000, 0x754F, 0x7074, 0x7475, 
            0x6147, 0x6E69, 0x6C43, 0x6165, 0x006E, 0x0000, 0xC2C8, 0x0000, 
            0x42C8, 0x0000, 0x0000, 0x0000, 0x3F80, 0xCCCD, 0x3DCC, 0x005C, 
            0x0004, 0x0000, 0x754F, 0x7074, 0x7475, 0x6147, 0x6E69, 0x6944, 
            0x7472, 0x0079, 0x0000, 0xC2C8, 0x0000, 0x42C8, 0x0000, 0x0000, 
            0x0000, 0x3F80, 0xCCCD, 0x3DCC, 0x005C, 0x0005, 0x0000, 0x764F, 
            0x7265, 0x6173, 0x706D, 0x696C, 0x676E, 0x0000, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x0000, 0x0000, 0x8000, 0x003F, 0x8000, 0x5C3F, 
            0x0600, 0x0000, 0x4300, 0x656C, 0x6E61, 0x6F4C, 0x0077, 0x0000, 
            0xC140, 0x0000, 0x4140, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 
            0x0000, 0x005C, 0x0007, 0x0000, 0x6C43, 0x6165, 0x4D6E, 0x6469, 
            0x0000, 0x4000, 0x00C1, 0x4000, 0x0041, 0x0000, 0x0000, 0x8000, 
            0x003F, 0x0000, 0x5C00, 0x0800, 0x0000, 0x4300, 0x656C, 0x6E61, 
            0x6948, 0x6867, 0x0000, 0x4000, 0x00C1, 0x4000, 0x0041, 0x0000, 
            0x0000, 0x8000, 0x003F, 0x0000, 0x5C00, 0x0900, 0x0000, 0x4300, 
            0x656C, 0x6E61, 0x7250, 0x7365, 0x6E65, 0x6563, 0x0000, 0x4000, 
            0x00C1, 0x4000, 0x0041, 0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 
            0x5C00, 0x0A00, 0x0000, 0x4400, 0x7269, 0x7974, 0x6F4C, 0x0077, 
            0x0000, 0xC140, 0x0000, 0x4140, 0x0000, 0x0000, 0x0000, 0x3F80, 
            0x0000, 0x0000, 0x005C, 0x000B, 0x0000, 0x6944, 0x7472, 0x4D79, 
            0x6469, 0x0000, 0x4000, 0x00C1, 0x4000, 0x0041, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x0000, 0x5C00, 0x0C00, 0x0000, 0x4400, 0x7269, 
            0x7974, 0x6948, 0x6867, 0x0000, 0x4000, 0x00C1, 0x4000, 0x0041, 
            0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 0x5C00, 0x0D00, 0x0000, 
            0x4400, 0x7269, 0x7974, 0x7250, 0x7365, 0x6E65, 0x6563, 0x0000, 
            0x4000, 0x00C1, 0x4000, 0x0041, 0x0000, 0x0000, 0x8000, 0x003F, 
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
		
		auto& xfader1 = this->getT(0);                                      // amp_impl::xfader1_t<NV>
		auto& xfader2 = this->getT(1);                                      // amp_impl::xfader2_t<NV>
		auto& soft_bypass = this->getT(2);                                  // amp_impl::soft_bypass_t<NV>
		auto& svf_eq9 = this->getT(2).getT(0);                              // filters::svf_eq<NV>
		auto& svf_eq10 = this->getT(2).getT(1);                             // filters::svf_eq<NV>
		auto& svf_eq11 = this->getT(2).getT(2);                             // filters::svf_eq<NV>
		auto& svf_eq12 = this->getT(2).getT(3);                             // filters::svf_eq<NV>
		auto& chain2 = this->getT(3);                                       // amp_impl::chain2_t<NV>
		auto& soft_bypass7 = this->getT(3).getT(0);                         // amp_impl::soft_bypass7_t<NV>
		auto& gain8 = this->getT(3).getT(0).getT(0);                        // core::gain<NV>
		auto& svf_eq = this->getT(3).getT(0).getT(1);                       // filters::svf_eq<NV>
		auto& svf_eq3 = this->getT(3).getT(0).getT(2);                      // filters::svf_eq<NV>
		auto& svf_eq2 = this->getT(3).getT(0).getT(3);                      // filters::svf_eq<NV>
		auto& soft_bypass5 = this->getT(3).getT(0).getT(4);                 // amp_impl::soft_bypass5_t<NV>
		auto& snex_shaper2 = this->getT(3).getT(0).getT(4).getT(0);         // amp_impl::snex_shaper2_t<NV>
		auto& soft_bypass6 = this->getT(3).getT(0).getT(5);                 // amp_impl::soft_bypass6_t<NV>
		auto& oversample4x1 = this->getT(3).getT(0).getT(5).getT(0);        // amp_impl::oversample4x1_t<NV>
		auto& snex_shaper3 = this->getT(3).getT(0).getT(5).getT(0).getT(0); // amp_impl::snex_shaper3_t<NV>
		auto& svf_eq1 = this->getT(3).getT(0).getT(6);                      // filters::svf_eq<NV>
		auto& gain9 = this->getT(3).getT(0).getT(7);                        // core::gain<NV>
		auto& soft_bypass3 = this->getT(3).getT(1);                         // amp_impl::soft_bypass3_t<NV>
		auto& gain6 = this->getT(3).getT(1).getT(0);                        // core::gain<NV>
		auto& svf_eq7 = this->getT(3).getT(1).getT(1);                      // filters::svf_eq<NV>
		auto& svf_eq6 = this->getT(3).getT(1).getT(2);                      // filters::svf_eq<NV>
		auto& svf_eq5 = this->getT(3).getT(1).getT(3);                      // filters::svf_eq<NV>
		auto& soft_bypass2 = this->getT(3).getT(1).getT(4);                 // amp_impl::soft_bypass2_t<NV>
		auto& snex_shaper = this->getT(3).getT(1).getT(4).getT(0);          // amp_impl::snex_shaper_t<NV>
		auto& soft_bypass4 = this->getT(3).getT(1).getT(5);                 // amp_impl::soft_bypass4_t<NV>
		auto& oversample4x = this->getT(3).getT(1).getT(5).getT(0);         // amp_impl::oversample4x_t<NV>
		auto& snex_shaper1 = this->getT(3).getT(1).getT(5).getT(0).getT(0); // amp_impl::snex_shaper1_t<NV>
		auto& svf_eq4 = this->getT(3).getT(1).getT(6);                      // filters::svf_eq<NV>
		auto& gain7 = this->getT(3).getT(1).getT(7);                        // core::gain<NV>
		auto& soft_bypass1 = this->getT(4);                                 // amp_impl::soft_bypass1_t<NV>
		auto& svf_eq8 = this->getT(4).getT(0);                              // filters::svf_eq<NV>
		auto& svf_eq13 = this->getT(4).getT(1);                             // filters::svf_eq<NV>
		auto& svf_eq14 = this->getT(4).getT(2);                             // filters::svf_eq<NV>
		auto& svf_eq15 = this->getT(4).getT(3);                             // filters::svf_eq<NV>
		auto& svf_eq16 = this->getT(4).getT(4);                             // filters::svf_eq<NV>
		auto& svf_eq17 = this->getT(4).getT(5);                             // filters::svf_eq<NV>
		auto& svf_eq18 = this->getT(4).getT(6);                             // filters::svf_eq<NV>
		auto& svf_eq19 = this->getT(4).getT(7);                             // filters::svf_eq<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, xfader1); // Channel -> xfader1::Value
		
		this->getParameterT(1).connectT(0, gain8); // InputGainClean -> gain8::Gain
		
		this->getParameterT(2).connectT(0, gain6); // InputGainDirty -> gain6::Gain
		
		this->getParameterT(3).connectT(0, gain9); // OutputGainClean -> gain9::Gain
		
		this->getParameterT(4).connectT(0, gain7); // OutputGainDirty -> gain7::Gain
		
		this->getParameterT(5).connectT(0, xfader2); // Oversampling -> xfader2::Value
		
		this->getParameterT(6).connectT(0, svf_eq); // CleanLow -> svf_eq::Gain
		
		this->getParameterT(7).connectT(0, svf_eq3); // CleanMid -> svf_eq3::Gain
		
		this->getParameterT(8).connectT(0, svf_eq2); // CleanHigh -> svf_eq2::Gain
		
		this->getParameterT(9).connectT(0, svf_eq1); // CleanPresence -> svf_eq1::Gain
		
		this->getParameterT(10).connectT(0, svf_eq7); // DirtyLow -> svf_eq7::Gain
		
		this->getParameterT(11).connectT(0, svf_eq6); // DirtyMid -> svf_eq6::Gain
		
		this->getParameterT(12).connectT(0, svf_eq5); // DirtyHigh -> svf_eq5::Gain
		
		this->getParameterT(13).connectT(0, svf_eq4); // DirtyPresence -> svf_eq4::Gain
		
		// Modulation Connections ------------------------------------------------------------------
		
		auto& xfader1_p = xfader1.getWrappedObject().getParameter();
		xfader1_p.getParameterT(0).connectT(0, soft_bypass7); // xfader1 -> soft_bypass7::Bypassed
		xfader1_p.getParameterT(1).connectT(0, soft_bypass3); // xfader1 -> soft_bypass3::Bypassed
		auto& xfader2_p = xfader2.getWrappedObject().getParameter();
		xfader2_p.getParameterT(0).connectT(0, soft_bypass5); // xfader2 -> soft_bypass5::Bypassed
		xfader2_p.getParameterT(0).connectT(1, soft_bypass2); // xfader2 -> soft_bypass2::Bypassed
		xfader2_p.getParameterT(1).connectT(0, soft_bypass6); // xfader2 -> soft_bypass6::Bypassed
		xfader2_p.getParameterT(1).connectT(1, soft_bypass4); // xfader2 -> soft_bypass4::Bypassed
		
		// Default Values --------------------------------------------------------------------------
		
		; // xfader1::Value is automated
		
		; // xfader2::Value is automated
		
		svf_eq9.setParameterT(0, 55.);  // filters::svf_eq::Frequency
		svf_eq9.setParameterT(1, 1.);   // filters::svf_eq::Q
		svf_eq9.setParameterT(2, 0.);   // filters::svf_eq::Gain
		svf_eq9.setParameterT(3, 0.01); // filters::svf_eq::Smoothing
		svf_eq9.setParameterT(4, 1.);   // filters::svf_eq::Mode
		svf_eq9.setParameterT(5, 1.);   // filters::svf_eq::Enabled
		
		svf_eq10.setParameterT(0, 870.); // filters::svf_eq::Frequency
		svf_eq10.setParameterT(1, 0.43); // filters::svf_eq::Q
		svf_eq10.setParameterT(2, 4.);   // filters::svf_eq::Gain
		svf_eq10.setParameterT(3, 0.01); // filters::svf_eq::Smoothing
		svf_eq10.setParameterT(4, 4.);   // filters::svf_eq::Mode
		svf_eq10.setParameterT(5, 1.);   // filters::svf_eq::Enabled
		
		svf_eq11.setParameterT(0, 3700.); // filters::svf_eq::Frequency
		svf_eq11.setParameterT(1, 0.3);   // filters::svf_eq::Q
		svf_eq11.setParameterT(2, 7.);    // filters::svf_eq::Gain
		svf_eq11.setParameterT(3, 0.01);  // filters::svf_eq::Smoothing
		svf_eq11.setParameterT(4, 4.);    // filters::svf_eq::Mode
		svf_eq11.setParameterT(5, 1.);    // filters::svf_eq::Enabled
		
		svf_eq12.setParameterT(0, 7000.); // filters::svf_eq::Frequency
		svf_eq12.setParameterT(1, 0.7);   // filters::svf_eq::Q
		svf_eq12.setParameterT(2, 16.);   // filters::svf_eq::Gain
		svf_eq12.setParameterT(3, 0.01);  // filters::svf_eq::Smoothing
		svf_eq12.setParameterT(4, 3.);    // filters::svf_eq::Mode
		svf_eq12.setParameterT(5, 1.);    // filters::svf_eq::Enabled
		
		;                            // gain8::Gain is automated
		gain8.setParameterT(1, 20.); // core::gain::Smoothing
		gain8.setParameterT(2, 0.);  // core::gain::ResetValue
		
		svf_eq.setParameterT(0, 110.);     // filters::svf_eq::Frequency
		svf_eq.setParameterT(1, 0.539866); // filters::svf_eq::Q
		;                                  // svf_eq::Gain is automated
		svf_eq.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		svf_eq3.setParameterT(0, 550.); // filters::svf_eq::Frequency
		svf_eq3.setParameterT(1, 1.);   // filters::svf_eq::Q
		;                               // svf_eq3::Gain is automated
		svf_eq3.setParameterT(3, 0.01); // filters::svf_eq::Smoothing
		svf_eq3.setParameterT(4, 4.);   // filters::svf_eq::Mode
		svf_eq3.setParameterT(5, 1.);   // filters::svf_eq::Enabled
		
		svf_eq2.setParameterT(0, 3600.);    // filters::svf_eq::Frequency
		svf_eq2.setParameterT(1, 0.539866); // filters::svf_eq::Q
		;                                   // svf_eq2::Gain is automated
		svf_eq2.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq2.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq2.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		oversample4x1.setParameterT(0, 0.); // container::chain::FilterType
		
		svf_eq1.setParameterT(0, 6500.);    // filters::svf_eq::Frequency
		svf_eq1.setParameterT(1, 0.539866); // filters::svf_eq::Q
		;                                   // svf_eq1::Gain is automated
		svf_eq1.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq1.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq1.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		;                            // gain9::Gain is automated
		gain9.setParameterT(1, 20.); // core::gain::Smoothing
		gain9.setParameterT(2, 0.);  // core::gain::ResetValue
		
		;                            // gain6::Gain is automated
		gain6.setParameterT(1, 20.); // core::gain::Smoothing
		gain6.setParameterT(2, 0.);  // core::gain::ResetValue
		
		svf_eq7.setParameterT(0, 110.);     // filters::svf_eq::Frequency
		svf_eq7.setParameterT(1, 0.539866); // filters::svf_eq::Q
		;                                   // svf_eq7::Gain is automated
		svf_eq7.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq7.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq7.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		svf_eq6.setParameterT(0, 550.); // filters::svf_eq::Frequency
		svf_eq6.setParameterT(1, 1.);   // filters::svf_eq::Q
		;                               // svf_eq6::Gain is automated
		svf_eq6.setParameterT(3, 0.01); // filters::svf_eq::Smoothing
		svf_eq6.setParameterT(4, 4.);   // filters::svf_eq::Mode
		svf_eq6.setParameterT(5, 1.);   // filters::svf_eq::Enabled
		
		svf_eq5.setParameterT(0, 3600.);    // filters::svf_eq::Frequency
		svf_eq5.setParameterT(1, 0.539866); // filters::svf_eq::Q
		;                                   // svf_eq5::Gain is automated
		svf_eq5.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq5.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq5.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		oversample4x.setParameterT(0, 0.); // container::chain::FilterType
		
		svf_eq4.setParameterT(0, 6500.);    // filters::svf_eq::Frequency
		svf_eq4.setParameterT(1, 0.539866); // filters::svf_eq::Q
		;                                   // svf_eq4::Gain is automated
		svf_eq4.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq4.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq4.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		;                            // gain7::Gain is automated
		gain7.setParameterT(1, 20.); // core::gain::Smoothing
		gain7.setParameterT(2, 0.);  // core::gain::ResetValue
		
		svf_eq8.setParameterT(0, 60.);  // filters::svf_eq::Frequency
		svf_eq8.setParameterT(1, 0.6);  // filters::svf_eq::Q
		svf_eq8.setParameterT(2, 0.);   // filters::svf_eq::Gain
		svf_eq8.setParameterT(3, 0.01); // filters::svf_eq::Smoothing
		svf_eq8.setParameterT(4, 1.);   // filters::svf_eq::Mode
		svf_eq8.setParameterT(5, 1.);   // filters::svf_eq::Enabled
		
		svf_eq13.setParameterT(0, 140.); // filters::svf_eq::Frequency
		svf_eq13.setParameterT(1, 2.37); // filters::svf_eq::Q
		svf_eq13.setParameterT(2, -3.5); // filters::svf_eq::Gain
		svf_eq13.setParameterT(3, 0.01); // filters::svf_eq::Smoothing
		svf_eq13.setParameterT(4, 4.);   // filters::svf_eq::Mode
		svf_eq13.setParameterT(5, 1.);   // filters::svf_eq::Enabled
		
		svf_eq14.setParameterT(0, 667.); // filters::svf_eq::Frequency
		svf_eq14.setParameterT(1, 1.2);  // filters::svf_eq::Q
		svf_eq14.setParameterT(2, -6.);  // filters::svf_eq::Gain
		svf_eq14.setParameterT(3, 0.01); // filters::svf_eq::Smoothing
		svf_eq14.setParameterT(4, 4.);   // filters::svf_eq::Mode
		svf_eq14.setParameterT(5, 1.);   // filters::svf_eq::Enabled
		
		svf_eq15.setParameterT(0, 2200.); // filters::svf_eq::Frequency
		svf_eq15.setParameterT(1, 0.3);   // filters::svf_eq::Q
		svf_eq15.setParameterT(2, 4.);    // filters::svf_eq::Gain
		svf_eq15.setParameterT(3, 0.01);  // filters::svf_eq::Smoothing
		svf_eq15.setParameterT(4, 4.);    // filters::svf_eq::Mode
		svf_eq15.setParameterT(5, 1.);    // filters::svf_eq::Enabled
		
		svf_eq16.setParameterT(0, 2600.); // filters::svf_eq::Frequency
		svf_eq16.setParameterT(1, 5.77);  // filters::svf_eq::Q
		svf_eq16.setParameterT(2, -5.1);  // filters::svf_eq::Gain
		svf_eq16.setParameterT(3, 0.01);  // filters::svf_eq::Smoothing
		svf_eq16.setParameterT(4, 4.);    // filters::svf_eq::Mode
		svf_eq16.setParameterT(5, 1.);    // filters::svf_eq::Enabled
		
		svf_eq17.setParameterT(0, 4000.); // filters::svf_eq::Frequency
		svf_eq17.setParameterT(1, 5.2);   // filters::svf_eq::Q
		svf_eq17.setParameterT(2, -7.);   // filters::svf_eq::Gain
		svf_eq17.setParameterT(3, 0.01);  // filters::svf_eq::Smoothing
		svf_eq17.setParameterT(4, 4.);    // filters::svf_eq::Mode
		svf_eq17.setParameterT(5, 1.);    // filters::svf_eq::Enabled
		
		svf_eq18.setParameterT(0, 6400.); // filters::svf_eq::Frequency
		svf_eq18.setParameterT(1, 1.);    // filters::svf_eq::Q
		svf_eq18.setParameterT(2, 4.8);   // filters::svf_eq::Gain
		svf_eq18.setParameterT(3, 0.01);  // filters::svf_eq::Smoothing
		svf_eq18.setParameterT(4, 3.);    // filters::svf_eq::Mode
		svf_eq18.setParameterT(5, 1.);    // filters::svf_eq::Enabled
		
		svf_eq19.setParameterT(0, 10000.); // filters::svf_eq::Frequency
		svf_eq19.setParameterT(1, 1.);     // filters::svf_eq::Q
		svf_eq19.setParameterT(2, 0.);     // filters::svf_eq::Gain
		svf_eq19.setParameterT(3, 0.01);   // filters::svf_eq::Smoothing
		svf_eq19.setParameterT(4, 0.);     // filters::svf_eq::Mode
		svf_eq19.setParameterT(5, 1.);     // filters::svf_eq::Enabled
		
		this->setParameterT(0, 1.);
		this->setParameterT(1, 0.);
		this->setParameterT(2, 75.);
		this->setParameterT(3, 0.);
		this->setParameterT(4, 0.);
		this->setParameterT(5, 0.);
		this->setParameterT(6, 0.);
		this->setParameterT(7, 0.);
		this->setParameterT(8, 0.);
		this->setParameterT(9, 0.);
		this->setParameterT(10, 0.);
		this->setParameterT(11, 0.);
		this->setParameterT(12, 0.);
		this->setParameterT(13, 0.);
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
		
		this->getT(3).getT(0).getT(4).getT(0).setExternalData(b, index);         // amp_impl::snex_shaper2_t<NV>
		this->getT(3).getT(0).getT(5).getT(0).getT(0).setExternalData(b, index); // amp_impl::snex_shaper3_t<NV>
		this->getT(3).getT(1).getT(4).getT(0).setExternalData(b, index);         // amp_impl::snex_shaper_t<NV>
		this->getT(3).getT(1).getT(5).getT(0).getT(0).setExternalData(b, index); // amp_impl::snex_shaper1_t<NV>
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
using amp = wrap::node<amp_impl::instance<NV>>;
}


