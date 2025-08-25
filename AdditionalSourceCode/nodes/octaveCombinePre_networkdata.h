namespace project
{

struct octaveCombinePre_networkdata: public scriptnode::dll::InterpretedNetworkData
{
	String getId() const override
	{
		return "octaveCombinePre";
	}
	bool isModNode() const override
	{
		return false;
	}
	String getNetworkData() const override
	{
		return "280.nT6K8CFyATGB.H4CwPBPKtI..LSxBfqzbIVWFxc9MTJLqD76NB5ddL7JKD4v.wXnzOv+gqGG0PdKKwnF7NNoiPdVTUWfSwFcfk2RwkAtwAmBKo4PhasMhQIXoVhf8Lg6muTHrnKnrO9RsTMOGk1HM1iE4Uj+vG27z9VmLUMzXcz8TF+4ZHVhgp.GlH4os4KEf7RXHbmTIg2XaDj5wM11MGjVP9ndcp7bMzpfh7rDWSleoVpkFP9liAebjyOGtwVXxwvOrMjSYjugiD4c34fn4l.Gf.kvQ44CLJtxKCHmfnaqETB2xtLlm8siQqPfOWsRVMPAFOrfKhqpactjuoLiqIPuBMf0+AiHPAb9jPPtEmJKL0uf1TsIJ.";
	}
};
}

