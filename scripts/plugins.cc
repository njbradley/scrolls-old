#ifndef PLUGINS
#define PLUGINS

#ifdef _WIN32
#include <libloaderapi.h>
#else
#include <dlfcn.h>
#endif

#include "plugins.h"

#include "cross-platform.h"

Plugin::Plugin(string newname, vector<BlockData*> newblocks): name(newname), blocks(newblocks) {
	
}

Plugin::Plugin(string newname): name(newname) {
	
}

Plugin::~Plugin() {

}

void Plugin::init() {
	
}

void Plugin::link() {
	
}

void Plugin::close() {
	
}



PluginManager::PluginManager() {
	vector<string> paths;
	get_files_folder("resources/plugins", &paths);
	for (string path : paths) {
		load_plugin("resources/plugins/" + path);
	}
}

PluginManager::~PluginManager() {
	for (Plugin* plugin : plugins) {
		close_plugin(plugin);
	}
}


void PluginManager::load_plugin(string path) {
#ifdef _WIN32
	HMODULE handle = LoadLibrary(path.c_str());
	Plugin* (*getplugin)() = (Plugin* (*)()) GetProcAddress(handle, "load_plugin");
#else
	void* handle = dlopen(path.c_str(), RTLD_NOW);
	Plugin* (*getplugin)() = (Plugin* (*)()) dlsym(handle, "load_plugin");
#endif
	Plugin* plugin = getplugin();
	plugin->manager = this;
	plugin->handle = handle;
	plugin->id = plugins.size();
	plugins.push_back(plugin);
	plugin->init();
}

void PluginManager::close_plugin(Plugin* plugin) {
#ifdef _WIN32
	HMODULE handle = plugin->handle;
	delete plugin;
	FreeLibrary(handle);
#else
	void* handle = plugin->handle;
	delete plugin;
	dlclose(handle);
#endif
}

Plugin* PluginManager::get_plugin(short id) {
	return plugins[id];
}

Plugin* PluginManager::get_plugin(string name) {
	for (Plugin* plugin : plugins) {
		if (plugin->name == name) {
			return plugin;
		}
	}
	return nullptr;
}

#endif
