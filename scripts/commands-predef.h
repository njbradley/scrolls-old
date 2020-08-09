#ifndef COMMANDS_PREDEF
#define COMMANDS_PREDEF

#include "classes.h"

template <typename Tret> using CommandFunc = function<Tret(Program*)>;
typedef CommandFunc<void> CommandVoidFunc;

template<typename Type> class CommandConst { public:
	Type value;
	CommandConst(Type newval);
	CommandFunc<Type> getfunc();
};

template <typename ... T> class CommandMethodTemplate {
	
};

template <typename Tret, typename ... Tparams> class CommandMethod { public:
	function<Tret(Program*,Tparams ...)> func;
	CommandMethodTemplate<Tparams...> templ;
	CommandMethod(function<Tret(Program*, Tparams ...)> newfunc);
	CommandFunc<Tret> getfunc(CommandFunc<Tparams> ... params);
};

template <typename Type> class CommandVar { public:
	Type* ref;
	bool unique;
	CommandVar(Type* newref);
	CommandVar(Type val);
	~CommandVar();
	CommandFunc<Type> getfunc();
	CommandVoidFunc setfunc(CommandFunc<Type> newval);
};

class Command { public:
	CommandVoidFunc func;
	Program* program;
	Command(Program* newprogram, istream& ifile);
	template <typename Type> CommandFunc<Type> parse(istream& ifile);
	template <typename Type> CommandFunc<Type> parse_const(istream& ifile);
	
	template <typename Tret, typename Tparamfirst, typename ... Tparamsleft, typename ... Tparams> CommandFunc<Tret>
	parse_func(istream& ifile, CommandMethodTemplate<Tparamfirst,Tparamsleft...> templ,
	CommandMethod<Tret,Tparams...,Tparamfirst,Tparamsleft...> method, CommandFunc<Tparams> ... paramfuncs);
	
	template <typename Tret, typename ... Tparams> CommandFunc<Tret>
	parse_func(istream& ifile, CommandMethodTemplate<> templ,
	CommandMethod<Tret,Tparams...> method, CommandFunc<Tparams> ... paramfuncs);
	
	template <typename Type> CommandFunc<Type> get_var(string id);
	template <typename Type> CommandFunc<Type> get_method(istream& ifile, string id);
	CommandVoidFunc set_var(istream& ifile, string id);
};

template <typename T> using Varlist = unordered_map<string,CommandVar<T> >;

class Program { public:
	World* world;
	ostream* out;
	ostream* errout;
	vector<Command> lines;
	Varlist<double> doublevars;
	Varlist<int> intvars;
	Varlist<string> stringvars;
	//CommandMethod<void,ivec3,Entity*> worldsummon;
	CommandMethod<void,double,string,int> worldsetblock;
	CommandMethod<void,double> printd;
	CommandMethod<void,string> prints;
	Program(World* world, ostream* newout, ostream* newerrout, istream& ifile);
	void run();
};



#endif
