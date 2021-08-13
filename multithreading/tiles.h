#ifndef MULTI_TILES_H
#define MULTI_TILES_H

#include "base/classes.h"
#include "base/tiles.h"
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
	T pop_front(bool* sucess);
	bool empty();
};

class ThreadedTileLoader : public TileLoader {
public:
	static const int num_threads = 1;
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
	int tick_ms;
	World* world;
	
	ThreadedTileLoader(World* world);
	~ThreadedTileLoader();
	void request_load_tile(ivec3 pos);
	void request_del_tile(ivec3 pos);
	void close();
};


class LoadingThread {
	ThreadedTileLoader* parent;
	int index;
public:
	LoadingThread(int newindex, ThreadedTileLoader* newparent);
	void operator()();
};

class DeletingThread {
	ThreadedTileLoader* parent;
	int index;
public:
	DeletingThread(int newindex, ThreadedTileLoader* newparent);
	void operator()();
};

class RenderingThread {
	ThreadedTileLoader* parent;
	bool busy = true;
	int times = 0;
	double start_time;
public:
	RenderingThread(ThreadedTileLoader* newparent);
	void operator()();
};

class TickThread {
	ThreadedTileLoader* parent;
	int ms;
public:
	TickThread(ThreadedTileLoader* newparent);
	void operator()();
};

#endif
