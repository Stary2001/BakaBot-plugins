#include <json/reader.h>
#include <json/value.h>
#include "plugin.h"
#include "bot.h"
#include "command.h"
#include "util.h"

class GooglePlugin : public Plugin
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
    return new GooglePlugin();
}

std::string GooglePlugin::name()
{
    return "google";
}

size_t writefunc(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	std::string *d = (std::string*)userdata;
	d->append(ptr, size * nmemb);
	return size * nmemb; 
}

//todo: fixup

COMMAND(google)
{
	if(info->in.size() == 0)
	{
		//todo: error
		bot->conn->send_privmsg(info->target, "Pass something, at least..");
		return;
	}

	std::string q = info->pop()->to_string();

	std::string customid = bot->config->get("google.cx")->as_string();
	std::string k = bot->config->get("google.apikey")->as_string();
	if(customid == "" || k == "") return;
	
	std::string resp = util::http_request("https://www.googleapis.com/customsearch/v1?", {{"q", q}, {"key", k}, {"cx", customid}});
	if(resp == "")
	{
		//todo: error
		bot->conn->send_privmsg(info->target, "Something went wrong!");
		return;
	}

	Json::Value root;
	Json::Reader r;
	if(r.parse(resp, root))
	{
		Json::Value &v = root["items"];
		if(v.size() != 0)
		{
			Json::Value &res = v[0];
			// todo: bleh.
			bot->conn->send_privmsg(info->target, res["title"].asString() + " | " + res["link"].asString());
		}
		else
		{
			// todo: error
			bot->conn->send_privmsg(info->target, "No results!");
		}
	}
	else
	{
		// todo: error
		bot->conn->send_privmsg(info->target, "Google returned invalid JSON!");
	}
}
END_COMMAND


void GooglePlugin::init(PluginHost *h)
{
    Bot *b = (Bot*)h;

    REGISTER_COMMAND(b, google);
    bot = b;
}

void GooglePlugin::deinit(PluginHost *h)
{
    REMOVE_COMMAND(bot, google);
}

