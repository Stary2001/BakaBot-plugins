class IPData : public CommandData
{
	friend class IPType;

public:
	IPData(std::string s) : CommandData(CommandData::get_type("ip")), str(s)
	{}
private:
	std::string str;
};

class IPType : public CommandDataType
{
	virtual std::string to_string(const CommandData* d)
	{
		if(!d->is_type(this)) { return ""; }
		return ((IPData*)d)->str;
	}

	virtual CommandData* from_string(std::string s)
	{
		return new IPData(s);
	}

	virtual std::vector<const CommandData*> select(CommandData *d, std::string t)
	{
		if(t != "ip") 
		{
			return {};
		}
		else
		{
			return {d};
		}
	}
};