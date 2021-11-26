#ifndef PLUGINS_PREDEF
#define PLUGINS_PREDEF

#include "classes.h"
#include <map>

#ifdef _WIN32
#include <libloaderapi.h>
#define LIBHANDLE HMODULE
#else
#include <dlfcn.h>
#define LIBHANDLE void*
#endif

struct PluginId {
	using id_t = uint64_t;
	using half_id_t = uint32_t;
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
	friend bool operator<(const PluginId& id1, const PluginId& id2) {return id1.id < id2.id;}
};


struct PluginReq {
	PluginId baseid;
	PluginId reqid;
	bool exact = false;
	
	PluginReq() {}
	PluginReq(istream& ifile);
};


class PluginFilter { public:
	std::map<PluginId,PluginReq> requirements;
	std::vector<string> ignored_plugins;
	
	PluginFilter();
};


PluginFilter* pluginfilter();


template <typename BaseType>
struct PluginDef {
	using ctor_func = typename BaseType::ctor_func;
	using destr_func = typename BaseType::destr_func;
	
	ctor_func newfunc = nullptr;
	destr_func delfunc = nullptr;
	PluginId id;
	int level;
	PluginDef<BaseType>* next = nullptr;
	PluginDef<BaseType>* children = nullptr;
	
	PluginDef(PluginId newid): id(newid), level(0) {
		
	}
	
	PluginDef(PluginId newid, PluginDef<BaseType>* parent): id(newid) {
		next = parent->children;
		parent->children = this;
		level = parent->level + 1;
	}
	
	PluginDef<BaseType>* find(PluginId search_id) {
		if (search_id == id) {
			return this;
		} else {
			PluginDef<BaseType>* def = children;
			while (def != nullptr) {
				PluginDef<BaseType>* result = def->find(search_id);
				if (result != nullptr) {
					return result;
				}
				def = def->next;
			}
			return nullptr;
		}
	}
	
	PluginDef<BaseType>* find_deepest() {
		PluginDef<BaseType>* def = children;
		PluginDef<BaseType>* chosen = this;
		while (def != nullptr) {
			PluginDef<BaseType>* result = def->find_deepest();
			if (result != nullptr and result->newfunc != nullptr and result->level > chosen->level) {
				chosen = result;
			}
			def = def->next;
		}
		return chosen;
	}
  
	PluginDef<BaseType>* choose_plugin() {
		PluginReq req = pluginfilter()->requirements[BaseType::plugindef()->id];
		PluginDef<BaseType>* def = this;
		if (req.reqid != PluginId()) {
			def = find(req.reqid);
		}
		if (!req.exact) {
			def = def->find_deepest();
		}
		return def;
	}
	
  void export_plugin(ctor_func nfunc, destr_func dfunc) {
    newfunc = nfunc;
    delfunc = dfunc;
		BaseType::choose_plugin();
  }
};

template <typename Func>
struct MakeGetFunc {

};

template <typename Btype, typename ... Args>
struct MakeGetFunc<Btype* (*)(Args...)> {
	template <typename CType>
	struct GetFunc {
		static Btype* newfunc(Args... args) {
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
	typedef typename MakeGetFunc<typename Ptype::ctor_func>::template GetFunc<Ptype> Funcs;
	
	ExportPlugin() {
		Ptype::plugindef()->export_plugin(Funcs::newfunc, Funcs::delfunc);
	}
};

template <typename Ptype, Ptype* ptr>
struct ExportPluginSingleton {
	using BaseType = typename Ptype::Plugin_BaseType;
	
