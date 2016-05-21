#include "plugin.h"
#include "events.h"
#include "bot.h"
#include "command.h"
#include "util.h"
#include <chrono>

class FactoidPlugin : public Plugin
{
public:
    virtual void init(PluginHost *h);
    virtual void deinit(PluginHost *h);
    virtual std::string name();

    Bot *bot;
};

extern "C" Plugin* plugin_init(PluginHost *h)
{
	return new FactoidPlugin();
}

std::string FactoidPlugin::name()
{
	return "factoid";
}

COMMAND(f, CommandFlags::None)
{
	std::string usage = "Usage: f [name]";
	if(info->in.size() == 0)
	{
		//todo: error
		info->error(usage);
	}

	std::string n = info->pop()->to_string();

	std::shared_ptr<ConfigNode> v = bot->config->get("factoids." + n);
	if(!v->is("null"))
	{
		info->next->in.push_back(new StringData(v->as_map()["value"]->to_string()));
	}
	else
	{
		info->error("Factoid '" + n + "' not found!");
	}
}
END_COMMAND

COMMAND(setf, CommandFlags::None)
{
	std::string usage = "Usage: setf [name] \"[value]\"";
	if(info->in.size() != 2)
	{
		info->error(usage);
	}

	std::string n = info->pop()->to_string();
	std::string value = info->pop()->to_string();

	std::map<std::string, Data*> m;
	m["value"] = new StringData(value);
	m["time"] = new IntData(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	m["setter"] = new StringData(info->sender->nick);
	Data *d = new MapData(m);
	bot->config->set("factoids." + n, d);
}
END_COMMAND

COMMAND(finfo, CommandFlags::None)
{

	std::string usage = "Usage: finfo [name]";
	if(info->in.size() == 0)
	{
		info->error(usage);	
	}

	std::string n = info->pop()->to_string();

	std::shared_ptr<ConfigNode> v = bot->config->get("factoids." + n);
	if(!v->is("null"))
	{
		auto m = v->as_map();
		long t = ((IntData*)m["time"])->i;

		info->next->in.push_back(new StringData("Set by " + m["setter"]->to_string() + " on " + std::ctime(&t)));
	}
	else
	{
		info->error("Factoid '" + n + "' not found!");
	}
}
END_COMMAND

void FactoidPlugin::init(PluginHost *h)
{
	Bot *b = (Bot*)h;

	REGISTER_COMMAND(b, f);
	REGISTER_COMMAND(b, setf);
	REGISTER_COMMAND(b, finfo);

	bot = b;
}

void FactoidPlugin::deinit(PluginHost *h)
{
	REMOVE_COMMAND(bot, f);
	REMOVE_COMMAND(bot, setf);
	REMOVE_COMMAND(bot, finfo);
}
