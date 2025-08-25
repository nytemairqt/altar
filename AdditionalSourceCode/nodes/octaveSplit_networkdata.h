namespace project
{

struct octaveSplit_networkdata: public scriptnode::dll::InterpretedNetworkData
{
	String getId() const override
	{
		return "octaveSplit";
	}
	bool isModNode() const override
	{
		return false;
	}
	String getNetworkData() const override
	{
		return "276.nT6K8CVzATEB.HyyuLBPKtI.CCiRh1hfLnV4jb1c5FQQt.pjRWs5JJd+aTlGHFCk98+nBxOcD3xsFpAqNBpihvrTcEoDsnLVuaQ6BriCdE1w8nwN29zPGPkVQvXf3l4pDBKcQkExUIkp44n1NwwfrHuh7OB4lorvURlIFbrN5A0BN70Mbiip.OlHYJ6xUJ.4c3XbgfHg6X6SSBxN11tWiRP9mdk.A70MqBJxyNukLuJoRJMf7tGKB4IuYMHF1gsadJi7NdjHeCv2CMuDc.BXjngxdvnXMuLnKJ3nWSXETDtkYYZdOH3nIX3x8UoTC7hoCC3hfUkbOWB1TF00D0qPCv7evHA514IBAzW7mrvJQBUNdzn.";
	}
};
}

