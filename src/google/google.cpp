/*#include <curlpp/cURLpp.hpp>

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

std::string request(std::string url)
{
	// magic
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

using namespace curlpp::options;
void GooglePlugin::google(Event *e)
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
	if(customid == "" || k = "") return true;

	
	std::string resp = request("https://www.googleapis.com/customsearch/v1?q=" + q + "&key=" + k + "&cx=" + customid);

	bot->conn->send_privmsg(ev->target, resp);

	return true;
}
*/