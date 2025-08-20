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
