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
using oversample4x4_t_ = container::chain<parameter::empty, 
                                          wrap::fix<2, snex_shaper2_t<NV>>>;

template <int NV>
using oversample4x4_t = wrap::oversample<4, oversample4x4_t_<NV>>;

template <int NV>
using soft_bypass9_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, oversample4x4_t<NV>>>;

template <int NV>
using soft_bypass9_t = bypass::smoothed<20, soft_bypass9_t_<NV>>;
template <int NV> using snex_shaper6_t = snex_shaper2_t<NV>;

template <int NV>
using soft_bypass16_t_ = container::chain<parameter::empty, 
                                          wrap::fix<2, snex_shaper6_t<NV>>>;

template <int NV>
using soft_bypass16_t = bypass::smoothed<20, soft_bypass16_t_<NV>>;

template <int NV>
using soft_bypass7_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, core::gain<NV>>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         soft_bypass9_t<NV>, 
                                         soft_bypass16_t<NV>, 
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
using snex_shaper4_t = wrap::no_data<core::snex_shaper<dirty_channel<NV>>>;

template <int NV>
using oversample4x3_t_ = container::chain<parameter::empty, 
                                          wrap::fix<2, snex_shaper4_t<NV>>>;

template <int NV>
using oversample4x3_t = wrap::oversample<4, oversample4x3_t_<NV>>;

template <int NV>
using soft_bypass10_t_ = container::chain<parameter::empty, 
                                          wrap::fix<2, oversample4x3_t<NV>>>;

template <int NV>
using soft_bypass10_t = bypass::smoothed<20, soft_bypass10_t_<NV>>;
template <int NV> using snex_shaper5_t = snex_shaper4_t<NV>;

template <int NV>
using soft_bypass11_t_ = container::chain<parameter::empty, 
                                          wrap::fix<2, snex_shaper5_t<NV>>>;

template <int NV>
using soft_bypass11_t = bypass::smoothed<20, soft_bypass11_t_<NV>>;

template <int NV>
using soft_bypass3_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, core::gain<NV>>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         filters::svf_eq<NV>, 
                                         soft_bypass10_t<NV>, 
                                         soft_bypass11_t<NV>, 
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
using xfader2_c0_0 = parameter::bypass<soft_bypass11_t<NV>>;

template <int NV>
using xfader2_c0_1 = parameter::bypass<soft_bypass16_t<NV>>;

template <int NV>
using xfader2_c0 = parameter::chain<ranges::Identity, 
                                    xfader2_c0_0<NV>, 
                                    xfader2_c0_1<NV>>;

template <int NV>
using xfader2_c1_0 = parameter::bypass<soft_bypass10_t<NV>>;

template <int NV>
using xfader2_c1_1 = parameter::bypass<soft_bypass9_t<NV>>;

template <int NV>
using xfader2_c1 = parameter::chain<ranges::Identity, 
                                    xfader2_c1_0<NV>, 
                                    xfader2_c1_1<NV>>;

template <int NV>
using xfader2_multimod = parameter::list<xfader2_c0<NV>, xfader2_c1<NV>>;

template <int NV>
using xfader2_t = control::xfader<xfader2_multimod<NV>, faders::switcher>;

template <int NV>
using chain2_t = container::chain<parameter::empty, 
                                  wrap::fix<2, soft_bypass7_t<NV>>, 
                                  soft_bypass3_t<NV>>;

