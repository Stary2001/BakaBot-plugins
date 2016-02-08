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

COMMAND(f)
{
	std::string usage = "Usage: f [name]";
	if(info->in.size() == 0)
	{
		//todo: error
		bot->conn->send_privmsg(info->target, usage);
		return;
	}

	std::string n = info->pop()->to_string();

	std::shared_ptr<ConfigNode> v = bot->config->get("factoids." + n);
	if(v->type() != NodeType::Null)
	{

		info->next->in.push_back(new StringData(v->as_map()["value"].string));
	}
	else
	{
		//todo: error
		bot->conn->send_privmsg(info->target, "Factoid '" + n + "' not found!");
	}

	return;
}
END_COMMAND

COMMAND(setf)
{
	std::string usage = "Usage: setf [name] \"[value]\"";
	if(info->in.size() != 2)
	{
		//todo: error
		bot->conn->send_privmsg(info->target, usage);
		return;
	}

	std::string n = info->pop()->to_string();
	std::string value = info->pop()->to_string();


	ConfigValue vv;
	vv.type = NodeType::Map;
	vv.map["value"] = ConfigValue(value);
	vv.map["time"] = ConfigValue(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	vv.map["setter"] = ConfigValue(info->sender->nick);
	bot->config->set("factoids." + n, vv);

	return;
}
END_COMMAND

COMMAND(finfo)
{

	std::string usage = "Usage: finfo [name]";
	if(info->in.size() == 0)
	{
		bot->conn->send_privmsg(info->target, usage);
		return;	
	}

	std::string n = info->pop()->to_string();

	std::shared_ptr<ConfigNode> v = bot->config->get("factoids." + n);
	if(v->type() != NodeType::Null)
	{
		auto m = v->as_map();
		bot->conn->send_privmsg(info->target, bot->conn->antiping(info->target, "Set by " + m["setter"].string + " on " + std::ctime(&m["time"].integer)));
	}
	else
	{
		bot->conn->send_privmsg(info->target, "Factoid '" + n + "' not found!");
	}

	return;
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
