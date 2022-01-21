#ifndef PLUGINS
#define PLUGINS

#include "plugins.h"

#include "cross-platform.h"
#include <filesystem>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#define LOADLIB(X) LoadLibrary(X)
#define FREELIB(X) FreeLibrary(X)
#define GETADDR(HAND, X) GetProcAddress(HAND, X)
#define DLLSUFFIX ".dll"

void print_err() {
  //Get the error message ID, if any.
  DWORD errorMessageID = ::GetLastError();
  if(errorMessageID == 0) {
      return; //No error message has been recorded
  }
  
  LPSTR messageBuffer = nullptr;

  //Ask Win32 to give us the string version of that message ID.
  //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
  
	cout.write(messageBuffer, size);
	
  //Free the Win32's string's buffer.
  LocalFree(messageBuffer);
}

#define PRINTERR print_err();
#else
#include <dlfcn.h>
#define LOADLIB(X) dlopen(X, RTLD_NOW | RTLD_GLOBAL)
// #define LOADLIB(X) dlopen(X, RTLD_NOW)
#define FREELIB(X) dlclose(X)
#define GETADDR(HAND, X) dlsym(HAND, X)
#define PRINTERR cout << dlerror() << endl;
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
			PRINTERR;
			handle = nullptr;
			error_loading = true;
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
	
	vector<string> to_load;
	
	for (std::filesystem::path fspath : std::filesystem::directory_iterator("./")) {
		string path = fspath.filename().string();
		if (std::find(ignored_plugins.begin(), ignored_plugins.end(), path) == ignored_plugins.end()
			and (stat(path + "/" + path + DLLSUFFIX) or stat(path + "/" + path + ".txt"))) {
			to_load.push_back(path);
		}
	}
	
	while (to_load.size() > 0) {
		int start_size = to_load.size();
	
		for (int i = to_load.size()-1; i >= 0; i --) {
			PluginLib* newlib = new PluginLib(to_load[i]);
			if (newlib->error_loading) {
				delete newlib;
			} else {
				plugins.push_back(newlib);
				to_load.erase(to_load.begin() + i);
			}
		}
		
		if (start_size == to_load.size()) {
			cout << "ERROR: cannot resolve errors loading libraries" << endl;
			cout << "  Could not load: ";
			for (string path : to_load) {
				cout << path << ' ';
			}
			cout << endl;
			break;
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
			} else {
				string newpath;
				line >> newpath;
				newpath = let + newpath;
				load_reqs(newpath);
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
