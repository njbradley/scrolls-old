#ifndef MULTI_PHYSICS
#define MULTI_PHYSICS

#include "scrolls/physics.h"
#include <thread>

#ifdef _WIN32

#include <windef.h>

class TimedTickRunner : public TickRunner { public:
	PLUGIN_HEAD(TimedTickRunner);
	HANDLE timerhandle;
	
	TimedTickRunner(World* world, float deltatime);
	~TimedTickRunner();
};

#else

struct TickThread {
	World* world;
	float goal_deltatime;
	bool running;
	void operator()();
};

class TimedTickRunner : public TickRunner { public:
	PLUGIN_HEAD(TimedTickRunner);
	std::thread tickthread;
	
	TimedTickRunner(World* world, float deltatime);
	~TimedTickRunner();
};

#endif

#endif
