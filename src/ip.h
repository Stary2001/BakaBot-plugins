class IPData : public Data
{
	friend class IPType;

public:
	IPData(std::string s) : Data(Data::get_type("ip")), str(s)
	{}
private:
	std::string str;
};

class IPType : public DataType
{
	virtual std::string to_string(const Data* d)
	{
		if(!d->is_type(this)) { return ""; }
		return ((IPData*)d)->str;
	}

	virtual Data* from_string(std::string s)
	{
		return new IPData(s);
	}

	virtual std::vector<const Data*> select(Data *d, std::string t)
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