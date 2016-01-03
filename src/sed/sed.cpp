#include <iostream>
#include <regex>
#include <algorithm>
#include "plugin.h"
#include "events.h"
#include "bot.h"

#define SCROLLBACK_SIZE 100

struct SedMessage
{
	User *sender;
	std::string message;
};

class SedPlugin : public Plugin
{
public:
    virtual void init(PluginHost *h);
    virtual void deinit(PluginHost *h);
    virtual std::string name();

private:
    bool msg(Event *e);
    bool toggle(Event *e);

    std::map<std::string, std::vector<SedMessage>> scrollback;
    Bot *bot;
};

extern "C" Plugin* plugin_init(PluginHost *h)
{
	return new SedPlugin();
}

std::string SedPlugin::name()
{
	return "sed";
}

void SedPlugin::init(PluginHost *h)
{
	std::cout << "init" << std::endl;
	Bot *b = (Bot*)h;

	using namespace std::placeholders;

	b->add_handler("command/sed", "sed", std::bind(&SedPlugin::toggle, this, _1));
	b->add_handler("irc/message", "sed", std::bind(&SedPlugin::msg, this, _1));

	//scrollback = std::map<Bot *, std::map<std::string, std::vector<std::string>>>();
	scrollback = std::map<std::string, std::vector<SedMessage>>();
	bot = b;
}

void SedPlugin::deinit(PluginHost *h)
{
	std::cout << "deinit" << std::endl;
	bot->remove_handler("irc/message", "sed");
	bot->remove_handler("command/sed", "sed");
}

bool SedPlugin::toggle(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);

	if(ev->target[0] != '#')
		return true;

	std::shared_ptr<ConfigNode> v = bot->config->get("sed.disable");

	if(v->type() == NodeType::Null)
	{
		ConfigValue vv;
		vv.type = NodeType::List;
		bot->config->set("sed.disable", vv);
	}

	std::vector<std::string>::iterator it = std::find(v->as_list().begin(), v->as_list().end(), ev->target);
	bool exists = it != v->as_list().end();
	bool state = true; // whether it should be enabled

	if(ev->params.size() == 0)
	{
		state = exists;
	}
	else
	{
		if(ev->params[0] == "on")
		{
			state = true;
		}
		else if(ev->params[0] == "off")
		{
			state = false;
		}
		else
		{
			bot->conn->send_privmsg(ev->target, "Usage: sed [on|off]");
			return true;
		}
	}

	if(state && exists)
	{
		v->as_list().erase(it);
		bot->conn->send_privmsg(ev->target, "Sed enabled.");
	}
	else if(!state && !exists)
	{
		v->as_list().push_back(ev->target);
		bot->conn->send_privmsg(ev->target, "Sed disabled.");
	}


	return true;
}

bool SedPlugin::msg(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);

	if(ev->target[0] == '#') // to a channel?
	{
		std::shared_ptr<ConfigNode> v = bot->config->get("sed.disable");

		if(v->type() == NodeType::List)
		{
			auto it = std::find(v->as_list().begin(), v->as_list().end(), ev->target);
			if(it != v->as_list().end())
			{
				return false;
			}
		}

		if(scrollback.find(ev->target) == scrollback.end())
		{
			scrollback[ev->target] = std::vector<SedMessage>();
		}

		std::vector<SedMessage> &scroll = scrollback[ev->target];
		
		if(ev->message.substr(0, 2) == "s/")
		{
			std::cout << ev->message << " " << scroll.size() << std::endl;
			size_t beg = 2; 
			size_t middle = ev->message.find('/', beg + 1);

			while(true)
			{
				if(middle == std::string::npos)
				{
					return false;
				}
				else if(ev->message[middle - 1] == '\\')
				{
					middle = ev->message.find('/', middle + 1);
				}
				else
				{
					break;
				}
			}

			size_t end = ev->message.find('/', middle + 1);
			while(true)
			{	
				if(end != std::string::npos && ev->message[middle - 1] == '\\') 
				{
					end = ev->message.find('/', end + 1);
				}
				else
				{
					break;
				}
			}

			if(end == std::string::npos)
			{
				end = ev->message.length();
			}

			std::string regex = ev->message.substr(beg, middle - beg);
			std::string replacement = ev->message.substr(middle + 1, end - middle - 1);
			std::string flags;
			if(end != ev->message.length())
			{
				flags = ev->message.substr(end + 1);
			}
			
			//std::cout << regex << " / " << replacement << " / " << flags << std::endl;

			auto syn_flags = std::regex_constants::ECMAScript;
			if(flags.find("i") != std::string::npos)
			{
				syn_flags |= std::regex_constants::icase;
			}

			try
			{
				std::regex r(regex.c_str(), syn_flags);

				auto match_flags = std::regex_constants::format_default;

				if(flags.find("g") == std::string::npos)
				{
					match_flags |= std::regex_constants::format_first_only;
				}

				auto it = scroll.rbegin();
				for(; it != scroll.rend(); it++)
				{
					if(std::regex_search(it->message, r, match_flags))
					{
						std::string resp = std::regex_replace(it->message, r, replacement, match_flags);
						bot->conn->send_privmsg(ev->target, "<" + it->sender->nick + "> " + resp);
						
						SedMessage m;
						m.sender = it->sender;
						m.message = resp;
						scroll.push_back(m);
						break;
					}
				}
			}
			catch(std::regex_error &e)
			{
				bot->conn->send_privmsg(ev->target, "you broke it! gg!");
			}
		}
		else
		{
			SedMessage m;
			m.sender = ev->sender;
			m.message = ev->message;
			scroll.push_back(m);
			if(scroll.size() >= SCROLLBACK_SIZE)
			{
				scroll.erase(scroll.begin());
			}
		}

		
	}
	return false;
}
