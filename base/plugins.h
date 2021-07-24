#ifndef PLUGINS_PREDEF
#define PLUGINS_PREDEF

#include "classes.h"

#ifdef _WIN32
#include <libloaderapi.h>
#define LIBHANDLE HMODULE
#define LOADLIB(X) LoadLibrary(X)
#define GETADDR(HAND, X) GetProcAddress(HAND, X)
#else
#include <dlfcn.h>
#define LIBHANDLE void*
#define LOADLIB(X) dlopen(X, RTLD_NOW | RTLD_GLOBAL)
#define GETADDR(HAND, X) dlsym(HAND, X)
#endif



class PluginLib { public:
	LIBHANDLE handle;
	const char* basename;
	void* (*getplugin)();
	
	PluginLib(string path);
	
	~PluginLib();
};

class PluginLoader { public:
	vector<PluginLib> plugins;
	
	PluginLoader();
	
	void* (*getplugin_func(const char* name))();
};

extern PluginLoader pluginloader;

template <typename PluginType>
class Plugin { public:
	PluginType* pointer;
	
	template <typename ... Args, std::enable_if_t<std::is_constructible<PluginType, Args...>::value, int> = 0>
	Plugin(Args ... args) {
		PluginType* (*getfunc)(Args...) = (PluginType* (*)(Args...)) pluginloader.getplugin_func(PluginType::basename);
		
		if (getfunc == nullptr) {
			pointer = new PluginType(args...);
		} else {
			pointer = getfunc(args...);
		}
	}
	
	~Plugin() {
		delete pointer;
	}
	
	PluginType* operator->() {
		return pointer;
	}
	
	operator PluginType*() {
		return pointer;
	}
};

template <typename Func>
struct PluginGetCtorArgs {
	
};

template <typename Type, typename ... Args>
struct PluginGetCtorArgs<Type* (*)(Args...)> {
	template <typename CType>
	static void* getfunc(Args... args) {
		return (void*) new CType(args...);
	}
};

#define PLUGIN_HEAD(X, params) static constexpr const char* basename = #X; \
	typedef void* (*ctor_func) params;

#define EXPORT_PLUGIN(X) extern "C" { \
	const char* getbasename() { return X::basename; } \
	X::ctor_func getplugin() { return &PluginGetCtorArgs<X::ctor_func>::getfunc<X>;} }





/*
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
};*/

#endif
