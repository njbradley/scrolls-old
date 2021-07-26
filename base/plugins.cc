#ifndef PLUGINS
#define PLUGINS

#include "plugins.h"

#include "cross-platform.h"
#include <filesystem>


PluginLib::PluginLib(string path) {
	string dllpath = path + "/" + path + DLLSUFFIX;
	dirname = path;
	handle = LOADLIB(dllpath.c_str());
	if (!handle) {
		cout << "Plugin error: [" << path << "]: cannot open dll" << endl;
	}
	
	PluginDef* plugindefs = (PluginDef*) GETADDR(handle, "plugins");
	if (plugindefs == nullptr) {
		cout << "Plugin error: [" << path << "]: cannot find plugins" << endl;
	}
	
	while (plugindefs->basename != nullptr) {
		plugins.push_back(*plugindefs);
		plugindefs ++;
	}
}

PluginLib::~PluginLib() {
	FreeLibrary(handle);
}




PluginLoader::PluginLoader() {
	cout << "Loading plugins " << endl;
	for (std::filesystem::path fspath : std::filesystem::directory_iterator("./")) {
		string path = fspath.filename().string();
		ifstream ifile(path + "/" + path + DLLSUFFIX);
		if (ifile.good()) {
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

void* PluginLoader::getplugin_func(const char* name) {
	for (PluginLib* pluginlib : plugins) {
		for (PluginDef plugindef : pluginlib->plugins) {
			if (strcmp(plugindef.basename, name) == 0) {
				return plugindef.getplugin;
			}
		}
	}
	return nullptr;
}


PluginLoader pluginloader;



#endif
