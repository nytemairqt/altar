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
		return "214.nT6K8CVo.TlA.H1xjDBPqsM.MxnHsIKpz.WXnOgAzGxsuiyd5wwvq5yDWYiszOv+M3QgCj7GVKNZubd6EaUn.ITBGR0SnDh2UGMjKXKEdG49cnlQWXYGjfmHfj2PtFqs1H4TwisYQajJOgHtyPasz44fyVYCChU1Ja1xfEbuoSEO71LYB3ckwpb2SnDJw1Ui0EmUliFA3NBJfbmAdezpRXAHfI7f8TK9Hw.tYihbaOxTa6nCQU2goRKTFtyfcEg2rQ8evGVjf.tBeR.q7P4eKXjYcjkJO.";
	}
};
}

