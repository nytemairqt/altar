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

namespace cabDesignerRecorder_impl
{
// ==============================| Node & Parameter type declarations |==============================

using oscilloscope_t = wrap::data<analyse::oscilloscope, 
                                  data::external::displaybuffer<0>>;

using soft_bypass_t_ = container::chain<parameter::empty, 
                                        wrap::fix<2, oscilloscope_t>>;

using soft_bypass_t = bypass::smoothed<20, soft_bypass_t_>;

namespace cabDesignerRecorder_t_parameters
{
using Record = parameter::bypass<cabDesignerRecorder_impl::soft_bypass_t>;

}

template <int NV>
using cabDesignerRecorder_t_ = container::chain<cabDesignerRecorder_t_parameters::Record, 
                                                wrap::fix<2, soft_bypass_t>, 
                                                math::clear<NV>>;

// =================================| Root node initialiser class |=================================

template <int NV> struct instance: public cabDesignerRecorder_impl::cabDesignerRecorder_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 1;
		
		SNEX_METADATA_ID(cabDesignerRecorder);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(18)
		{
			0x005C, 0x0000, 0x0000, 0x6552, 0x6F63, 0x6472, 0x0000, 0x0000, 
            0x0000, 0x0000, 0x3F80, 0x0000, 0x0000, 0x0000, 0x3F80, 0x0000, 
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
		
		auto& soft_bypass = this->getT(0);          // cabDesignerRecorder_impl::soft_bypass_t
		auto& oscilloscope = this->getT(0).getT(0); // cabDesignerRecorder_impl::oscilloscope_t
		auto& clear = this->getT(1);                // math::clear<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		this->getParameterT(0).connectT(0, soft_bypass); // Record -> soft_bypass::Bypassed
		
		// Default Values --------------------------------------------------------------------------
		
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
		
		this->getT(0).getT(0).setExternalData(b, index); // cabDesignerRecorder_impl::oscilloscope_t
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
using cabDesignerRecorder = wrap::node<cabDesignerRecorder_impl::instance<NV>>;
}