namespace amp_t_parameters
{
// Parameter list for amp_impl::amp_t --------------------------------------------------------------

template <int NV>
using channel = parameter::plain<amp_impl::xfader1_t<NV>, 
                                 0>;
template <int NV>
using inputGainClean = parameter::plain<core::gain<NV>, 0>;
template <int NV> using inputGainDirty = inputGainClean<NV>;
template <int NV> using outputGainClean = inputGainClean<NV>;
template <int NV> using outputGainDirty = inputGainClean<NV>;
template <int NV>
using oversampling = parameter::plain<amp_impl::xfader2_t<NV>, 
                                      0>;
template <int NV>
using cleanLow = parameter::plain<filters::svf_eq<NV>, 2>;
template <int NV> using cleanMid = cleanLow<NV>;
template <int NV> using cleanHigh = cleanLow<NV>;
template <int NV> using cleanPresence = cleanLow<NV>;
template <int NV> using dirtyLow = cleanLow<NV>;
template <int NV> using dirtyMid = cleanLow<NV>;
template <int NV> using dirtyHigh = cleanLow<NV>;
template <int NV> using dirtyPresence = cleanLow<NV>;
template <int NV>
using amp_t_plist = parameter::list<channel<NV>, 
                                    inputGainClean<NV>, 
                                    inputGainDirty<NV>, 
                                    outputGainClean<NV>, 
                                    outputGainDirty<NV>, 
                                    oversampling<NV>, 
                                    cleanLow<NV>, 
                                    cleanMid<NV>, 
                                    cleanHigh<NV>, 
                                    cleanPresence<NV>, 
                                    dirtyLow<NV>, 
                                    dirtyMid<NV>, 
                                    dirtyHigh<NV>, 
                                    dirtyPresence<NV>>;
}

