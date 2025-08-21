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
using snex_shaper1_t = wrap::no_data<core::snex_shaper<clean_channel<NV>>>;

template <int NV>
using oversample4x1_t_ = container::chain<parameter::empty, 
                                          wrap::fix<2, snex_shaper1_t<NV>>>;

template <int NV>
using oversample4x1_t = wrap::oversample<4, oversample4x1_t_<NV>>;

template <int NV>
using soft_bypass_t_ = container::chain<parameter::empty, 
                                        wrap::fix<2, core::gain<NV>>, 
                                        oversample4x1_t<NV>, 
                                        core::gain<NV>, 
                                        core::gain<NV>>;

template <int NV>
using soft_bypass_t = bypass::smoothed<20, soft_bypass_t_<NV>>;
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
using snex_shaper3_t = wrap::no_data<core::snex_shaper<dirty_channel<NV>>>;

template <int NV>
using oversample4x2_t_ = container::chain<parameter::empty, 
                                          wrap::fix<2, snex_shaper3_t<NV>>>;

template <int NV>
using oversample4x2_t = wrap::oversample<4, oversample4x2_t_<NV>>;

template <int NV>
using soft_bypass1_t_ = container::chain<parameter::empty, 
                                         wrap::fix<2, core::gain<NV>>, 
                                         oversample4x2_t<NV>, 
                                         core::gain<NV>, 
                                         core::gain<NV>>;

template <int NV>
using soft_bypass1_t = bypass::smoothed<20, soft_bypass1_t_<NV>>;

template <int NV>
using split1_t = container::split<parameter::empty, 
                                  wrap::fix<2, soft_bypass_t<NV>>, 
                                  soft_bypass1_t<NV>>;

namespace amp_t_parameters
{
// Parameter list for amp_impl::amp_t --------------------------------------------------------------

DECLARE_PARAMETER_RANGE(inputGainClean_InputRange, 
                        -72., 
                        72.);
DECLARE_PARAMETER_RANGE_STEP(inputGainClean_0Range, 
                             -72., 
                             72., 
                             0.1);

template <int NV>
using inputGainClean_0 = parameter::from0To1<core::gain<NV>, 
                                             0, 
                                             inputGainClean_0Range>;

template <int NV>
using inputGainClean = parameter::chain<inputGainClean_InputRange, 
                                        inputGainClean_0<NV>>;

DECLARE_PARAMETER_RANGE(inputGainDirty_InputRange, 
                        -72., 
                        72.);
template <int NV> using inputGainDirty_0 = inputGainClean_0<NV>;

template <int NV>
using inputGainDirty = parameter::chain<inputGainDirty_InputRange, 
                                        inputGainDirty_0<NV>>;

DECLARE_PARAMETER_RANGE(outputGainClean_InputRange, 
                        -72., 
                        72.);
template <int NV> using outputGainClean_0 = inputGainClean_0<NV>;

template <int NV>
using outputGainClean = parameter::chain<outputGainClean_InputRange, 
                                         outputGainClean_0<NV>>;

DECLARE_PARAMETER_RANGE(outputGainDirty_InputRange, 
                        -72., 
                        72.);
template <int NV> using outputGainDirty_0 = inputGainClean_0<NV>;

template <int NV>
using outputGainDirty = parameter::chain<outputGainDirty_InputRange, 
                                         outputGainDirty_0<NV>>;

template <int NV>
using channel = parameter::plain<amp_impl::xfader_t<NV>, 0>;
template <int NV>
using amp_t_plist = parameter::list<channel<NV>, 
                                    inputGainClean<NV>, 
                                    inputGainDirty<NV>, 
                                    outputGainClean<NV>, 
                                    outputGainDirty<NV>>;
}

