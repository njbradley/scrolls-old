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

struct PluginId {
	using id_t = unsigned long int;
	using half_id_t = unsigned int;
	id_t id;
	
	constexpr char compress_char(char let) const {
		if ('a' <= let and let <= 'z') {
			let = let - ('a' - 'A');
		}
		if (let != 'Q' and let != 'J') {
			if (let == 'W') let = 'Q';
			if (let == 'Y') let = 'J';
			if ('A' <= let and let <= 'V') {
				return let - 'A';
			} else if ('0' <= let and let <= '9') {
				return let - '0' + 22;
			}
		}
		return 32;
	}
	constexpr char decompress_char(char let) const {
		if (let < 22) {
			let += 'A';
		} else {
			let += '0' - 22;
		}
		if (let == 'Q') let = 'W';
		if (let == 'J') let = 'Y';
		return let;
	}
	
	constexpr PluginId(): id(0) {};
	explicit constexpr PluginId(const char* str): id(0) {
		half_id_t multiplier = 1;
		half_id_t first_part = 0;
		half_id_t last_part = 0;
		
		int len = 0;
		int i = 0;
		for (i = 0; str[i] != 0 and multiplier*16 != 0; i ++) {
			char let = compress_char(str[i]);
			if (let < 32) {
				first_part += let * multiplier;
				multiplier *= 32;
				len ++;
			}
		}
		
		int last_bit = sizeof(half_id_t)*8/5*5;
		half_id_t max_mul = 1;
		while (max_mul*32*16 != 0) max_mul *= 32;
		multiplier = max_mul;
		
		int j = 0;
		while (str[j] != 0) j ++;
		for (j = j-1; j >= i and multiplier != 0; j --) {
			char let = compress_char(str[j]);
			if (let < 32) {
				last_part += let * multiplier;
				multiplier /= 32;
				len ++;
			}
		}
		
		if (last_part == 0) {
			id = first_part;
		} else if (multiplier == 0) {
			id = id_t(last_part) * max_mul * 32 + first_part;
		} else {
			id = id_t(last_part/(multiplier)) * max_mul + first_part;
		}
		id = id * 16 + len;
	}
	explicit constexpr PluginId(id_t newid): id(newid) {};
	
	string name() const;
	
	friend bool operator==(const PluginId& id1, const PluginId& id2) {return id1.id == id2.id;}
	friend bool operator!=(const PluginId& id1, const PluginId& id2) {return !(id1 == id2);}
};

template <typename BaseType>
struct PluginDef {
	using ctor_func = typename BaseType::ctor_func;
	using destr_func = void (*) (BaseType*);
	
	ctor_func getfunc;
	destr_func delfunc;
	PluginId id;
	PluginDef<BaseType>* next;
	
	PluginDef(ctor_func new_getfunc, destr_func new_delfunc, const char* name);
	
	PluginDef<BaseType>* find(PluginId search_id) {
		PluginDef<BaseType>* def = this;
		while (def != nullptr and def->id != search_id) {
			def = def->next;
		}
		return def;
	}
};

template <typename BaseType>
extern PluginDef<BaseType>* pluginhead;

template <typename BaseType>
PluginDef<BaseType>::PluginDef(ctor_func new_getfunc, destr_func new_delfunc, const char* name):
getfunc(new_getfunc), delfunc(new_delfunc), id(name), next(pluginhead<BaseType>) {
	pluginhead<BaseType> = this;
}

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
	PluginDef<PluginType>* def;
	PluginType* pointer = nullptr;
	
	
	
	void close() {
		if (pointer != nullptr) {
			def->delfunc(pointer);
			pointer = nullptr;
		}
	}
	
	~BasePlugin() {
		close();
	}
	
	template <typename ... Args>
	void init(Args ... args) {
		def = pluginhead<PluginType>;
		if (def != nullptr) {
			pointer = def->getfunc(args...);
		}
	}
	
	template <typename ... Args>
	void init(PluginId id, Args ... args) {
		def = pluginhead<PluginType>.find(id);
		if (def != nullptr) {
			pointer = def->getfunc(args...);
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
		return (UpCast*) pointer;
	}
};

