#ifndef COMMANDS_PREDEF
#define COMMANDS_PREDEF

#include "classes.h"

template <typename Tret> using CommandFunc = function<Tret(Program*)>;
typedef CommandFunc<void> CommandVoidFunc;
template <typename Tret> CommandFunc<Tret> CommandNullFunc();
template <> CommandFunc<void> CommandNullFunc();

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
	CommandVar();
	~CommandVar();
	CommandFunc<Type> getfunc();
	CommandVoidFunc setfunc(CommandFunc<Type> newval);
};

template <typename Type> class CommandOperator { public:
	char oper;
	CommandOperator(char newoper);
	CommandFunc<Type> getfunc(CommandFunc<Type> first, CommandFunc<Type> second);
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
	void new_var(istream& ifile, string type, string id);
	CommandVoidFunc set_oper_var(istream& ifile, string id, char oper);
};

template <typename T> using Varlist = unordered_map<string,CommandVar<T> >;

class Program { public:
	World* world;
	bool error;
	ostream* out;
	ostream* errout;
	vector<Command> lines;
	vector<string> history;
	Varlist<double> doublevars;
	Varlist<int> intvars;
	Varlist<string> stringvars;
	Varlist<vec3> vec3vars;
	CommandMethod<void,vec3,string> worldsummon;
	CommandMethod<void,ivec3,string,int> worldsetblock;
	CommandMethod<void,double> printd;
	CommandMethod<void,string> prints;
	CommandMethod<void,vec3> printv3;
	CommandMethod<int,double> dtoi;
	CommandMethod<ivec3,vec3> vec3toivec3;
	CommandMethod<void> killall;
	CommandMethod<void,string,int> give;
	CommandMethod<void,double,double> tp;
	Program(World* world, ostream* newout, ostream* newerrout);
	void parse_lines(istream& ifile);
	void run();
};


#endif
