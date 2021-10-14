#ifndef PLUGINS_PREDEF
#define PLUGINS_PREDEF

#include "classes.h"

#ifdef _WIN32
#include <libloaderapi.h>
#define LIBHANDLE HMODULE
#else
#include <dlfcn.h>
#define LIBHANDLE void*
#endif



struct PluginDef {
	void* getfunc;
	void* delfunc;
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
	void (*delfunc)(PluginType*);
	
	void close() {
		if (pointer != nullptr) {
			delfunc(pointer);
			pointer = nullptr;
		}
	}
	
	~BasePlugin() {
		close();
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
		if (plugin_headptr<PluginType> != nullptr) {
			delfunc = (void (*) (PluginType*)) plugin_headptr<PluginType>->delfunc;
		}
	}
	
	PluginType* operator->() {
		return pointer;
	}
	
	operator PluginType*() {
		return pointer;
	}
	
	template <typename UpCast>
	UpCast* as() {
		return dynamic_cast<UpCast*>(pointer);
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


template <typename BaseType, typename UpCastType>
class PluginUpCast { public:
	BasePlugin<BaseType>* plugin_ref;
	UpCastType* operator->() {
		UpCastType* newptr = dynamic_cast<UpCastType*>((BaseType*) *plugin_ref);
		if (newptr == nullptr) {
			cout << "ERR: plugin " << typeid(BaseType).name() << " is required to be " << typeid(UpCastType).name() << " by another plugin " << endl;
		}
		return newptr;
	}
};

template <typename PluginType>
struct GetPluginType {
	
};

template <typename PluginType>
struct GetPluginType<Plugin<PluginType>> {
	using type = PluginType;
};

template <typename PluginType>
struct GetPluginType<PluginNow<PluginType>> {
	using type = PluginType;
};

template <typename PluginType>
class BasePluginList { public:
	vector<PluginType*> pointers;
	vector<void (*)(PluginType*)> delfuncs;
	
	template <typename Ptype = PluginType, typename ... Args,
		std::enable_if_t<IsFunction<typename Ptype::ctor_func, Args...>::value, int> = 0>
	void init(Args ... args) {
		PluginDef* def = plugin_headptr<Ptype>;
		while (def != nullptr) {
			typename Ptype::ctor_func getfunc = (typename Ptype::ctor_func) def->getfunc;
			pointers.push_back(getfunc(args...));
			delfuncs.push_back((void (*) (PluginType*)) def->delfunc);
			def = def->next;
		}
	}
	
	void close() {
		for (int i = 0; i < pointers.size(); i ++) {
			delfuncs[i](pointers[i]);
		}
		pointers.clear();
		delfuncs.clear();
	}
	
	~BasePluginList() {
		close();
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
		static Btype* getfunc(Args... args) {
			return new CType(args...);
		}
		static void delfunc(Btype* ptr) {
			delete ptr;
		}
	};
};






template <typename Ptype>
struct ExportPlugin {
	using BaseType = typename Ptype::Plugin_BaseType;
	// template <T>
	// using TmpType = typename PluginGetCtorArgs<typename Ptype::ctor_func>::GetFunc<T>;
	// using GArgs = TmpType<Ptype>;
	typedef typename PluginGetCtorArgs<typename Ptype::ctor_func>::template GetFunc<Ptype> GArgs;
	PluginDef plugindef {(void*) GArgs::getfunc, (void*) GArgs::delfunc, plugin_headptr<BaseType>};
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
	PluginDef plugindef {(void*)&getfunc, (void*)&delfunc, plugin_headptr<BaseType>};
	ExportPluginSingleton() {
		plugin_headptr<BaseType> = &plugindef;
	}
	static BaseType* getfunc() {
		return ptr;
	}
	static void delfunc(BaseType* newptr) {
		
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
	void load();
	~PluginLoader();
};

extern PluginLoader pluginloader;

#define DEFINE_PLUGIN(X) template <> PluginDef* plugin_headptr<X> = nullptr;

#define EXPORT_PLUGIN(X) ExportPlugin<X> _exported_plugin_ ## X;

#define EXPORT_PLUGIN_SINGLETON(X) ExportPluginSingleton<decltype(X),&X> _exported_plugin_singleton_ ## X;

#define PLUGIN_HEAD(X, params) typedef X Plugin_BaseType; \
	typedef X* (*ctor_func) params;

#define PLUGIN_REQUIRES(prefix, plugin, NewType) \
	PluginUpCast<GetPluginType<decltype(prefix plugin)>::type, NewType> plugin {&(prefix plugin)};

#endif
