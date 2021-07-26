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
// #define LOADLIB(X) dlopen(X, RTLD_NOW | RTLD_GLOBAL)
#define LOADLIB(X) dlopen(X, RTLD_NOW)
#define GETADDR(HAND, X) dlsym(HAND, X)
#define DLLSUFFIX ".so"
#endif


extern "C" {

struct PluginDef {
	const char* basename;
	void* getplugin;
};

}




class PluginLib { public:
	LIBHANDLE handle;
	vector<PluginDef> plugins;
	string dirname;
	
	PluginLib(string path);
	
	~PluginLib();
};

class PluginLoader { public:
	vector<PluginLib*> plugins;
	
	PluginLoader();
	~PluginLoader();
	
	void* getplugin_func(const char* name);
};

extern PluginLoader pluginloader;

template <typename PluginType>
class Plugin { public:
	PluginType* pointer;
	
	template <typename PType = PluginType, typename ... Args,
			std::enable_if_t<std::is_constructible<PType, Args...>::value and !std::is_abstract<PType>::value, int> = 0>
	Plugin(Args ... args) {
		cout << "Loading plugin " << PType::basename << " ... " << endl;
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
		cout << "Loading plugin " << PType::basename << " with no default! ..." << endl;
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
	struct GetFunc {
		static void* getfunc(Args... args) {
			return (void*) new CType(args...);
		}
		// static constexpr void* ptr = getfunc;
	};
};

template <typename X>
struct PluginGetFuncPtr {
  using GArgs = typename PluginGetCtorArgs<typename X::ctor_func>::GetFunc<X>;
  static constexpr void* (*ptr)() = GArgs::getfunc;
};

template <typename ... Ptypes>
struct ExportPlugins {
  PluginDef plugins[sizeof...(Ptypes)] = {{Ptypes::basename, (void*)PluginGetFuncPtr<Ptypes>::ptr}... };
  PluginDef terminator = {nullptr, nullptr};
};

#define PLUGIN_HEAD(X, params) static constexpr const char* basename = #X; \
	typedef void* (*ctor_func) params;


#define EXPORT_PLUGINS(...) extern "C" { \
	ExportPlugins<__VA_ARGS__> plugins; }

#define EXPORT_PLUGIN(X) extern "C" { \
	ExportPlugins<X> plugins; }





#endif
