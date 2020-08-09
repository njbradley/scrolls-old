#ifndef COMMANDS
#define COMMANDS

#include "commands-predef.h"
#include "world-predef.h"
#include "blocks-predef.h"
#include "mobs-predef.h"

CommandVoidFunc throwerror(string message) {
	return [=](Program* program) {*program->errout << message << endl; };
}

CommandVoidFunc printmessage(string message) {
	return [=](Program* program) {*program->out << message << endl; };
}






template <typename Type> CommandConst<Type>::CommandConst(Type newval): value(newval) {
	
}

template <typename Type> CommandFunc<Type> CommandConst<Type>::getfunc() {
	Type val = value;
	return [=] (Program* program) {
		cout << "get const" << endl;
		cout << val << endl;
		return val;
	};
}





template <typename Tret, typename ... Tparams> CommandMethod<Tret,Tparams...>::CommandMethod(function<Tret(Program*,Tparams ...)> newfunc): func(newfunc) {
	
}

template <typename Tret, typename ... Tparams> CommandFunc<Tret> CommandMethod<Tret,Tparams...>::getfunc(CommandFunc<Tparams> ... params) {
	return [=] (Program* program) {
		func(program, (params(program)) ...);
	};
}

template <typename Type> CommandVar<Type>::CommandVar(Type* newref): ref(newref), unique(false) {
	
}

template <typename Type> CommandVar<Type>::CommandVar(Type val): unique(true) {
	ref = new Type(val);
}

template <typename Type> CommandVar<Type>::~CommandVar() {
	if (unique) {
		delete ref;
	}
}

template <typename Type> CommandFunc<Type> CommandVar<Type>::getfunc() {
	return [=] (Program* program) {
		return *ref;
	};
}

template <typename Type> CommandVoidFunc CommandVar<Type>::setfunc(CommandFunc<Type> newval) {
	return [=] (Program* program) {
		cout << "hi" << endl;
		cout << newval(program) << endl;
		*ref = newval(program);
	};
}












template <typename Type> CommandFunc<Type> Command::parse(istream& ifile) {
	char type = ifile.peek();
	while (type == ' ') {
		ifile.get();
		type = ifile.peek();
	}
	if (type == '$') {
		string id;
		char lett;
		ifile.get();
		
		while ((lett = ifile.peek()) != ' ' and lett != '(' and lett != '+' and lett != '=' and lett != '-' and lett != ';' and !ifile.eof()) {
			id.push_back(ifile.get());
		}
		cout << "id " << id << endl;
		while (lett == ' ' and !ifile.eof()) {
			ifile.get();
			lett = ifile.peek();
		}
		
		if (lett == '(') {
			ifile.get();
			return get_method<Type>(ifile, id);
		} else {
			cout << "getvar " << endl;
			return get_var<Type>(id);
		}
	} else {
		cout << "const" << endl;
		CommandFunc<Type> func = parse_const<Type>(ifile);
		cout << func(program) << endl;
		return func;
	}
}


template <typename Type> CommandFunc<Type> Command::parse_const(istream& ifile) {
	Type val;
	ifile >> val;
	cout << val << endl;
	CommandFunc<Type> func;
	if (true) {
		CommandConst<Type> comconst(val);
		func = comconst.getfunc();
		cout << func(program) << endl;
	}
	
	cout << func(program) << endl;
	return func;
}

// template <> CommandFunc<ivec3> Command::parse_const<ivec3>(istream& ifile) {
// 	ivec3 val;
// 	ifile >> val.x >> val.y >> val.z;
// 	CommandConst<ivec3> comconst(val);
// 	return comconst.getfunc();
// }
//
// template <> CommandFunc<vec3> Command::parse_const<vec3>(istream& ifile) {
// 	vec3 val;
// 	ifile >> val.x >> val.y >> val.z;
// 	CommandConst<vec3> comconst(val);
// 	return comconst.getfunc();
// }

template <> CommandFunc<string> Command::parse_const<string>(istream& ifile) {
	string val;
	while (ifile.peek() != ' ' and ifile.peek() != ')' and ifile.peek() != ',' and ifile.peek() != ';') {
		val.push_back(ifile.get());
	}
	CommandConst<string> comconst(val);
	return comconst.getfunc();
}


template <typename Tret, typename Tparamfirst, typename ... Tparamsleft, typename ... Tparams> CommandFunc<Tret>
Command::parse_func(istream& ifile, CommandMethodTemplate<Tparamfirst,Tparamsleft...> templ,
CommandMethod<Tret,Tparams...,Tparamfirst,Tparamsleft...> method, CommandFunc<Tparams> ... paramfuncs) {
	CommandFunc<Tparamfirst> funcval = parse<Tparamfirst>(ifile);
	char lett;
	ifile >> lett;
	if (lett != ',' and lett != ')') {
		*program->errout << "ERR: expected ',' or ')' but got '" << lett << "'" << endl;
	}
	return parse_func<Tret>(ifile, CommandMethodTemplate<Tparamsleft...>(), method, paramfuncs..., funcval);
}

