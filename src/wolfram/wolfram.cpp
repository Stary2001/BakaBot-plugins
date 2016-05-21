#include <curl/curl.h>
#include <tinyxml2.h>
#include "plugin.h"
#include "bot.h"
#include "command.h"
#include "util.h"

using namespace tinyxml2;

class WolframPlugin : public Plugin
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
    return new WolframPlugin();
}

std::string WolframPlugin::name()
{
    return "wolfram";
}

COMMAND(wolfram, CommandFlags::OneParam)
{
	if(info->in.size() == 0)
	{
		info->error("Usage: wolfram \"[thing]\"");
	}

	std::string id = bot->config->get("wolfram.appid")->as_string();
	Data *vv = info->pop();

	std::string resp = util::http_request("http://api.wolframalpha.com/v2/query?", {{"input", vv->to_string()}, {"appid", id}, {"format", "plaintext,image"}});

	XMLDocument d;
	XMLError e = d.Parse(resp.c_str());

	if(e == XML_NO_ERROR)
	{
		XMLElement *root = d.RootElement();
		std::string res = root->Attribute("success");
		if(res == "false")
		{
			info->error("Query failed! (no results?)");
		}

		XMLElement *e = root->FirstChildElement("pod");

		while(e != nullptr)
		{
			std::string type = e->Attribute("id");
			if(type == "Result" || type.find("Plot") != std::string::npos)
			{
				XMLElement *sub = e->FirstChildElement("subpod");
				if(sub)
				{
					XMLElement *text = sub->FirstChildElement("plaintext");
					if(type != "Result" || text == nullptr)
					{
						XMLElement *img = sub->FirstChildElement("img");
						if(img)
						{
							info->next->in.push_back(new StringData(img->Attribute("src")));
							break;
						}
						info->error("wtf");
					}

					if(text)
					{
						info->next->in.push_back(new StringData(text->GetText()));
						break;
					}
				}

				info->error("wtf");
			}

			e = e->NextSiblingElement("pod");
		}

		if(e == nullptr)
		{
			info->error("Wolfram Alpha did not return a Result pod!");
		}
	}
	else
	{
		info->error("Failed to parse Wolfram Alpha API response!");
	}
}
END_COMMAND

void WolframPlugin::init(PluginHost *h)
{
    Bot *b = (Bot*)h;

    REGISTER_COMMAND(b, wolfram);

    bot = b;
}

void WolframPlugin::deinit(PluginHost *h)
{
	REMOVE_COMMAND(bot, wolfram);
}
