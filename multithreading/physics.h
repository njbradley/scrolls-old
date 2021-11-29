#ifndef MULTI_PHYSICS
#define MULTI_PHYSICS

#include "scrolls/physics.h"
#include <thread>



class TimedTickRunner : public TickRunner { public:
	PLUGIN_HEAD(TimedTickRunner);
	
	TimedTickRunner(World* world, float deltatime);
	~TimedTickRunner();
};


#endif
