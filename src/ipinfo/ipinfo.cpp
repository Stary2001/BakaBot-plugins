#include <curl/curl.h>
#include <json/reader.h>
#include <json/value.h>
#include "plugin.h"
#include "bot.h"
#include "util.h"

class IPInfoPlugin : public Plugin
{
public:
    virtual void init(PluginHost *h);
    virtual void deinit(PluginHost *h);
    virtual std::string name();
private:
	bool lookup(Event *e);
	Bot *bot;		
};

extern "C" Plugin* plugin_init(PluginHost *h)
{
    return new IPInfoPlugin();
}

std::string IPInfoPlugin::name()
{
    return "ipinfo";
}

void IPInfoPlugin::init(PluginHost *h)
{
    Bot *b = (Bot*)h;

    using namespace std::placeholders;
    b->add_handler("command/ipinfo", "ipinfo", std::bind(&IPInfoPlugin::lookup, this, _1));

    bot = b;
}

void IPInfoPlugin::deinit(PluginHost *h)
{
    bot->remove_handler("command/ipinfo", "ipinfo");
}

bool IPInfoPlugin::lookup(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);

	if(ev->params.size() == 0)
	{
		bot->conn->send_privmsg(ev->target, "Usage: dns [hosts]");
		return true;
	}

	std::string resp = util::http_request("http://ipinfo.io/" + ev->params[0] + "/org");

	if(resp != "")
	{
		bot->conn->send_privmsg(ev->target, resp);
	}
	else
	{
		bot->conn->send_privmsg(ev->target, "Something went wrong!");
	}

	return true;
}
