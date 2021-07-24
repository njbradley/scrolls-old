#ifndef BASE_DEBUG_H
#define BASE_DEBUG_H

#include "classes.h"
#include "plugins.h"

class Debugger { public:
	PLUGIN_HEAD(Debugger, ());
	
	Debugger() {};
	virtual ~Debugger() {};
	virtual void setup_backtrace() {};
};

class Logger { public:
	PLUGIN_HEAD(Logger, ());
	
	Logger() {};
	virtual ~Logger() {};
	virtual ostream& log(int page);
	virtual ostream& operator[](int page);
};

extern Plugin<Debugger> debugger;
extern Plugin<Logger> logger;

#endif
