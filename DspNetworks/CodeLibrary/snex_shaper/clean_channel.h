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
