#include "plugin.h"
#include "bot.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class DNSPlugin : public Plugin
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
    return new DNSPlugin();
}

std::string DNSPlugin::name()
{
    return "dns";
}

void DNSPlugin::init(PluginHost *h)
{
    Bot *b = (Bot*)h;

    using namespace std::placeholders;
    b->add_handler("command/dns", "dns", std::bind(&DNSPlugin::lookup, this, _1));

    bot = b;
}

void DNSPlugin::deinit(PluginHost *h)
{
    bot->remove_handler("command/dns", "dns");
}

bool DNSPlugin::lookup(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);
	
    if(ev->params.size() == 0)
	{
		bot->conn->send_privmsg(ev->target, "Usage: dns [hosts]");
		return true;
	}

	std::string out;

	addrinfo *r, *rp;
	addrinfo hints = {0};
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	for(auto s: ev->params)
    {
		int res = getaddrinfo(s.c_str(), "80", &hints, &r);
		if(res != 0)
		{
			bot->conn->send_privmsg(ev->target, "getaddrinfo returned errno " + std::to_string(errno));
			continue;
		}

		out += s + ": ";

		for(rp = r; rp != NULL; rp = rp->ai_next)
		{
			sockaddr *sa = rp->ai_addr;
			char *addr = new char[INET6_ADDRSTRLEN+1];
			void *v = NULL;

			if(sa->sa_family == AF_INET)
			{
				v = &(((sockaddr_in*)sa)->sin_addr);
			}
			else if(sa->sa_family == AF_INET6)
			{
				v = &(((sockaddr_in6*)sa)->sin6_addr);
			}
			else
			{
				continue;
			}

			if(inet_ntop(sa->sa_family, v, addr, INET6_ADDRSTRLEN+1) == NULL)
			{
				bot->conn->send_privmsg(ev->target, "inet_ntop returned errno " + std::to_string(errno));
				continue;
			}

			out += addr;
			out += ", ";
			delete[] addr;
		}
		out = out.substr(0, out.length()-2);

		out += " |";
    }

	out = out.substr(0, out.length()-2);

	freeaddrinfo(r);

	bot->conn->send_privmsg(ev->target, out);
	
	return true;
}
