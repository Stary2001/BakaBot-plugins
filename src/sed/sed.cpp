#include <iostream>
#include <regex>
#include <algorithm>
#include "plugin.h"
#include "events.h"
#include "command.h"
#include "bot.h"

#define SCROLLBACK_SIZE 100


enum class MessageType
{
	Normal,
	Me
};

struct SedMessage
{
	User *sender;
	std::string message;
	MessageType type;
};

class SedPlugin : public Plugin
{
public:
    virtual void init(PluginHost *h);
    virtual void deinit(PluginHost *h);
    virtual std::string name();

private:
    bool msg(Event *e);

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

COMMAND(togglesed, CommandFlags::None)
{
	if(info->target[0] != '#')
		return;

	std::shared_ptr<ConfigNode> v = bot->config->get("sed.disable");

	if(v->is("null"))
	{
		std::vector<Data*> d;
		bot->config->set("sed.disable", new ListData(d));
	}

	DataEquals f(info->target);
	std::vector<Data*>::iterator it = std::find_if(v->as_list().begin(), v->as_list().end(), f);
	bool exists = it != v->as_list().end();

	if(exists)
	{
		v->as_list().erase(it);
		bot->conn->send_privmsg(info->target, "Sed enabled.");
	}
	else if(!exists)
	{
		v->as_list().push_back(new StringData(info->target));
		bot->conn->send_privmsg(info->target, "Sed disabled.");
	}
}
END_COMMAND

bool SedPlugin::msg(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);

	if(ev->target[0] == '#') // to a channel?
	{
		std::shared_ptr<ConfigNode> v = bot->config->get("sed.disable");

		if(v->is("list"))
		{
			DataEquals f(ev->target);
			auto it = std::find_if(v->as_list().begin(), v->as_list().end(), f);
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
				if(end != std::string::npos && ev->message[end - 1] == '\\') 
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
						if(it->type == MessageType::Normal)
						{
							bot->conn->send_privmsg(ev->target, "<" + it->sender->nick + "> " + resp);
						}
						else if(it->type == MessageType::Me)
						{
							bot->conn->send_privmsg(ev->target, "* " + it->sender->nick + " " + resp);
						}
						
						SedMessage m;
						m.sender = it->sender;
						m.message = resp;
						m.type = it->type;
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
			if(ev->message[0] == '\x01' && ev->message[ev->message.length()-1] == '\x01') // ctcp....
			{
				m.message = ev->message.substr(1, ev->message.length()-2);
				size_t p = m.message.find(' ');
				if(p == std::string::npos)
				{
					// malformed ctcp
					return false;
				}
				m.message = m.message.substr(p + 1);
				m.type = MessageType::Me;
			}
			else
			{
				m.message = ev->message;
				m.type = MessageType::Normal;
			}
			scroll.push_back(m);
			if(scroll.size() >= SCROLLBACK_SIZE)
			{
				scroll.erase(scroll.begin());
			}
		}

		
	}
	return false;
}

void SedPlugin::init(PluginHost *h)
{
	Bot *b = (Bot*)h;

	REGISTER_COMMAND(b, togglesed);

	using namespace std::placeholders;
	b->add_handler("irc/privmsg", "sed", std::bind(&SedPlugin::msg, this, _1));

	//scrollback = std::map<Bot *, std::map<std::string, std::vector<std::string>>>();
	scrollback = std::map<std::string, std::vector<SedMessage>>();
	bot = b;
}

void SedPlugin::deinit(PluginHost *h)
{
	REMOVE_COMMAND(bot, togglesed);
	bot->remove_handler("command/sed", "sed");
}