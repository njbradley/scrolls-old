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
	const char* (*func)() = (const char* (*)()) GETADDR(handle, "getbasename");
	if (func == nullptr) {
		cout << "Plugin error: [" << path << "]: cannot find function getbasename" << endl;
	}
	basename = func();
	cout << " basename " << basename << ' ' << (void*) basename << endl;
	void* (*func2)() = (void* (*)()) GETADDR(handle, "getplugin");
	if (func2 == nullptr) {
		cout << "Plugin error: [" << path << "]: cannot find function getplugin" << endl;
	}
	getplugin = (void* (*)()) func2();
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

void* (*PluginLoader::getplugin_func(const char* name))() {
	for (PluginLib* plugin : plugins) {
		if (strcmp(plugin->basename, name) == 0) {
			return plugin->getplugin;
		}
	}
	return nullptr;
}


PluginLoader pluginloader;



#endif