	PluginDef<BaseType> plugindef;
	ExportPluginSingleton(const char* name): plugindef(PluginId(name), Ptype::plugindef()) {
		plugindef.export_plugin(newfunc, delfunc);
	}
	static BaseType* newfunc() {
		return ptr;
	}
	static void delfunc(BaseType* newptr) {
		
	}
};

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b

#define UNIQUENAME(name) CONCAT(name, __COUNTER__)

#define DEFINE_PLUGIN(X) \
	PluginDef<X>* X::selected_plugin = nullptr;

#define DEFINE_AND_EXPORT_PLUGIN(X) \
	PluginDef<X>* X::selected_plugin = nullptr; \
	static ExportPlugin<X> UNIQUENAME(_export_plugin_);

#define EXPORT_PLUGIN(X) \
	static ExportPlugin<X> UNIQUENAME(_export_plugin_);

#define EXPORT_PLUGIN_SINGLETON(X) \
	static ExportPluginSingleton<std::remove_pointer<decltype(X)>::type,X> UNIQUENAME(_export_singleton_) (#X);

#define BASE_PLUGIN_HEAD(X, params) \
	typedef X Plugin_BaseType; \
	typedef X Plugin_Type; \
	\
	typedef X* (*ctor_func) params; \
	typedef void (*destr_func) (X*); \
	\
	static PluginDef<X>* plugindef() { \
		static PluginDef<X> plugdef (PluginId(#X)); \
		return &plugdef; \
	} \
	static PluginDef<X>* selected_plugin; \
	\
	static void choose_plugin() { \
		selected_plugin = plugindef()->choose_plugin(); \
	} \
	\
	template <typename ... Args> \
	static X* plugnew(Args ... args) { \
		return selected_plugin->newfunc(args...); \
	} \
	template <typename ... Args> \
	static X* plugnew(PluginId search_id, Args ... args) { \
		return plugindef()->find(search_id)->newfunc(args...); \
	} \
	static void plugdelete(X* ptr) { \
		ptr->get_plugindef()->delfunc(ptr); \
	} \
	virtual PluginDef<X>* get_plugindef() const { return plugindef(); } \
	virtual PluginId get_plugin_id() const { return plugindef()->id; }
	


#define PLUGIN_HEAD(X) \
	static PluginDef<Plugin_BaseType>* plugindef() { \
		static PluginDef<Plugin_BaseType> plugdef (PluginId(#X), Plugin_ParentType::plugindef()); \
		return &plugdef; \
	} \
	typedef Plugin_Type Plugin_ParentType; \
	typedef X Plugin_Type; \
	virtual PluginDef<Plugin_BaseType>* get_plugindef() const { return plugindef(); } \
	virtual PluginId get_plugin_id() const { return plugindef()->id; }


























template <typename PluginType>
class BasePlugin { public:
	PluginType* pointer = nullptr;
	
	
	
	void close() {
		if (pointer != nullptr) {
			PluginType::plugdelete(pointer);
			pointer = nullptr;
		}
	}
	
	~BasePlugin() {
		close();
	}
	
	template <typename ... Args>
	void init(Args ... args) {
		pointer = PluginType::plugnew(args...);
	}
	
	template <typename ... Args>
	void init(PluginId id, Args ... args) {
		pointer = PluginType::plugnew(id, args...);
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
	
	void iter(PluginDef<PluginType>* def) {
		if (def->newfunc != nullptr) {
			defs.push_back(def);
		}
		for (PluginDef<PluginType>* cdef = def->children; cdef != nullptr; cdef = cdef->next) {
			iter(cdef);
		}
	}
	
	template <typename ... Args>
	void init(Args ... args) {
		iter(PluginType::plugindef());
		for (PluginDef<PluginType>* def : defs) {
			pointers.push_back(def->newfunc(args...));
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

extern PluginLoader pluginloader;

#define PLUGIN_REQUIRES(prefix, plugin, NewType) \
	PluginUpCast<GetPluginType<decltype(prefix plugin)>::type, NewType> plugin {&(prefix plugin)};

#define PLUGIN_REQUIRES_RUNTIME(oldname, newname, NewType) \
	struct { \
		typedef GetPluginType<decltype(oldname)>::type OldType; \
		NewType* operator->() { \
			return (NewType*) ((OldType*) oldname); \
		} \
	} newname;

#endif
