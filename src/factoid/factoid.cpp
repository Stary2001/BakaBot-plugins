#include "plugin.h"
#include "events.h"
#include "bot.h"
#include <chrono>

class FactoidPlugin : public Plugin
{
public:
    virtual void init(PluginHost *h);
    virtual void deinit(PluginHost *h);
    virtual std::string name();

private:
    bool factoid(Event *e);
    bool factoid_info(Event *e);
	bool factoid_set(Event *e);

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

void FactoidPlugin::init(PluginHost *h)
{
	Bot *b = (Bot*)h;

	using namespace std::placeholders;
	b->add_handler("command/f", "factoid", std::bind(&FactoidPlugin::factoid, this, _1));
	b->add_handler("command/setf", "factoid", std::bind(&FactoidPlugin::factoid_set, this, _1));
	b->add_handler("command/finfo", "factoid", std::bind(&FactoidPlugin::factoid_info, this, _1));

	bot = b;
}

void FactoidPlugin::deinit(PluginHost *h)
{
	bot->remove_handler("command/f", "factoid");
	bot->remove_handler("command/setf", "factoid");
	bot->remove_handler("command/finfo", "factoid");
}

bool FactoidPlugin::factoid(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);
	std::string usage = "Usage: f [name]";
	if(ev->params.size() == 0)
	{
		bot->conn->send_privmsg(ev->target, usage);
		return true;
	}

	std::shared_ptr<ConfigNode> v = bot->config->get("factoids." + ev->params[0]);
	if(v->type() != NodeType::Null)
	{
		bot->conn->send_privmsg(ev->target, v->as_map()["value"].string);
	}
	else
	{
		bot->conn->send_privmsg(ev->target, "Factoid '" + ev->params[0] + "' not found!");
	}

	return true;
}

bool FactoidPlugin::factoid_set(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);
	std::string usage = "Usage: setf [name] [value]";
	if(ev->params.size() < 2)
	{
		bot->conn->send_privmsg(ev->target, usage);
		return true;
	}

	std::string value;
	std::string n = ev->params[0];
	ev->params.erase(ev->params.begin());

	for(auto v: ev->params)
	{
		value += v + " ";
	}
	value = value.substr(0, value.length() - 1);
	ConfigValue vv;
	vv.type = NodeType::Map;
	vv.map["value"] = ConfigValue(value);
	vv.map["time"] = ConfigValue(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	vv.map["setter"] = ConfigValue(ev->sender->nick);
	bot->config->set("factoids." + n, vv);
}

bool FactoidPlugin::factoid_info(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);
	
	std::string usage = "Usage: finfo [name]";
	if(ev->params.size() == 0)
	{
		bot->conn->send_privmsg(ev->target, usage);
		return true;	
	}
	std::shared_ptr<ConfigNode> v = bot->config->get("factoids." + ev->params[0]);
	if(v->type() != NodeType::Null)
	{
		auto m = v->as_map();
		bot->conn->send_privmsg(ev->target, "Set by " + m["setter"].string + " on " + std::ctime(&m["time"].integer));
	}
	else
	{
		bot->conn->send_privmsg(ev->target, "Factoid '" + ev->params[0] + "' not found!");
	}

	return true;
}