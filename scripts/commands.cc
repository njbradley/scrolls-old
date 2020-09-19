#ifndef COMMANDS
#define COMMANDS

#include "commands.h"
#include "world.h"
#include "blocks.h"
#include "mobs.h"
#include "blockdata.h"
#include "tiles.h"

template <typename Tret> CommandFunc<Tret> CommandNullFunc() {
	return CommandFunc<Tret>([] (Program* program) {Tret val; return val;});
}

template <> CommandFunc<void> CommandNullFunc() {
	return CommandFunc<void>([] (Program* program) {});
}

CommandVoidFunc throwerror(string message) {
	return [=](Program* program) {*program->errout << message << endl; };
}

CommandVoidFunc printmessage(string message) {
	return [=](Program* program) {*program->out << message << endl; };
}

bool is_string_char(char c) {
	return c != ' ' and c != '+' and c != '-' and c != '/' and c != '*' and c != ';' and c != '(' and c != ')' and c != ',';
}

template <typename T> void debuglog(T val) {
	cout << val << endl;
}

template <> void debuglog<vec3>(vec3 val) {
	cout << val.x << ' ' << val.y << ' ' << val.z << endl;
}

template <> void debuglog<ivec3>(ivec3 val) {
	cout << val.x << ' ' << val.y << ' ' << val.z << endl;
}



template <typename Type> CommandConst<Type>::CommandConst(Type newval): value(newval) {
	
}

template <typename Type> CommandFunc<Type> CommandConst<Type>::getfunc() {
	Type val = value;
	return [=] (Program* program) {
		//cout << "const return ";
		//debuglog<Type>(val);
		return val;
	};
}





template <typename Tret, typename ... Tparams> CommandMethod<Tret,Tparams...>::CommandMethod(function<Tret(Program*,Tparams ...)> newfunc): func(newfunc) {
	
}

template <typename Tret, typename ... Tparams> CommandFunc<Tret> CommandMethod<Tret,Tparams...>::getfunc(CommandFunc<Tparams> ... params) {
	function<Tret(Program*,Tparams ...)> capturefunc = func;
	return [=] (Program* program) {
		return capturefunc(program, (params(program)) ...);
	};
}

// '<lambda closure object>CommandMethod<T, Tparams>::
// getfunc(CommandFunc<Tparams>...) [with Tret = int;
// Tparams = {double}; CommandFunc<Type> = std::function<int(Program*)>]
// ::<lambda(Program*)>{std::function<int(Program*, double)>((*(const std::function<int(Program*, double)>*)(& capturefunc)))
// 	, std::function<double(Program*)>((*(const std::function<double(Program*)>*)(& params#0)))}'
// CommandMethod<T, Tparams>::getfunc(CommandFunc<Tparams>...)
// [with Tret = int; Tparams = {double}; CommandFunc<Type> = std::function<int(Program*)>]
// ::<lambda(Program*)>' to 'CommandFunc<int> {aka std::function<int(Program*)>}

template <typename Type> CommandVar<Type>::CommandVar(Type* newref): ref(newref), unique(false) {
	
}

template <typename Type> CommandVar<Type>::CommandVar(): ref(nullptr), unique(false) {
	
}

template <typename Type> CommandVar<Type>::CommandVar(Type val): unique(true) {
	ref = new Type(val);
	cout << "generating new ref " << ref << endl;
}

template <typename Type> CommandVar<Type>::~CommandVar() {
	if (unique) {
		cout << "deleting ref " << ref << endl;
		delete ref;
	}
}

template <typename Type> CommandFunc<Type> CommandVar<Type>::getfunc() {
	Type* reference = ref;
	return [=] (Program* program) {
		//cout << "return var ";
		//debuglog<Type>(*reference);
		return *reference;
	};
}

template <typename Type> CommandVoidFunc CommandVar<Type>::setfunc(CommandFunc<Type> newval) {
	Type* reference = ref;
	return [=] (Program* program) {
		*reference = newval(program);
		///cout << "new val ";
		//debuglog<Type>(*reference);
	};
}



template <typename Type> CommandOperator<Type>::CommandOperator(char newoper): oper(newoper) {
	
}

