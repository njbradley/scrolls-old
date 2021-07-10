#ifndef PLUGINS_PREDEF
#define PLUGINS_PREDEF

#include "classes.h"

#ifdef _WIN32
#include <libloaderapi.h>
#endif

class Plugin { public:
	short id;
	string name;
	vector<BlockData*> blocks;
	
	PluginManager* manager;
	
#ifdef _WIN32
	HMODULE handle;
#else
	void* handle;
#endif
	
	Plugin(string newname, vector<BlockData*> newblocks);
	Plugin(string newname);
	virtual ~Plugin();
	
	virtual void init();
	virtual void link();
	virtual void close();
};

class PluginManager { public:
	vector<Plugin*> plugins;
	
	PluginManager();
	~PluginManager();
	void load_plugin(string path);
	void close_plugin(Plugin* plugin);
	Plugin* get_plugin(short id);
	Plugin* get_plugin(string name);
};

#endif
