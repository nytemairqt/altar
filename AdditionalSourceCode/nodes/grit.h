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

namespace grit_impl
{
// ==============================| Node & Parameter type declarations |==============================

using chain_t = container::chain<parameter::empty, 
                                 wrap::fix<2, core::empty>>;
template <int NumVoices> struct grit
{
	SNEX_NODE(grit);
	float pi = 3.1415926536f;
	float bias = .3f; // -0.3 to 0.3
	float drive = 1.0f;
	float x = 0.0f;
	float y= 0.0f;
	float neg = 0.0f;
	// Implement the Waveshaper here...
	float getSample(float input)
	{
		x = input;
		if (x > 0.0f)
		{
			y = Math.sin(x * pi) + x * bias;
		}
		else
		{
			y = Math.atan(x * 3.0f) + 0.0f - (x * bias);
			y = Math.sin(y);
		}
		return y;
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
using snex_shaper_t = wrap::no_data<core::snex_shaper<grit<NV>>>;

template <int NV>
using chain1_t = container::chain<parameter::empty, 
                                  wrap::fix<2, filters::svf<NV>>, 
                                  filters::svf<NV>, 
                                  snex_shaper_t<NV>, 
                                  filters::svf<NV>, 
                                  filters::svf<NV>, 
                                  filters::svf_eq<NV>, 
                                  core::gain<NV>>;

template <int NV>
using split_t = container::split<parameter::empty, 
                                 wrap::fix<2, chain_t>, 
                                 chain1_t<NV>>;

template <int NV>
using soft_bypass_t_ = container::chain<parameter::empty, 
                                        wrap::fix<2, split_t<NV>>>;

template <int NV>
using soft_bypass_t = bypass::smoothed<20, soft_bypass_t_<NV>>;

namespace grit_t_parameters
{
}

template <int NV>
using grit_t_ = container::chain<parameter::plain<core::gain<NV>, 0>, 
                                 wrap::fix<2, soft_bypass_t<NV>>>;

// =================================| Root node initialiser class |=================================

template <int NV> struct instance: public grit_impl::grit_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(grit);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(16)
		{
			0x005C, 0x0000, 0x0000, 0x7247, 0x7469, 0x0000, 0xC800, 0x00C2, 
            0x0000, 0x3300, 0x1333, 0x00C1, 0x8000, 0xCD3F, 0xCCCC, 0x003D
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
		
		auto& soft_bypass = this->getT(0);                         // grit_impl::soft_bypass_t<NV>
		auto& split = this->getT(0).getT(0);                       // grit_impl::split_t<NV>
		auto& chain = this->getT(0).getT(0).getT(0);               // grit_impl::chain_t
		auto& chain1 = this->getT(0).getT(0).getT(1);              // grit_impl::chain1_t<NV>
		auto& svf = this->getT(0).getT(0).getT(1).getT(0);         // filters::svf<NV>
		auto& svf4 = this->getT(0).getT(0).getT(1).getT(1);        // filters::svf<NV>
		auto& snex_shaper = this->getT(0).getT(0).getT(1).getT(2); // grit_impl::snex_shaper_t<NV>
		auto& svf1 = this->getT(0).getT(0).getT(1).getT(3);        // filters::svf<NV>
		auto& svf5 = this->getT(0).getT(0).getT(1).getT(4);        // filters::svf<NV>
		auto& svf_eq = this->getT(0).getT(0).getT(1).getT(5);      // filters::svf_eq<NV>
		auto& gain1 = this->getT(0).getT(0).getT(1).getT(6);       // core::gain<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, gain1); // Grit -> gain1::Gain
		
		// Default Values --------------------------------------------------------------------------
		
		svf.setParameterT(0, 426.102); // filters::svf::Frequency
		svf.setParameterT(1, 1.);      // filters::svf::Q
		svf.setParameterT(2, 0.);      // filters::svf::Gain
		svf.setParameterT(3, 0.01);    // filters::svf::Smoothing
		svf.setParameterT(4, 0.);      // filters::svf::Mode
		svf.setParameterT(5, 1.);      // filters::svf::Enabled
		
		svf4.setParameterT(0, 89.3711); // filters::svf::Frequency
		svf4.setParameterT(1, 1.);      // filters::svf::Q
		svf4.setParameterT(2, 0.);      // filters::svf::Gain
		svf4.setParameterT(3, 0.01);    // filters::svf::Smoothing
		svf4.setParameterT(4, 1.);      // filters::svf::Mode
		svf4.setParameterT(5, 1.);      // filters::svf::Enabled
		
		svf1.setParameterT(0, 313.229); // filters::svf::Frequency
		svf1.setParameterT(1, 0.38711); // filters::svf::Q
		svf1.setParameterT(2, 0.);      // filters::svf::Gain
		svf1.setParameterT(3, 0.01);    // filters::svf::Smoothing
		svf1.setParameterT(4, 0.);      // filters::svf::Mode
		svf1.setParameterT(5, 1.);      // filters::svf::Enabled
		
		svf5.setParameterT(0, 93.919); // filters::svf::Frequency
		svf5.setParameterT(1, 1.);     // filters::svf::Q
		svf5.setParameterT(2, 0.);     // filters::svf::Gain
		svf5.setParameterT(3, 0.01);   // filters::svf::Smoothing
		svf5.setParameterT(4, 1.);     // filters::svf::Mode
		svf5.setParameterT(5, 1.);     // filters::svf::Enabled
		
		svf_eq.setParameterT(0, 243.65);  // filters::svf_eq::Frequency
		svf_eq.setParameterT(1, 1.80211); // filters::svf_eq::Q
		svf_eq.setParameterT(2, -7.776);  // filters::svf_eq::Gain
		svf_eq.setParameterT(3, 0.01);    // filters::svf_eq::Smoothing
		svf_eq.setParameterT(4, 4.);      // filters::svf_eq::Mode
		svf_eq.setParameterT(5, 1.);      // filters::svf_eq::Enabled
		
		;                            // gain1::Gain is automated
		gain1.setParameterT(1, 20.); // core::gain::Smoothing
		gain1.setParameterT(2, 0.);  // core::gain::ResetValue
		
		this->setParameterT(0, -9.2);
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
		
		this->getT(0).getT(0).getT(1).getT(2).setExternalData(b, index); // grit_impl::snex_shaper_t<NV>
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
using grit = wrap::node<grit_impl::instance<NV>>;
}


