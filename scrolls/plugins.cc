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


PluginReq::PluginReq(istream& ifile) {
	string buf;
	getline(ifile, buf, '=');
	baseid = PluginId(buf.c_str());
	char let;
	ifile >> let >> buf;
	if (let == '>') {
		exact = false;
	} else {
		exact = true;
		buf = let + buf;
	}
	reqid = PluginId(buf.c_str());
}



vector<void(*)()>* plugin_choosers() {
	static vector<void(*)()> choosers;
	return &choosers;
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

void PluginLoader::load(string path) {
	load_reqs(path);
	
	struct stat info;
	for (std::filesystem::path fspath : std::filesystem::directory_iterator("./")) {
		string path = fspath.filename().string();
		if (std::find(ignored_plugins.begin(), ignored_plugins.end(), path) == ignored_plugins.end()
			and (stat((path + "/" + path + DLLSUFFIX).c_str(), &info) == 0
			or stat((path + "/" + path + ".txt").c_str(), &info) == 0)) {
			plugins.push_back(new PluginLib(path));
		}
	}
	
	choose_plugins();
}

void PluginLoader::load_reqs(string path) {
	ifstream ifile(path);
	string linebuf;
	while (ifile.good()) {
		getline(ifile, linebuf);
		linebuf = linebuf.substr(0, linebuf.find('#'));
		
		stringstream line(linebuf);
		
		if (linebuf.find('=') == string::npos) {
			char let = 0;
			line >> let;
			if (let == '!') {
				string ignore;
				line >> ignore;
				if (ignore.length() > 0) {
					ignored_plugins.push_back(ignore);
					cout << "Ignoring plugin " << ignore << '/' << ignore << endl;
				}
			}
		} else {
			PluginReq req(line);
			if (req.baseid != PluginId()) {
				requirements[req.baseid] = req;
				cout << req.baseid.name() << " = " << req.reqid.name() << endl;
			}
		}
	}
}

void PluginLoader::choose_plugins() {
	for (void (*func)() : *plugin_choosers()) {
		func();
	}
}

PluginLoader::~PluginLoader() {
	for (PluginLib* plugin : plugins) {
		delete plugin;
	}
}

PluginLoader pluginloader;

#endif
