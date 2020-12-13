#ifndef MULTITHREADING_PREDEF
#define MULTITHREADING_PREDEF

#include "classes.h"

#include <vector>
#include <cmath>
#include <cstdio>
#include <limits>
#include <chrono>
#include <thread>
#include <mutex>
#include <list>

using std::thread;

class ThreadJob {
public:
	ivec3 pos;
	Tile* result;
	bool complete;
	std::timed_mutex lock;
	ThreadJob(ivec3 npos);
};

template <typename T>
class JobQueue {
	std::list<T> queue;
	std::mutex getlock;
public:
	void push_back(T newitem);
	T pop_front();
	bool empty();
};

class ThreadManager {
public:
	static const int num_threads = 3;
	JobQueue<ivec3> load_queue;
	JobQueue<ivec3> del_queue;
	ThreadJob* loading[num_threads];
	ivec3* deleting[num_threads];
	bool load_running[num_threads];
	bool del_running[num_threads];
	bool rendering;
	bool render_running;
	std::thread loading_threads[num_threads];
	std::thread deleting_threads[num_threads];
	std::thread rendering_thread;
	std::thread tick_thread;
	int render_num_verts = 0;
	int tick_ms;
	ThreadManager();
	bool add_loading_job(ivec3 pos);
	bool add_deleting_job(ivec3 pos);
	vector<Tile*> get_loaded_tiles();
	void close();
};


class LoadingThread {
	ThreadManager* parent;
	GLFWwindow* window;
	int index;
public:
	LoadingThread(GLFWwindow* window, int newindex, ThreadManager* newparent);
	void operator()();
};

class DeletingThread {
	ThreadManager* parent;
	GLFWwindow* window;
	int index;
public:
	DeletingThread(GLFWwindow* window, int newindex, ThreadManager* newparent);
	void operator()();
};

class RenderingThread {
	ThreadManager* parent;
	GLFWwindow* window;
	bool busy = true;
	int times = 0;
	double start_time;
public:
	RenderingThread(GLFWwindow* window, ThreadManager* newparent);
	void operator()();
};

class TickThread {
	ThreadManager* parent;
	GLFWwindow* window;
	int ms;
public:
	TickThread(GLFWwindow* window, ThreadManager* newparent);
	void operator()();
};

#endif
