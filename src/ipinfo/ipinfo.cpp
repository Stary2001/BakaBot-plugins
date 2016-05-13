#include <curl/curl.h>
#include "plugin.h"
#include "bot.h"
#include "command.h"
#include "../ip.h"
#include "util.h"

class IPInfoPlugin : public Plugin
{
public:
    virtual void init(PluginHost *h);
    virtual void deinit(PluginHost *h);
    virtual std::string name();
private:
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

COMMAND(ipinfo)
{
	if(info->in.size() == 0)
	{
		info->error("Usage: ipinfo [ips]");
	}

	Data *vv = info->pop();

	std::vector<const Data*> v = vv->select("ip");

	for(auto ip: v)
	{
		std::string resp = util::http_request("http://ipinfo.io/" + ip->to_string() + "/org");

		if(resp != "")
		{
			info->next->in.push_back(new StringData(resp));
		}
		else
		{
			info->error("HTTP request failed!");
		}
	}
}
END_COMMAND

void IPInfoPlugin::init(PluginHost *h)
{
	Data::add_type("ip", new IPType());

    Bot *b = (Bot*)h;

    REGISTER_COMMAND(b, ipinfo);

    bot = b;
}

void IPInfoPlugin::deinit(PluginHost *h)
{
	REMOVE_COMMAND(bot, ipinfo);
}
