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




struct PluginDef {
	void* getfunc;
	struct PluginDef* next;
};

template <typename Ptype>
extern PluginDef* plugin_headptr;

template <typename Func, typename ... Args>
struct IsFunction {
	static const bool value = false;
};

template <typename Type, typename ... Args>
struct IsFunction<Type(*)(Args...),Args...> {
	static const bool value = true;
};

template <typename Type, typename ... Args>
struct IsFunction<Type(Args...),Args...> {
	static const bool value = true;
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
		std::enable_if_t<IsFunction<typename Ptype::ctor_func, Args...>::value, int> = 0>
	static Ptype* create(Args ... args) {
		if (plugin_headptr<Ptype> != nullptr) {
			typename Ptype::ctor_func getfunc = (typename Ptype::ctor_func) plugin_headptr<Ptype>->getfunc;
			return getfunc(args...);
		}
		return nullptr;
	}
	
	template <typename ... Args>
	void init(Args ... args) {
		pointer = create(args...);
	}
	
	void close() {
		if (pointer != nullptr) {
			delete pointer;
			pointer = nullptr;
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
		cout << "Plugin now init " << typeid(PluginType).name() << endl;
		this->init(args...);
	}
};


template <typename PluginType>
class BasePluginList { public:
	vector<PluginType*> pointers;
	
	template <typename Ptype = PluginType, typename ... Args,
		std::enable_if_t<IsFunction<typename Ptype::ctor_func, Args...>::value, int> = 0>
	void init(Args ... args) {
		PluginDef* def = plugin_headptr<Ptype>;
		while (def != nullptr) {
			typename Ptype::ctor_func getfunc = (typename Ptype::ctor_func) def->getfunc;
			pointers.push_back(getfunc(args...));
			def = def->next;
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
		this->init(args...);
	}
};





template <typename Func>
struct PluginGetCtorArgs {

};

template <typename Btype, typename ... Args>
struct PluginGetCtorArgs<Btype* (*)(Args...)> {
	template <typename CType>
	struct GetFunc {
		static void* getfunc(Args... args) {
			return (void*) new CType(args...);
		}
	};
};






template <typename Ptype>
struct ExportPlugin {
	using BaseType = typename Ptype::Plugin_BaseType;
	using GArgs = typename PluginGetCtorArgs<typename Ptype::ctor_func>::GetFunc<Ptype>;
	PluginDef plugindef {(void*)GArgs::getfunc, plugin_headptr<BaseType>};
	ExportPlugin() {
		cout << "exporting plugin " << typeid(BaseType).name() << ' ' << typeid(Ptype).name() << ' ' << plugin_headptr<BaseType> << ' ' << &plugin_headptr<BaseType> << endl;
		plugin_headptr<BaseType> = &plugindef;
	}
};

template <typename Ptype, Ptype* ptr>
struct ExportPluginSingleton {
	static_assert(IsFunction<typename Ptype::ctor_func>::value,
		"ExportPluginSingleton can only be used with objects with no constructor parameters");
	using BaseType = typename Ptype::Plugin_BaseType;
	PluginDef plugindef {(void*)&getfunc, plugin_headptr<BaseType>};
	ExportPluginSingleton() {
		plugin_headptr<BaseType> = &plugindef;
	}
	static BaseType* getfunc() {
		return ptr;
	}
};





template <typename PluginType>
class Storage : public BasePluginList<PluginType> { public:
	using BasePluginList<PluginType>::pointers;
	using BasePluginList<PluginType>::operator[];
	
	PluginType* get(int index) {
		return pointers[index];
	}
	
	PluginType* get(string name) {
	  for (PluginType* data : pointers) {
	    if (data->name == name) {
	      return data;
	    }
	  }
	  return nullptr;
	}
	
	PluginType* operator[] (string name) {
		return get(name);
	}
};


class PluginLib { public:
	LIBHANDLE handle = nullptr;
	vector<PluginDef> plugins;
	string dirname;
	
	PluginLib(string path);
	
	~PluginLib();
};

class PluginLoader { public:
	vector<PluginLib*> plugins;
	
	PluginLoader();
	~PluginLoader();
};

extern PluginLoader pluginloader;

#define DEFINE_PLUGIN(X) template <> PluginDef* plugin_headptr<X> = nullptr;

#define EXPORT_PLUGIN(X) ExportPlugin<X> _exported_plugin_ ## X;

#define EXPORT_PLUGIN_SINGLETON(X) ExportPluginSingleton<decltype(X),&X> _exported_plugin_singleton_ ## X;

#define PLUGIN_HEAD(X, params) typedef X Plugin_BaseType; \
	typedef X* (*ctor_func) params;



#endif
