#ifndef COMMANDS_PREDEF
#define COMMANDS_PREDEF

#include "classes.h"

typedef function<void(World*,ostream&)> CommandFunc;

class Command { public:
	CommandFunc func;
	Command(istream& ifile);
};

class Program { public:
	vector<CommandFunc> lines;
	Program(istream& ifile);
	void run(World* world);
};



#endif
