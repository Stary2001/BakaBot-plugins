#include <curl/curl.h>
#include <json/reader.h>
#include <json/value.h>
#include "plugin.h"
#include "bot.h"

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


std::string request(std::string url, std::map<std::string, std::string> params)
{
	// magic
	CURL *easy = curl_easy_init();

	bool first = false;
	for(auto pair: params)
	{
		char *q = curl_easy_escape(easy, pair.second.c_str(), pair.second.length());
		if(first)
		{
			first = true;
			url += "?";
		}
		else
		{
			url += "&";
		}
		url += pair.first + "=" + q ;
		curl_free(q);
	}

	curl_easy_setopt(easy, CURLOPT_URL, url.c_str());

	std::string dat;

	curl_easy_setopt(easy, CURLOPT_WRITEDATA, &dat);
	curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, writefunc);

	if(curl_easy_perform(easy) == CURLE_OK)
	{
		curl_easy_cleanup(easy);
		return dat;
	}
	else
	{
		return "{}";
	}
}

void GooglePlugin::init(PluginHost *h)
{
    Bot *b = (Bot*)h;

    using namespace std::placeholders;
    b->add_handler("command/google", "google", std::bind(&GooglePlugin::google, this, _1));

	curl_global_init(CURL_GLOBAL_ALL);

    bot = b;
}

void GooglePlugin::deinit(PluginHost *h)
{
	curl_global_cleanup();

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
	
	std::string resp = request("https://www.googleapis.com/customsearch/v1?", {{"q", q}, {"key", k}, {"cx", customid}});
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