template <int NV>
using amp_t_ = container::chain<amp_t_parameters::amp_t_plist<NV>, 
                                wrap::fix<2, xfader_t<NV>>, 
                                split1_t<NV>>;

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
		SNEX_METADATA_ENCODED_PARAMETERS(102)
		{
			0x005C, 0x0000, 0x0000, 0x6863, 0x6E61, 0x656E, 0x006C, 0x0000, 
            0x0000, 0x0000, 0x3F80, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 
            0x3F80, 0x005C, 0x0001, 0x0000, 0x6E69, 0x7570, 0x4774, 0x6961, 
            0x436E, 0x656C, 0x6E61, 0x0000, 0x9000, 0x00C2, 0x9000, 0xBC42, 
            0x1374, 0x00C0, 0x8000, 0x003F, 0x0000, 0x5C00, 0x0200, 0x0000, 
            0x6900, 0x706E, 0x7475, 0x6147, 0x6E69, 0x6944, 0x7472, 0x0079, 
            0x0000, 0xC290, 0x0000, 0x4290, 0x0000, 0x4290, 0x0000, 0x3F80, 
            0x0000, 0x0000, 0x005C, 0x0003, 0x0000, 0x756F, 0x7074, 0x7475, 
            0x6147, 0x6E69, 0x6C43, 0x6165, 0x006E, 0x0000, 0xC290, 0x0000, 
            0x4290, 0x51EC, 0x40B8, 0x0000, 0x3F80, 0x0000, 0x0000, 0x005C, 
            0x0004, 0x0000, 0x756F, 0x7074, 0x7475, 0x6147, 0x6E69, 0x6944, 
            0x7472, 0x0079, 0x0000, 0xC290, 0x0000, 0x4290, 0xF7CF, 0xC1D3, 
            0x0000, 0x3F80, 0x0000, 0x0000, 0x0000, 0x0000
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
		
		auto& xfader = this->getT(0);                               // amp_impl::xfader_t<NV>
		auto& split1 = this->getT(1);                               // amp_impl::split1_t<NV>
		auto& soft_bypass = this->getT(1).getT(0);                  // amp_impl::soft_bypass_t<NV>
		auto& gain2 = this->getT(1).getT(0).getT(0);                // core::gain<NV>
		auto& oversample4x1 = this->getT(1).getT(0).getT(1);        // amp_impl::oversample4x1_t<NV>
		auto& snex_shaper1 = this->getT(1).getT(0).getT(1).getT(0); // amp_impl::snex_shaper1_t<NV>
		auto& gain3 = this->getT(1).getT(0).getT(2);                // core::gain<NV>
		auto& gain = this->getT(1).getT(0).getT(3);                 // core::gain<NV>
		auto& soft_bypass1 = this->getT(1).getT(1);                 // amp_impl::soft_bypass1_t<NV>
		auto& gain4 = this->getT(1).getT(1).getT(0);                // core::gain<NV>
		auto& oversample4x2 = this->getT(1).getT(1).getT(1);        // amp_impl::oversample4x2_t<NV>
		auto& snex_shaper3 = this->getT(1).getT(1).getT(1).getT(0); // amp_impl::snex_shaper3_t<NV>
		auto& gain5 = this->getT(1).getT(1).getT(2);                // core::gain<NV>
		auto& gain1 = this->getT(1).getT(1).getT(3);                // core::gain<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, xfader); // channel -> xfader::Value
		
		this->getParameterT(1).connectT(0, gain2); // inputGainClean -> gain2::Gain
		
		this->getParameterT(2).connectT(0, gain4); // inputGainDirty -> gain4::Gain
		
		this->getParameterT(3).connectT(0, gain3); // outputGainClean -> gain3::Gain
		
		this->getParameterT(4).connectT(0, gain5); // outputGainDirty -> gain5::Gain
		
		// Modulation Connections ------------------------------------------------------------------
		
		auto& xfader_p = xfader.getWrappedObject().getParameter();
		xfader_p.getParameterT(0).connectT(0, gain);  // xfader -> gain::Gain
		xfader_p.getParameterT(1).connectT(0, gain1); // xfader -> gain1::Gain
		
		// Default Values --------------------------------------------------------------------------
		
		; // xfader::Value is automated
		
		;                            // gain2::Gain is automated
		gain2.setParameterT(1, 20.); // core::gain::Smoothing
		gain2.setParameterT(2, 0.);  // core::gain::ResetValue
		
		oversample4x1.setParameterT(0, 0.); // container::chain::FilterType
		
		;                            // gain3::Gain is automated
		gain3.setParameterT(1, 20.); // core::gain::Smoothing
		gain3.setParameterT(2, 0.);  // core::gain::ResetValue
		
		;                           // gain::Gain is automated
		gain.setParameterT(1, 20.); // core::gain::Smoothing
		gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		;                            // gain4::Gain is automated
		gain4.setParameterT(1, 20.); // core::gain::Smoothing
		gain4.setParameterT(2, 0.);  // core::gain::ResetValue
		
		oversample4x2.setParameterT(0, 0.); // container::chain::FilterType
		
		;                            // gain5::Gain is automated
		gain5.setParameterT(1, 20.); // core::gain::Smoothing
		gain5.setParameterT(2, 0.);  // core::gain::ResetValue
		
		;                            // gain1::Gain is automated
		gain1.setParameterT(1, 20.); // core::gain::Smoothing
		gain1.setParameterT(2, 0.);  // core::gain::ResetValue
		
		this->setParameterT(0, 0.);
		this->setParameterT(1, -2.304);
		this->setParameterT(2, 72.);
		this->setParameterT(3, 5.76);
		this->setParameterT(4, -26.);
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
		
		this->getT(1).getT(0).getT(1).getT(0).setExternalData(b, index); // amp_impl::snex_shaper1_t<NV>
		this->getT(1).getT(1).getT(1).getT(0).setExternalData(b, index); // amp_impl::snex_shaper3_t<NV>
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


