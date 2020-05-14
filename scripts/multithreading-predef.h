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
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
class ThreadJob {
public:
	ivec3 pos;
	Tile* result;
	bool complete;
	ThreadJob(ivec3 npos);
};

class ThreadManager {
public:
	static const int num_threads = 3;
	ThreadJob* loading[num_threads];
	ivec3* deleting[num_threads];
	bool load_running[num_threads];
	bool del_running[num_threads];
	bool rendering;
	bool render_running;
	std::thread loading_threads[num_threads];
	std::thread deleting_threads[num_threads];
	std::thread rendering_thread;
	int render_num_verts = 0;
	ThreadManager(GLFWwindow* window);
	bool add_loading_job(ivec3 pos);
	bool add_deleting_job(ivec3 pos);
	vector<Tile*> get_loaded_tiles();
	void close();
};

ThreadManager* threadmanager;


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
public:
	RenderingThread(GLFWwindow* window, ThreadManager* newparent);
	void operator()();
};
		
#endif
