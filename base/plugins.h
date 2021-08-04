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
	
	void* getplugin_func(const char* name, void* skip_to = nullptr);
};

extern PluginLoader pluginloader;

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

template <typename PluginType>
class Plugin : public BasePlugin<PluginType> { public:
	Plugin() { }
};

template <typename PluginType>
class PluginNow : public BasePlugin<PluginType> { public:
	template <typename ... Args>
	PluginNow(Args ... args) {
		load(args...);
	}
};

template <typename PluginType>
class BasePluginList { public:
	vector<PluginType*> pointers;
	
	template <typename Ptype = PluginType, typename ... Args,
		std::enable_if_t<std::is_invocable<typename Ptype::ctor_func, Args...>::value, int> = 0>
	void init(Args ... args) {
		
		void* funcptr = pluginloader.getplugin_func(Ptype::basename);
		
		if constexpr (!std::is_abstract<Ptype>::value) {
			pointers.push_back(new Ptype(args...));
		}
		
		while (funcptr != nullptr) {
			Ptype* (*getfunc)() = (Ptype* (*) ()) funcptr;
			pointers.push_back(getfunc(args...));
			funcptr = pluginloader.getplugin_func(Ptype::basename, funcptr);
		}
	}
	
	PluginType* operator[](int index) {
		return pointers[index];
	}
	
	size_t size() const {
		return pointers.size();
	}
	
	using iterator = typename vector<PluginType*>::const_iterator;
	
	iterator begin() const {
		return pointers.begin();
	}
	iterator end() const {
		return pointers.end();
	}
};


template <typename PluginType>
class PluginList : public BasePluginList<PluginType> { public:
	PluginList() { }
};

template <typename PluginType>
class PluginListNow : public BasePluginList<PluginType> { public:
	template <typename ... Args>
	PluginListNow(Args ... args) {
		load(args...);
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
