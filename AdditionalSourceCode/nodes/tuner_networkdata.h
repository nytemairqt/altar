namespace project
{

struct tuner_networkdata: public scriptnode::dll::InterpretedNetworkData
{
	String getId() const override
	{
		return "tuner";
	}
	bool isModNode() const override
	{
		return false;
	}
	String getNetworkData() const override
	{
		return "225.nT6K8CVt.zqA.H8hlHBPKNM.L3lTflr.LNgI6SvaD9EUAk+yXKKJJvVzZ5j.aoe.++fHEjfDue.kKnKdXw7nMy4sWrUAGPkT3PpbURg3cE5Q1fsT.3dx86RcixvxOXAWg.IdK4Zr1JjjSFQVGFERtbEh3tCs0xmHDNakNMIVYqrYaCZvcmNYDwa2jIf2UG6xcWkTIEaWMVabVYPMFv8EJjbuAdiz5RvEf.jQD55AIiORZ.WsQQt2iYR7YuBiqGhZ2g4RKtLb2f8Eg2sQ83fOrHAAbE9j.VggxyVvHv5HqWd.";
	}
};
}

