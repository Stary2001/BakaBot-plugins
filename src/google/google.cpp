#include <json/reader.h>
#include <json/value.h>
#include "plugin.h"
#include "bot.h"
#include "util.h"

class GooglePlugin : public Plugin
{
public:
    virtual void init(PluginHost *h);
    virtual void deinit(PluginHost *h);
    virtual std::string name();
private:
	bool google(Event *e);
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


void GooglePlugin::init(PluginHost *h)
{
    Bot *b = (Bot*)h;

    using namespace std::placeholders;
    b->add_handler("command/google", "google", std::bind(&GooglePlugin::google, this, _1));
    bot = b;
}

void GooglePlugin::deinit(PluginHost *h)
{
    bot->remove_handler("command/google", "google");
}

bool GooglePlugin::google(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);

	std::string q;
	for(auto v: ev->params)
	{
		q += v + " ";
	}
	q = q.substr(0, q.length() - 1);

	std::string customid = bot->config->get("google.cx")->as_string();
	std::string k = bot->config->get("google.apikey")->as_string();
	if(customid == "" || k == "") return true;
	
	std::string resp = util::http_request("https://www.googleapis.com/customsearch/v1?", {{"q", q}, {"key", k}, {"cx", customid}});
	if(resp == "")
	{
		bot->conn->send_privmsg(ev->target, "Something went wrong!");
		return true;
	}

	Json::Value root;
	Json::Reader r;
	if(r.parse(resp, root))
	{
		Json::Value &v = root["items"];
		if(v.size() != 0)
		{
			Json::Value &res = v[0];
			bot->conn->send_privmsg(ev->target, res["title"].asString() + " | " + res["link"].asString());
		}
		else
		{
			bot->conn->send_privmsg(ev->target, "No results!");
		}
	}
	else
	{
		bot->conn->send_privmsg(ev->target, "Google returned invalid JSON!");
	}

	return true;
}