template <typename Type> CommandFunc<Type> CommandOperator<Type>::getfunc(CommandFunc<Type> first, CommandFunc<Type> second) {
	if (oper == '+') {
		return [=] (Program* program) {
			return first(program) + second(program);
		};
	}
	if (oper == '-') {
		return [=] (Program* program) {
			return first(program) - second(program);
		};
	}
	if (oper == '*') {
		return [=] (Program* program) {
			return first(program) * second(program);
		};
	}
	if (oper == '/') {
		return [=] (Program* program) {
			return first(program) / second(program);
		};
	}
	if (oper == '%') {
		return [=] (Program* program) {
			return first(program) % second(program);
		};
	}
	return CommandNullFunc<Type>();
}

template <> CommandFunc<double> CommandOperator<double>::getfunc(CommandFunc<double> first, CommandFunc<double> second) {
	if (oper == '+') {
		return [=] (Program* program) {
			return first(program) + second(program);
		};
	}
	if (oper == '-') {
		return [=] (Program* program) {
			return first(program) - second(program);
		};
	}
	if (oper == '*') {
		return [=] (Program* program) {
			return first(program) * second(program);
		};
	}
	if (oper == '/') {
		return [=] (Program* program) {
			return first(program) / second(program);
		};
	}
	return CommandNullFunc<double>();
}

template <> CommandFunc<vec3> CommandOperator<vec3>::getfunc(CommandFunc<vec3> first, CommandFunc<vec3> second) {
	if (oper == '+') {
		return [=] (Program* program) {
			return first(program) + second(program);
		};
	}
	if (oper == '-') {
		return [=] (Program* program) {
			return first(program) - second(program);
		};
	}
	if (oper == '*') {
		return [=] (Program* program) {
			return first(program) * second(program);
		};
	}
	if (oper == '/') {
		return [=] (Program* program) {
			return first(program) / second(program);
		};
	}
	return CommandNullFunc<vec3>();
}

template <> CommandFunc<string> CommandOperator<string>::getfunc(CommandFunc<string> first, CommandFunc<string> second) {
	if (oper == '+') {
		return [=] (Program* program) {
			return first(program) + second(program);
		};
	}
	return CommandNullFunc<string>();
}







template <typename Type> CommandFunc<Type> Command::parse(istream& ifile) {
	CommandFunc<Type> ret;
	vector<CommandFunc<Type> > funcs;
	vector<char> opers;
	bool isoper = true;
	
	
	while (isoper) {
		char type = ifile.peek();
		while (type == ' ' and !ifile.eof()) {
			ifile.get();
			type = ifile.peek();
		}
		if (type == '$') {
			string id;
			char lett;
			ifile.get();
			
			while (is_string_char(ifile.peek()) and !ifile.eof()) {
				id.push_back(ifile.get());
			}
			lett = ifile.peek();
			while (lett == ' ' and !ifile.eof()) {
				ifile.get();
				lett = ifile.peek();
			}
			
			if (lett == '(') {
				ifile.get();
				funcs.push_back(get_method<Type>(ifile, id));
			} else {
				funcs.push_back(get_var<Type>(id));
			}
		} else {
			CommandFunc<Type> func = parse_const<Type>(ifile);
			funcs.push_back(func);
		}
		
		char lett = ifile.peek();
		while (lett == ' ' and !ifile.eof()) {
			ifile.get();
			lett = ifile.peek();
		}
		
		isoper = lett == '+' or lett == '-' or lett == '*' or lett == '/' or lett == '%';
		if (isoper) {
			opers.push_back(lett);
			ifile.get();
		}
	}
	
	vector<vector<char> > oper_order {{'*', '/', '%'}, {'+', '-'}};
	
	for (vector<char>& search_opers : oper_order) {
		for (int i = 0; i < opers.size(); i ++) {
			for (char search_oper : search_opers) {
				if (opers[i] == search_oper) {
					CommandOperator<Type> coper(search_oper);
					funcs[i] = coper.getfunc(funcs[i], funcs[i+1]);
					funcs.erase(funcs.begin() + i+1);
					opers.erase(opers.begin() + i);
					i --;
					break;
				}
			}
		}
	}
	
	return funcs[0];
}


template <typename Type> CommandFunc<Type> Command::parse_const(istream& ifile) {
	Type val;
	ifile >> val;
	CommandConst<Type> comconst(val);
	return comconst.getfunc();
}

template <> CommandFunc<ivec3> Command::parse_const<ivec3>(istream& ifile) {
	ivec3 val;
	ifile >> val.x >> val.y >> val.z;
	CommandConst<ivec3> comconst(val);
	return comconst.getfunc();
}

template <> CommandFunc<vec3> Command::parse_const<vec3>(istream& ifile) {
	vec3 val;
	ifile >> val.x >> val.y >> val.z;
	CommandConst<vec3> comconst(val);
	return comconst.getfunc();
}

