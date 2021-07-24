#ifndef DWARF_DEBUG_H
#define DWARF_DEBUG_H

#include "base/debug.h"

class DwarfDebugger: public Debugger { public:
	virtual void setup_backtrace();
};


#endif
