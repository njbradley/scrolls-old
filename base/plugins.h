#ifndef PLUGINS_PREDEF
#define PLUGINS_PREDEF

#include "classes.h"

#ifdef _WIN32
#include <libloaderapi.h>
#define LIBHANDLE HMODULE
#define LOADLIB(X) LoadLibrary(X)
#define GETADDR(HAND, X) GetProcAddress(HAND, X)
#define DLLSUFFIX ".dll"
#else
#include <dlfcn.h>
#define LIBHANDLE void*
#define LOADLIB(X) dlopen(X, RTLD_NOW | RTLD_GLOBAL)
#define GETADDR(HAND, X) dlsym(HAND, X)
#define DLLSUFFIX ".so"
#endif



class PluginLib { public:
	LIBHANDLE handle;
	const char* basename;
	void* (*getplugin)();
	string dirname;
	
	PluginLib(string path);
	
	~PluginLib();
};

class PluginLoader { public:
	vector<PluginLib*> plugins;
	
	PluginLoader();
	~PluginLoader();
	
	void* (*getplugin_func(const char* name))();
};

extern PluginLoader pluginloader;

template <typename PluginType>
class Plugin { public:
	PluginType* pointer;
	
	template <typename PType = PluginType, typename ... Args,
			std::enable_if_t<std::is_constructible<PType, Args...>::value and !std::is_abstract<PType>::value, int> = 0>
	Plugin(Args ... args) {
		cout << "Loading plugin ..." << endl;
		PluginType* (*getfunc)(Args...) = (PluginType* (*)(Args...)) pluginloader.getplugin_func(PluginType::basename);
		
		if (getfunc == nullptr) {
			cout << " used default" << endl;
			pointer = new PluginType(args...);
		} else {
			cout << " loaded from plugin" << endl;
			pointer = getfunc(args...);
		}
	}
	
	template <typename PType = PluginType, typename ... Args,
			std::enable_if_t<std::is_abstract<PType>::value, int> = 0>
	Plugin(Args ... args) {
		cout << "Loading plugin with no default! ..." << endl;
		PluginType* (*getfunc)(Args...) = (PluginType* (*)(Args...)) pluginloader.getplugin_func(PluginType::basename);
		
		if (getfunc == nullptr) {
			cout << " AAAAAAA " << endl;
			pointer = nullptr;
		} else {
			cout << " loaded from plugin" << endl;
			pointer = getfunc(args...);
		}
	}
	
	~Plugin() {
		if (pointer != nullptr) {
			delete pointer;
		}
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





#endif
