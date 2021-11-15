#include "physics.h"
#include "scrolls/world.h"

#ifdef _WIN32

#include <windows.h>

static World* global_world;
static float global_deltatime;

void __stdcall
TimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired) {
	// static double lastTime = getTime();
	// static int times = 0;
  // times ++;
  // double curtime = getTime();
  // if (int(lastTime) < int(curtime)) {
  //   cout << times << endl;
  //   times = 0;
  // }
	// lastTime = curtime;
	
  global_world->tick(getTime(), global_deltatime);
}

TimedTickRunner::TimedTickRunner(World* world, float deltatime): TickRunner(world, deltatime) {
	global_world = world;
	global_deltatime = deltatime;
	int milliseconds = int(deltatime*1000);
  CreateTimerQueueTimer(&timerhandle, NULL, TimerCallback, NULL, milliseconds, milliseconds, WT_EXECUTEDEFAULT );
}

TimedTickRunner::~TimedTickRunner() {
  DeleteTimerQueueTimer(NULL, timerhandle, NULL);
}

#else

void TickThread::operator()() {
	double curtime = getTime();
	double lasttime = curtime;
	while (running) {
		curtime = getTime();
		double deltatime = curtime - lasttime;
		lasttime = curtime;
		world->tick(curtime, World::tick_deltatime);
		
		int ms = (deltatime) * 1000;
		if (ms < 35) {
			std::this_thread::sleep_for(std::chrono::milliseconds(35 - ms));
		}
		// cout << getTime() - start_time << endl;
	}
}

TimedTickRunner::TimedTickRunner(World* world, float deltatime): TickRunner(world, deltatime),
tickthread(TickThread{world, deltatime, true}) {
	
}

TimedTickRunner::~TimedTickRunner() {
	tickthread.join();
}

#endif

EXPORT_PLUGIN(TimedTickRunner);
