#ifndef DWARF_DEBUG_H
#define DWARF_DEBUG_H

#include "scrolls/debug.h"

class DwarfDebugger: public Debugger { public:
	virtual void setup_backtrace();
};

#endif
