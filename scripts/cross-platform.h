#ifndef CROSS_PLATFORM
#define CROSS_PLATFORM

#ifdef _WIN32
#include "win.h"
#else
#include "unix.h"
#endif

#endif