template <typename PluginType>
class Plugin : public BasePlugin<PluginType> { public:
	
};

template <typename PluginType>
class PluginNow : public BasePlugin<PluginType> { public:
	template <typename ... Args>
	PluginNow(Args ... args) {
		this->init(args...);
	}
	
	template <typename ... Args>
	PluginNow(PluginId id, Args ... args) {
		this->init(id, args...);
	}
};


template <typename BaseType, typename UpCastType>
class PluginUpCast { public:
	BasePlugin<BaseType>* plugin_ref;
	UpCastType* operator->() {
		static bool checked = false;
		if (!checked) {
			UpCastType* newptr = dynamic_cast<UpCastType*>((BaseType*) *plugin_ref);
			if (newptr == nullptr) {
				cout << "ERR: plugin " << typeid(BaseType).name() << " is required to be " << typeid(UpCastType).name() << " by another plugin " << endl;
			}
			checked = true;
		}
		return (UpCastType*)((BaseType*) *plugin_ref);
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
	vector<PluginDef<PluginType>*> defs;
	vector<PluginType*> pointers;
	
	
	
	template <typename ... Args>
	void init(Args ... args) {
		PluginDef<PluginType>* def = pluginhead<PluginType>;
		while (def != nullptr) {
			defs.push_back(def);
			pointers.push_back(def->getfunc(args...));
			def = def->next;
		}
	}
	
	void close() {
		for (int i = 0; i < pointers.size(); i ++) {
			defs[i]->delfunc(pointers[i]);
		}
		pointers.clear();
		defs.clear();
	}
	
	~BasePluginList() {
		close();
	}
	
	PluginType* operator[](int index) {
		return pointers[index];
	}
	
	PluginType* operator[](PluginId id) {
		for (int i = 0; i < pointers; i ++) {
			if (defs[i]->id == id) {
				return pointers;
			}
		}
		return nullptr;
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
	typedef typename PluginGetCtorArgs<typename Ptype::ctor_func>::template GetFunc<Ptype> GArgs;
	
	PluginDef<BaseType> plugindef;
	
	ExportPlugin(): plugindef(GArgs::getfunc, GArgs::delfunc, Ptype::plugin_name) {
		cout << "exporting plugin " << Ptype::plugin_id.name() << ' ' << BaseType::plugin_baseid.name() << endl;
	}
};

template <typename Ptype, Ptype* ptr>
struct ExportPluginSingleton {
	static_assert(IsFunction<typename Ptype::ctor_func>::value,
		"ExportPluginSingleton can only be used with objects with no constructor parameters");
	using BaseType = typename Ptype::Plugin_BaseType;
	
	PluginDef<BaseType> plugindef;
	ExportPluginSingleton(const char* name): plugindef(getfunc, delfunc, name) {
		
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


#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b

#define UNIQUENAME CONCAT(_unique_name_, __COUNTER__)

extern PluginLoader pluginloader;

#define DEFINE_PLUGIN(X) template <> PluginDef<X>* pluginhead<X> = nullptr;

#define EXPORT_PLUGIN(X) static ExportPlugin<X> UNIQUENAME;

#define EXPORT_PLUGIN_SINGLETON(X) static ExportPluginSingleton<decltype(X),&X> UNIQUENAME (#X);

#define BASE_PLUGIN_HEAD(X, params) typedef X Plugin_BaseType; \
	typedef X* (*ctor_func) params; \
	static constexpr const char* plugin_basename = #X; \
	static constexpr PluginId plugin_baseid = PluginId(#X); \
	static constexpr const char* plugin_name = #X; \
	static constexpr PluginId plugin_id = PluginId(#X); \
	static const unsigned int plugin_level = 0;

#define PLUGIN_HEAD(X) static constexpr const char* plugin_name = #X; \
	static constexpr PluginId plugin_id = PluginId(#X); \
	static const unsigned int plugin_level = plugin_level + 1;

#define PLUGIN_REQUIRES(prefix, plugin, NewType) \
	PluginUpCast<GetPluginType<decltype(prefix plugin)>::type, NewType> plugin {&(prefix plugin)};

#endif
