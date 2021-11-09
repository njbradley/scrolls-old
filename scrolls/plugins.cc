#ifndef PLUGINS
#define PLUGINS

#include "plugins.h"

#include "cross-platform.h"
#include <filesystem>
#include <sys/stat.h>

#ifdef _WIN32
#include <libloaderapi.h>
#define LOADLIB(X) LoadLibrary(X)
#define FREELIB(X) FreeLibrary(X)
#define GETADDR(HAND, X) GetProcAddress(HAND, X)
#define DLLSUFFIX ".dll"
#else
#include <dlfcn.h>
// #define LOADLIB(X) dlopen(X, RTLD_NOW | RTLD_GLOBAL)
#define LOADLIB(X) dlopen(X, RTLD_NOW)
#define FREELIB(X) dlclose(X)
#define GETADDR(HAND, X) dlsym(HAND, X)
#define DLLSUFFIX ".so"
#endif




string PluginId::name() const {
	string str ("............");
	id_t tmpid = id;
	int len = tmpid%16;
	tmpid /= 16;
	for (int i = 0; i < len; i ++) {
		str[i] = decompress_char(tmpid % 32);
		tmpid /= 32;
	}
	return str;
}

PluginLib::PluginLib(string path) {
	struct stat info;
	string dllpath = path + "/" + path + DLLSUFFIX;
	dirname = path;
	if (stat(dllpath.c_str(), &info) == 0) {
		handle = LOADLIB(dllpath.c_str());
		if (!handle) {
			cout << "Plugin error: [" << path << "]: cannot load dll" << endl;
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
	
}

void PluginLoader::load() {
	struct stat info;
	for (std::filesystem::path fspath : std::filesystem::directory_iterator("./")) {
		string path = fspath.filename().string();
		if (stat((path + "/" + path + DLLSUFFIX).c_str(), &info) == 0 or stat((path + "/" + path + ".txt").c_str(), &info) == 0) {
			plugins.push_back(new PluginLib(path));
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