template <int NV>
using amp_t_ = container::chain<amp_t_parameters::amp_t_plist<NV>, 
                                wrap::fix<2, xfader1_t<NV>>, 
                                xfader2_t<NV>, 
                                chain2_t<NV>>;

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
			0x005C, 0x0000, 0x0000, 0x6863, 0x6E61, 0x656E, 0x006C, 0x0000, 
            0x0000, 0x0000, 0x3F80, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 
            0x3F80, 0x005C, 0x0001, 0x0000, 0x6E69, 0x7570, 0x4774, 0x6961, 
            0x436E, 0x656C, 0x6E61, 0x0000, 0xC800, 0x00C2, 0xC800, 0x0042, 
            0x0000, 0x0000, 0x8000, 0xCD3F, 0xCCCC, 0x5C3D, 0x0200, 0x0000, 
            0x6900, 0x706E, 0x7475, 0x6147, 0x6E69, 0x6944, 0x7472, 0x0079, 
            0x0000, 0xC2C8, 0x0000, 0x42C8, 0x0000, 0x0000, 0x0000, 0x3F80, 
            0xCCCD, 0x3DCC, 0x005C, 0x0003, 0x0000, 0x756F, 0x7074, 0x7475, 
            0x6147, 0x6E69, 0x6C43, 0x6165, 0x006E, 0x0000, 0xC2C8, 0x0000, 
            0x42C8, 0x0000, 0x0000, 0x0000, 0x3F80, 0xCCCD, 0x3DCC, 0x005C, 
            0x0004, 0x0000, 0x756F, 0x7074, 0x7475, 0x6147, 0x6E69, 0x6944, 
            0x7472, 0x0079, 0x0000, 0xC2C8, 0x0000, 0x42C8, 0x0000, 0x0000, 
            0x0000, 0x3F80, 0xCCCD, 0x3DCC, 0x005C, 0x0005, 0x0000, 0x766F, 
            0x7265, 0x6173, 0x706D, 0x696C, 0x676E, 0x0000, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x8000, 0x003F, 0x8000, 0x003F, 0x8000, 0x5C3F, 
            0x0600, 0x0000, 0x6300, 0x656C, 0x6E61, 0x6F4C, 0x0077, 0x0000, 
            0xC140, 0x0000, 0x4140, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 
            0x0000, 0x005C, 0x0007, 0x0000, 0x6C63, 0x6165, 0x4D6E, 0x6469, 
            0x0000, 0x4000, 0x00C1, 0x4000, 0x0041, 0x0000, 0x0000, 0x8000, 
            0x003F, 0x0000, 0x5C00, 0x0800, 0x0000, 0x6300, 0x656C, 0x6E61, 
            0x6948, 0x6867, 0x0000, 0x4000, 0x00C1, 0x4000, 0x0041, 0x0000, 
            0x0000, 0x8000, 0x003F, 0x0000, 0x5C00, 0x0900, 0x0000, 0x6300, 
            0x656C, 0x6E61, 0x7250, 0x7365, 0x6E65, 0x6563, 0x0000, 0x4000, 
            0x00C1, 0x4000, 0x0041, 0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 
            0x5C00, 0x0A00, 0x0000, 0x6400, 0x7269, 0x7974, 0x6F4C, 0x0077, 
            0x0000, 0xC140, 0x0000, 0x4140, 0x0000, 0x0000, 0x0000, 0x3F80, 
            0x0000, 0x0000, 0x005C, 0x000B, 0x0000, 0x6964, 0x7472, 0x4D79, 
            0x6469, 0x0000, 0x4000, 0x00C1, 0x4000, 0x0041, 0x0000, 0x0000, 
            0x8000, 0x003F, 0x0000, 0x5C00, 0x0C00, 0x0000, 0x6400, 0x7269, 
            0x7974, 0x6948, 0x6867, 0x0000, 0x4000, 0x00C1, 0x4000, 0x0041, 
            0x0000, 0x0000, 0x8000, 0x003F, 0x0000, 0x5C00, 0x0D00, 0x0000, 
            0x6400, 0x7269, 0x7974, 0x7250, 0x7365, 0x6E65, 0x6563, 0x0000, 
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
		auto& chain2 = this->getT(2);                                       // amp_impl::chain2_t<NV>
		auto& soft_bypass7 = this->getT(2).getT(0);                         // amp_impl::soft_bypass7_t<NV>
		auto& gain8 = this->getT(2).getT(0).getT(0);                        // core::gain<NV>
		auto& svf_eq = this->getT(2).getT(0).getT(1);                       // filters::svf_eq<NV>
		auto& svf_eq3 = this->getT(2).getT(0).getT(2);                      // filters::svf_eq<NV>
		auto& svf_eq2 = this->getT(2).getT(0).getT(3);                      // filters::svf_eq<NV>
		auto& soft_bypass9 = this->getT(2).getT(0).getT(4);                 // amp_impl::soft_bypass9_t<NV>
		auto& oversample4x4 = this->getT(2).getT(0).getT(4).getT(0);        // amp_impl::oversample4x4_t<NV>
		auto& snex_shaper2 = this->getT(2).getT(0).getT(4).getT(0).getT(0); // amp_impl::snex_shaper2_t<NV>
		auto& soft_bypass16 = this->getT(2).getT(0).getT(5);                // amp_impl::soft_bypass16_t<NV>
		auto& snex_shaper6 = this->getT(2).getT(0).getT(5).getT(0);         // amp_impl::snex_shaper6_t<NV>
		auto& svf_eq1 = this->getT(2).getT(0).getT(6);                      // filters::svf_eq<NV>
		auto& gain9 = this->getT(2).getT(0).getT(7);                        // core::gain<NV>
		auto& soft_bypass3 = this->getT(2).getT(1);                         // amp_impl::soft_bypass3_t<NV>
		auto& gain6 = this->getT(2).getT(1).getT(0);                        // core::gain<NV>
		auto& svf_eq7 = this->getT(2).getT(1).getT(1);                      // filters::svf_eq<NV>
		auto& svf_eq6 = this->getT(2).getT(1).getT(2);                      // filters::svf_eq<NV>
		auto& svf_eq5 = this->getT(2).getT(1).getT(3);                      // filters::svf_eq<NV>
		auto& soft_bypass10 = this->getT(2).getT(1).getT(4);                // amp_impl::soft_bypass10_t<NV>
		auto& oversample4x3 = this->getT(2).getT(1).getT(4).getT(0);        // amp_impl::oversample4x3_t<NV>
		auto& snex_shaper4 = this->getT(2).getT(1).getT(4).getT(0).getT(0); // amp_impl::snex_shaper4_t<NV>
		auto& soft_bypass11 = this->getT(2).getT(1).getT(5);                // amp_impl::soft_bypass11_t<NV>
		auto& snex_shaper5 = this->getT(2).getT(1).getT(5).getT(0);         // amp_impl::snex_shaper5_t<NV>
		auto& svf_eq4 = this->getT(2).getT(1).getT(6);                      // filters::svf_eq<NV>
		auto& gain7 = this->getT(2).getT(1).getT(7);                        // core::gain<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, xfader1); // channel -> xfader1::Value
		
		this->getParameterT(1).connectT(0, gain8); // inputGainClean -> gain8::Gain
		
		this->getParameterT(2).connectT(0, gain6); // inputGainDirty -> gain6::Gain
		
		this->getParameterT(3).connectT(0, gain9); // outputGainClean -> gain9::Gain
		
		this->getParameterT(4).connectT(0, gain7); // outputGainDirty -> gain7::Gain
		
		this->getParameterT(5).connectT(0, xfader2); // oversampling -> xfader2::Value
		
		this->getParameterT(6).connectT(0, svf_eq); // cleanLow -> svf_eq::Gain
		
		this->getParameterT(7).connectT(0, svf_eq3); // cleanMid -> svf_eq3::Gain
		
		this->getParameterT(8).connectT(0, svf_eq2); // cleanHigh -> svf_eq2::Gain
		
		this->getParameterT(9).connectT(0, svf_eq1); // cleanPresence -> svf_eq1::Gain
		
		this->getParameterT(10).connectT(0, svf_eq7); // dirtyLow -> svf_eq7::Gain
		
		this->getParameterT(11).connectT(0, svf_eq6); // dirtyMid -> svf_eq6::Gain
		
		this->getParameterT(12).connectT(0, svf_eq5); // dirtyHigh -> svf_eq5::Gain
		
		this->getParameterT(13).connectT(0, svf_eq4); // dirtyPresence -> svf_eq4::Gain
		
		// Modulation Connections ------------------------------------------------------------------
		
		auto& xfader1_p = xfader1.getWrappedObject().getParameter();
		xfader1_p.getParameterT(0).connectT(0, soft_bypass7); // xfader1 -> soft_bypass7::Bypassed
		xfader1_p.getParameterT(1).connectT(0, soft_bypass3); // xfader1 -> soft_bypass3::Bypassed
		auto& xfader2_p = xfader2.getWrappedObject().getParameter();
		xfader2_p.getParameterT(0).connectT(0, soft_bypass11); // xfader2 -> soft_bypass11::Bypassed
		xfader2_p.getParameterT(0).connectT(1, soft_bypass16); // xfader2 -> soft_bypass16::Bypassed
		xfader2_p.getParameterT(1).connectT(0, soft_bypass10); // xfader2 -> soft_bypass10::Bypassed
		xfader2_p.getParameterT(1).connectT(1, soft_bypass9);  // xfader2 -> soft_bypass9::Bypassed
		
		// Default Values --------------------------------------------------------------------------
		
		; // xfader1::Value is automated
		
		; // xfader2::Value is automated
		
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
		
		oversample4x4.setParameterT(0, 0.); // container::chain::FilterType
		
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
		
		oversample4x3.setParameterT(0, 0.); // container::chain::FilterType
		
		svf_eq4.setParameterT(0, 6500.);    // filters::svf_eq::Frequency
		svf_eq4.setParameterT(1, 0.539866); // filters::svf_eq::Q
		;                                   // svf_eq4::Gain is automated
		svf_eq4.setParameterT(3, 0.01);     // filters::svf_eq::Smoothing
		svf_eq4.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq4.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		;                            // gain7::Gain is automated
		gain7.setParameterT(1, 20.); // core::gain::Smoothing
		gain7.setParameterT(2, 0.);  // core::gain::ResetValue
		
		this->setParameterT(0, 0.);
		this->setParameterT(1, 0.);
		this->setParameterT(2, 0.);
		this->setParameterT(3, 0.);
		this->setParameterT(4, 0.);
		this->setParameterT(5, 1.);
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
		
		this->getT(2).getT(0).getT(4).getT(0).getT(0).setExternalData(b, index); // amp_impl::snex_shaper2_t<NV>
		this->getT(2).getT(0).getT(5).getT(0).setExternalData(b, index);         // amp_impl::snex_shaper6_t<NV>
		this->getT(2).getT(1).getT(4).getT(0).getT(0).setExternalData(b, index); // amp_impl::snex_shaper4_t<NV>
		this->getT(2).getT(1).getT(5).getT(0).setExternalData(b, index);         // amp_impl::snex_shaper5_t<NV>
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


