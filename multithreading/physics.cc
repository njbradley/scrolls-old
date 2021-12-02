#include "physics.h"
#include "scrolls/world.h"

#include <mutex>
#include <cstring>

static World* global_world;
static float global_deltatime;
std::mutex tick_lock;

void timer_callback() {
	// static int times = 0;
	// static double last_time = getTime();
	// double this_time = getTime();
	// times ++;
	// if (int(last_time) != int(this_time)) {
	// 	cout << " times " << times << endl;
	// 	times = 0;
	// }
	// last_time = this_time;
	
	// static double last_time = getTime();
	// double this_time = getTime();
	// cout << (this_time - last_time) * 1000 << endl;
	// last_time = this_time;
	
	std::unique_lock<std::mutex> lock(tick_lock, std::try_to_lock);
	if(lock.owns_lock()){
		global_world->tick(getTime(), global_deltatime);
	} else {
		cout << "dropping tick " << endl;
	}
}

#ifdef _WIN32
#include <windows.h>

HANDLE timerhandle;

void __stdcall
TimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired) {
	timer_callback();
}

void create_timer() {
	int milliseconds = int(global_deltatime*1000);
	CreateTimerQueueTimer(&timerhandle, NULL, TimerCallback, NULL, milliseconds, milliseconds, WT_EXECUTEDEFAULT );
}

void del_timer() {
	DeleteTimerQueueTimer(NULL, timerhandle, NULL);
}

#else
#include <signal.h>

timer_t timerid;

void TimerCallback(union sigval val) {
	timer_callback();
}

void create_timer() {
	
	struct sigevent sig;
	memset(&sig, '\0', sizeof(struct sigevent));
	sig.sigev_notify = SIGEV_THREAD;
	sig.sigev_notify_function = TimerCallback;
	sig.sigev_value.sival_int = 0;
	
  if (!timer_create(CLOCK_MONOTONIC, &sig, &timerid)) {
		struct itimerspec in;
		memset(&in, '\0', sizeof(struct itimerspec));
		in.it_interval.tv_sec = 0;
		in.it_interval.tv_nsec = long(global_deltatime * 1000000000);
		in.it_value.tv_sec = 0;
		in.it_value.tv_nsec = in.it_interval.tv_nsec;
		//issue the periodic timer request here.
		if (timer_settime(timerid, 0, &in, nullptr)) {
			perror("setting time failed");
		}
	} else {
		perror("creating timer failed");
		cout << errno << ' ' << EINVAL << endl;
	}
}

void del_timer() {
	struct itimerspec in;
	memset(&in, '\0', sizeof(struct itimerspec));
	if (timer_settime(timerid, 0, &in, nullptr)) {
		perror("setting (disable) time failed");
	}
	if (timer_delete(timerid)) {
		perror("deleting time failed");
	}
}

#endif

TimedTickRunner::TimedTickRunner(World* world, float deltatime): TickRunner(world, deltatime) {
	global_world = world;
	global_deltatime = deltatime;
	create_timer();
}

TimedTickRunner::~TimedTickRunner() {
	std::lock_guard<std::mutex> guard(tick_lock);
	del_timer();
}


EXPORT_PLUGIN(TimedTickRunner);