// template <typename Tret, typename ... Tparams, int progress, typename Tparamfirst, typename ... Tparamsleft> CommandFunc<Tret>
// Command::parse_func(istream& ifile, CommandMethod<Tret,Tparams...,Tparamfirst,Tparamsleft...> method, CommandFunc<Tparams> ... paramfuncs) {
// 	CommandFunc<Tparamfirst> funcval = parse<Tparamfirst>(ifile);
// 	return parse_func<Tret, Tparams..., Tparamfirst, progress+1, Tparamsleft...>(ifile, paramfuncs..., funcval);
// }

template <typename Tret, typename ... Tparams> CommandFunc<Tret>
Command::parse_func(istream& ifile, CommandMethodTemplate<> templ,
CommandMethod<Tret,Tparams...> method, CommandFunc<Tparams> ... paramfuncs) {
	return method.getfunc(paramfuncs...);
}

// template <typename Tret, typename ... Tparams, int progress> CommandFunc<Tret>
// Command::parse_func(istream& ifile, CommandMethod<Tret,Tparams...> method, CommandFunc<Tparams> ... paramfuncs) {
// 	return method.getfunc(paramfuncs...);
// }


template <typename Type> CommandFunc<Type> Command::get_var(string id) {
	*program->errout << "ERR: this type is not supported as a variable" << endl;
	return CommandFunc<Type>();
}

template <> CommandFunc<int> Command::get_var<int>(string id) {
	return program->intvars.at(id).getfunc();
}

template <> CommandFunc<double> Command::get_var<double>(string id) {
	return program->doublevars.at(id).getfunc();
}

template <> CommandFunc<string> Command::get_var<string>(string id) {
	return program->stringvars.at(id).getfunc();
}



CommandVoidFunc Command::set_var(istream& ifile, string id) {
	if (program->intvars.find(id) != program->intvars.end()) {
		return program->intvars.at(id).setfunc(parse<int>(ifile));
	}
	if (program->doublevars.find(id) != program->doublevars.end()) {
		CommandFunc<double> func = parse<double>(ifile);
		cout << "set_var" << endl;
		cout << func(program) << endl;
		return program->doublevars.at(id).setfunc(func);
	}
	if (program->stringvars.find(id) != program->stringvars.end()) {
		return program->stringvars.at(id).setfunc(parse<string>(ifile));
	}
}


template <typename Type> CommandFunc<Type> Command::get_method(istream& ifile, string id) {
	*program->errout << "ERR: no methods defined for this type" << endl;
	return CommandFunc<Type>();
}

template <> CommandFunc<void> Command::get_method<void>(istream& ifile, string id) {
	if (id == "world.setblock") {
		return parse_func<void>(ifile, program->worldsetblock.templ, program->worldsetblock);
	} else if (id == "printd") {
		return parse_func<void>(ifile, program->printd.templ, program->printd);
	} else if (id == "prints") {
		return parse_func<void>(ifile, program->prints.templ, program->prints);
	} else {
		return CommandFunc<void>();
	}
}

Command::Command(Program* newprogram, istream& ifile): program(newprogram) {
	string id;
	char lett;
	
	while ((lett = ifile.peek()) != ' ' and lett != '(' and lett != '+' and lett != '=' and lett != '-') {
		id.push_back(ifile.get());
	}
	cout << id << endl;
	while (lett == ' ') {
		ifile.get();
		lett = ifile.peek();
	}
	cout << lett << endl;
	if (lett == '(') {
		ifile.get();
		func = get_method<void>(ifile, id);
	} else if (lett == '=') {
		ifile.get();
		func = set_var(ifile, id);
	}
}










Program::Program(World* nworld, ostream* newout, ostream* newerrout, istream& ifile): world(nworld), out(newout), errout(newerrout),
worldsetblock([] (Program* program, double pos, string blockname, int direction) {
	//program->world->set(pos.x, pos.y, pos.z, blocks->names[blockname], direction);
}),
printd([] (Program* program, double val) {
	cout << "printd function" << endl;
	cout << val << " val " << endl;
	*program->out << val << endl;
}),
prints([] (Program* program, string val) {
	*program->out << val << endl;
}),
doublevars({
	{"world.time", CommandVar<double>(&world->daytime)},
}) {
	string buf;
	while (!ifile.eof()) {
		lines.emplace_back(this, ifile);
		getline(ifile, buf, ';');
	}
}

void Program::run() {
	for (Command& command : lines) {
		command.func(this);
	}
}





#endif
