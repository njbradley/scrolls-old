#ifndef GDB_DEBUG_H
#define GDB_DEBUG_H

#include "scrolls/debug.h"

class GDBDebugger: public Debugger { public:
	PLUGIN_HEAD(GDBDebugger);
	
	GDBDebugger();
};

#endif
