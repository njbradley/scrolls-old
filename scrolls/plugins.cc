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
		cout << "loaded " << dllpath << " into address space " << endl;
		if (!handle) {
			cout << "Plugin error: [" << path << "]: cannot open dll" << endl;
			handle = nullptr;
		}
	}
}

PluginLib::~PluginLib() {
	if (handle != nullptr) {
		FREELIB(handle);
	}
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

PluginLoader pluginloader;

#endif
