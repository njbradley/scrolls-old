#ifndef PLUGINS
#define PLUGINS

#include "plugins.h"

#include "cross-platform.h"
#include <filesystem>
#include <sys/stat.h>


PluginLib::PluginLib(string path) {
	struct stat info;
	string dllpath = path + "/" + path + DLLSUFFIX;
	dirname = path;
	if (stat(dllpath.c_str(), &info) == 0) {
		handle = LOADLIB(dllpath.c_str());
		if (!handle) {
			cout << "Plugin error: [" << path << "]: cannot open dll" << endl;
		} else {
			
			PluginDef* plugindefs = (PluginDef*) GETADDR(handle, "plugins");
			if (plugindefs == nullptr) {
				cout << "Plugin error: [" << path << "]: cannot find plugins" << endl;
			}
			
			while (plugindefs->basename != nullptr) {
				plugins.push_back(*plugindefs);
				plugindefs ++;
			}
		}
	}
}

PluginLib::~PluginLib() {
	FREELIB(handle);
}




PluginLoader::PluginLoader() {
	cout << "Loading plugins " << endl;
	struct stat info;
	for (std::filesystem::path fspath : std::filesystem::directory_iterator("./")) {
		string path = fspath.filename().string();
		if (stat((path + "/" + path + DLLSUFFIX).c_str(), &info) == 0 or stat((path + "/" + path + ".txt").c_str(), &info) == 0) {
			plugins.push_back(new PluginLib(path));
			cout << " loaded " << path << endl;
		}
	}
}

PluginLoader::~PluginLoader() {
	for (PluginLib* plugin : plugins) {
		delete plugin;
	}
}

void* PluginLoader::getplugin_func(const char* name, void* skip_to) {
	bool skip = skip_to != nullptr;
	for (PluginLib* pluginlib : plugins) {
		for (PluginDef plugindef : pluginlib->plugins) {
			if (!skip and strcmp(plugindef.basename, name) == 0) {
				return plugindef.getplugin;
			}
			skip = skip and plugindef.getplugin != skip_to;
		}
	}
	return nullptr;
}


PluginLoader pluginloader;



#endif
