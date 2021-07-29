#ifndef PLUGINS_PREDEF
#define PLUGINS_PREDEF

#include "classes.h"

#ifdef _WIN32
#include <libloaderapi.h>
#define LIBHANDLE HMODULE
#define LOADLIB(X) LoadLibrary(X)
#define FREELIB(X) FreeLibrary(X)
#define GETADDR(HAND, X) GetProcAddress(HAND, X)
#define DLLSUFFIX ".dll"
#else
#include <dlfcn.h>
#define LIBHANDLE void*
// #define LOADLIB(X) dlopen(X, RTLD_NOW | RTLD_GLOBAL)
#define LOADLIB(X) dlopen(X, RTLD_NOW)
#define FREELIB(X) dlfree(X)
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

enum PluginLoadOrder {
	PluginLoad_NOW,
	PluginLoad_LATER
};

template <typename PluginType>
class BasePlugin { public:
	PluginType* pointer = nullptr;
	
	~BasePlugin() {
		if (pointer != nullptr) {
			delete pointer;
		}
	}
	
	template <typename Ptype = PluginType, typename ... Args,
		std::enable_if_t<std::is_invocable<typename Ptype::ctor_func, Args...>::value, int> = 0>
	void init(Args ... args) {
		Ptype* (*getfunc)() = (Ptype* (*) ()) pluginloader.getplugin_func(Ptype::basename);
		
		if (getfunc == nullptr) {
			if constexpr (!std::is_abstract<Ptype>::value) {
				pointer = new Ptype(args...);
			}
		} else {
			pointer = getfunc(args...);
		}
	}
	
	PluginType* operator->() {
		return pointer;
	}
	
	operator PluginType*() {
		return pointer;
	}
};

template <typename PluginType, PluginLoadOrder order = PluginLoad_LATER>
class Plugin : public BasePlugin<PluginType> { public:
	Plugin() { }
};

template <typename PluginType>
class Plugin<PluginType, PluginLoad_NOW> : public BasePlugin<PluginType> { public:
	template <typename ... Args>
	Plugin(Args ... args) {
		load(args...);
	}
};

template <typename Ptype>
using PluginNow = Plugin<Ptype, PluginLoad_NOW>;

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
