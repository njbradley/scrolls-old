#ifndef DWARF_DEBUG_H
#define DWARF_DEBUG_H

#include "scrolls/debug.h"

class DwarfDebugger: public Debugger { public:
	PLUGIN_HEAD(DwarfDebugger);
	
	DwarfDebugger();
	virtual void setup_backtrace();
};

#endif