template <> CommandFunc<string> Command::parse_const<string>(istream& ifile) {
	string val;
	while ((is_string_char(ifile.peek()) or ifile.peek() == '-') and !ifile.eof()) {
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

template <typename Tret, typename ... Tparams> CommandFunc<Tret>
Command::parse_func(istream& ifile, CommandMethodTemplate<> templ,
CommandMethod<Tret,Tparams...> method, CommandFunc<Tparams> ... paramfuncs) {
	return method.getfunc(paramfuncs...);
}


template <typename Type> CommandFunc<Type> Command::get_var(string id) {
	*program->errout << "ERR: this type is not supported as a variable" << endl;
	program->error = true;
	return CommandNullFunc<Type>();
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

template <> CommandFunc<vec3> Command::get_var<vec3>(string id) {
	return program->vec3vars.at(id).getfunc();
}



CommandVoidFunc Command::set_var(istream& ifile, string id) {
	if (program->intvars.find(id) != program->intvars.end()) {
		return program->intvars.at(id).setfunc(parse<int>(ifile));
	}
	if (program->doublevars.find(id) != program->doublevars.end()) {
		CommandFunc<double> func = parse<double>(ifile);
		return program->doublevars.at(id).setfunc(func);
	}
	if (program->stringvars.find(id) != program->stringvars.end()) {
		return program->stringvars.at(id).setfunc(parse<string>(ifile));
	}
	if (program->vec3vars.find(id) != program->vec3vars.end()) {
		return program->vec3vars.at(id).setfunc(parse<vec3>(ifile));
	}
	*program->errout << "ERR: no variable '" << id << "' found" << endl;
	program->error = true;
	return CommandNullFunc<void>();
}

void Command::new_var(istream& ifile, string type, string id) {
	if (type == "int") {
		program->intvars.emplace(id, 0);
	} else if (type == "float") {
		program->doublevars.emplace(id, 0.0);
	} else if (type == "string") {
		program->stringvars.emplace(id, string(""));
	} else if (type == "vec3") {
		program->vec3vars.emplace(id, vec3(0,0,0));
	} else {
		*program->errout << "ERR: no type called '" << type << "' " << endl;
		program->error = true;
	}
}
	
	

CommandVoidFunc Command::set_oper_var(istream& ifile, string id, char oper) {
	if (program->intvars.find(id) != program->intvars.end()) {
		CommandOperator<int> coper(oper);
		return program->intvars.at(id).setfunc(coper.getfunc(get_var<int>(id), parse<int>(ifile)));
	}
	if (program->doublevars.find(id) != program->doublevars.end()) {
		CommandOperator<double> coper(oper);
		CommandFunc<double> func = parse<double>(ifile);
		return program->doublevars.at(id).setfunc(coper.getfunc(get_var<double>(id),func));
	}
	if (program->stringvars.find(id) != program->stringvars.end()) {
		CommandOperator<string> coper(oper);
		return program->stringvars.at(id).setfunc(coper.getfunc(get_var<string>(id), parse<string>(ifile)));
	}
	if (program->vec3vars.find(id) != program->vec3vars.end()) {
		CommandOperator<vec3> coper(oper);
		return program->vec3vars.at(id).setfunc(coper.getfunc(get_var<vec3>(id), parse<vec3>(ifile)));
	}
}

template <typename Type> CommandFunc<Type> Command::get_method(istream& ifile, string id) {
	*program->errout << "ERR: no methods defined for this type" << endl;
	program->error = true;
	return CommandNullFunc<Type>();
}

template <> CommandFunc<void> Command::get_method<void>(istream& ifile, string id) {
	if (id == "world.setblock") {
		return parse_func<void>(ifile, program->worldsetblock.templ, program->worldsetblock);
	} else if (id == "world.summon") {
		return parse_func<void>(ifile, program->worldsummon.templ, program->worldsummon);
	} else if (id == "printd") {
		return parse_func<void>(ifile, program->printd.templ, program->printd);
	} else if (id == "prints") {
		return parse_func<void>(ifile, program->prints.templ, program->prints);
	} else if (id == "printv3") {
		return parse_func<void>(ifile, program->printv3.templ, program->printv3);
	} else if (id == "killall") {
		return parse_func<void>(ifile, program->killall.templ, program->killall);
	} else {
		*program->errout << "ERR: no method by the name of '" << id << "'" << endl;
		program->error = true;
		return CommandNullFunc<void>();
	}
}

template <> CommandFunc<int> Command::get_method<int>(istream& ifile, string id) {
	if (id == "round") {
		return parse_func<int>(ifile, program->dtoi.templ, program->dtoi);
	} else {
		*program->errout << "ERR: no method by the name of '" << id << "'" << endl;
		program->error = true;
		return CommandNullFunc<int>();
	}
}

template <> CommandFunc<ivec3> Command::get_method<ivec3>(istream& ifile, string id) {
	if (id == "round") {
		return parse_func<ivec3>(ifile, program->vec3toivec3.templ, program->vec3toivec3);
	} else {
		*program->errout << "ERR: no method by the name of '" << id << "'" << endl;
		program->error = true;
		return CommandNullFunc<ivec3>();
	}
}

Command::Command(Program* newprogram, istream& ifile): program(newprogram), func(CommandNullFunc<void>()) {
	string id;
	char lett;
	
	while (is_string_char(ifile.peek()) and !ifile.eof()) {
		id.push_back(ifile.get());
	}
	lett = ifile.peek();
	while (lett == ' ' and !ifile.eof()) {
		ifile.get();
		lett = ifile.peek();
	}
	if (lett == '(') {
		ifile.get();
		func = get_method<void>(ifile, id);
	} else if (lett == '=') {
		ifile.get();
		func = set_var(ifile, id);
	} else if (id == "int" or id == "float" or id == "string" or id == "vec3") {
		string type = id;
		id = "";
		while (is_string_char(ifile.peek()) and !ifile.eof()) {
			id.push_back(ifile.get());
		}
		lett = ifile.peek();
		while (lett == ' ' and !ifile.eof()) {
			ifile.get();
			lett = ifile.peek();
		}
		new_var(ifile, type, id);
		if (lett == '=') {
			ifile.get();
			func = set_var(ifile, id);
		}
	} else if (lett == '+' or lett == '-' or lett == '*' or lett == '/' or lett == '%') {
		char oper = ifile.get();
		if (ifile.peek() == '=') {
			ifile.get();
			func = set_oper_var(ifile, id, oper);
		} else {
			*program->errout << "ERR: did you mean '" << oper << "=' ?" << endl;
		}
	}
}










Program::Program(World* nworld, ostream* newout, ostream* newerrout): world(nworld), out(newout), errout(newerrout),
worldsummon([] (Program* program, vec3 pos, string entityname) {
	DisplayEntity* entity;
	if (entityname == "pig") entity = new Pig(program->world, pos);
	else if (entityname == "skeleton") entity = new Skeleton(program->world, pos);
  else if (entityname == "ent") entity = new Ent(program->world, pos);
	else if (entityname == "glofly") entity = new Glofly(program->world, pos);
	else if (entityname == "ghost") entity = new Ghost(program->world, pos);
	else entity = new Mob(program->world, pos, entityname);
	
	program->world->summon(entity);
}),
worldsetblock([] (Program* program, ivec3 pos, string blockname, int direction) {
	program->world->set(pos.x, pos.y, pos.z, blocks->names[blockname], direction);
}),
printd([] (Program* program, double val) {
	*program->out << val << endl;
}),
prints([] (Program* program, string val) {
	*program->out << val << endl;
}),
printv3([] (Program* program, vec3 val) {
	*program->out << val.x << ' ' << val.y << ' ' << val.z << endl;
}),
dtoi([] (Program* program, double val) {
	return int(val);
}),
vec3toivec3([] (Program* program, vec3 val) {
	return ivec3(val);
}),
killall([] (Program* program) {
	TileLoop loop(program->world);
	for (Tile* tile : loop) {
		tile->entities.clear();
	}
}),
doublevars({
	{"world.time", CommandVar<double>(&world->daytime)},
	{"me.health", CommandVar<double>()},
}),
vec3vars({
	{"me.pos", CommandVar<vec3>(nullptr)},
	{"me.vel", CommandVar<vec3>()},
}) {
	
}

void Program::parse_lines(istream& ifile) {
	error = false;
	string buf;
	while (!ifile.eof()) {
		lines.emplace_back(this, ifile);
		getline(ifile, buf, ';');
	}
}

void Program::run() {
	if (error) {
		*errout << "ERR: the program compiled with errors" << endl;
	} else {
		for (Command& command : lines) {
			command.func(this);
		}
	}
}

template class CommandVar<int>;
template class CommandVar<double>;
template class CommandVar<string>;
template class CommandVar<ivec3>;
template class CommandVar<vec3>;

#endif
