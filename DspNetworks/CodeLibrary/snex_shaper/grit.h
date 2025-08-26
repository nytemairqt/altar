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
