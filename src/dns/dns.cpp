#include "plugin.h"
#include "bot.h"
#include "command.h"
#include "../ip.h"

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

COMMAND(dns)
{
    if(info->in.size() == 0)
	{
		info->error("Usage: dns [hosts]");
	}

	addrinfo *r, *rp;
	addrinfo hints = {0};
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	while(info->in.size() != 0)
    {
    	std::string s = info->pop()->to_string();

		int res = getaddrinfo(s.c_str(), "80", &hints, &r);
		if(res != 0)
		{
			info->error("getaddrinfo returned errno " + std::to_string(errno));
		}

		std::vector<CommandData*> out;

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
				delete[] addr;
				freeaddrinfo(r);

				info->error("inet_ntop returned errno " + std::to_string(errno));
				// continue;
			}

			out.push_back(new IPData(addr));

			delete[] addr;
		}

		info->next->in.push_back(new PairData(new StringData(s), new ListData(out)));
    }

	freeaddrinfo(r);
}
END_COMMAND


void DNSPlugin::init(PluginHost *h)
{
    Bot *b = (Bot*)h;
    CommandData::add_type("ip", new IPType());

    REGISTER_COMMAND(b, dns);

    bot = b;
}

void DNSPlugin::deinit(PluginHost *h)
{
	REMOVE_COMMAND(bot, dns);
}
