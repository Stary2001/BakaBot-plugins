#include "plugin.h"
#include "bot.h"
#include "command.h"
#include "util.h"

class FunPlugin : public Plugin
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
    return new FunPlugin();
}

std::string FunPlugin::name()
{
    return "fun";
}

COMMAND(choose)
{
	if(info->in.size() == 0)
	{
		info->error("Usage: choose [choice] or [choice] or ...");
	}

	std::string s;
	while(info->in.size() != 0)
	{
		s.append(info->pop()->to_string());
		s.append(" ");
	}
	s = s.substr(0, s.length()-1);

	auto choices = util::split(s, " or ");
	if(choices.size() < 2)
	{
		info->error("Usage: choose [choice] or [choice] or ...");
	}

	int i = util::rand(0, choices.size()-1);
	info->next->in.push_back(new StringData(choices[i]));
}
END_COMMAND

void FunPlugin::init(PluginHost *h)
{
    Bot *b = (Bot*)h;

    REGISTER_COMMAND(b, choose);

    bot = b;
}

void FunPlugin::deinit(PluginHost *h)
{
	REMOVE_COMMAND(bot, choose);
}
