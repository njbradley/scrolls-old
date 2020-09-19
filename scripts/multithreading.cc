#ifndef MULITTHREADING
#define MULITTHREADING

#include "multithreading.h"

#include "classes.h"
#include "ui.h"
#include "rendervec.h"
#include "world.h"
#include "tiles.h"

#include <atomic>

ThreadManager* threadmanager;

ThreadJob::ThreadJob(ivec3 npos): pos(npos), result(nullptr), complete(false) {
	
}

ThreadManager::ThreadManager(GLFWwindow* window) {
	rendering = false;
	render_running = true;
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	GLFWwindow* renderwindow = glfwCreateWindow( screen_x, screen_y, "back", nullptr, window);
	
	rendering_thread = thread(RenderingThread(renderwindow, this));
	
	// glfwWindowHint(GLFW_SAMPLES, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	// GLFWwindow* tickwindow = glfwCreateWindow( screen_x, screen_y, "back", nullptr, window);
	//
	tick_thread = thread(TickThread(nullptr, this));
	
	for (int i = 0; i < num_threads; i ++) {
		loading[i] = nullptr;
		deleting[i] = nullptr;
		load_running[i] = true;
		del_running[i] = true;
		// glfwWindowHint(GLFW_SAMPLES, 4);
		// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
		// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		// glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		// GLFWwindow* loadingwindow = glfwCreateWindow( screen_x, screen_y, "back", nullptr, window);
		
		// glfwWindowHint(GLFW_SAMPLES, 4);
		// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
		// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		// glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		// GLFWwindow* deletingwindow = glfwCreateWindow( screen_x, screen_y, "back", nullptr, window);
		loading_threads[i] = thread(LoadingThread(nullptr, i, this));
		deleting_threads[i] = thread(DeletingThread(nullptr, i, this));
	}
}

bool ThreadManager::add_loading_job(ivec3 pos) {
	for (int i = 0; i < num_threads; i ++) {
		if (loading[i] == nullptr) {
			loading[i] = new ThreadJob(pos);
			return true;
		}
	}
	return false;
}

bool ThreadManager::add_deleting_job(ivec3 pos) {
	for (int i = 0; i < num_threads; i ++) {
		if (deleting[i] == nullptr) {
			deleting[i] = new ivec3(pos);
			return true;
		}
	}
	return false;
}

vector<Tile*> ThreadManager::get_loaded_tiles() {
	vector<Tile*> tiles;
	for ( int i = 0; i < num_threads; i ++) {
		if (loading[i] != nullptr and loading[i]->complete and loading[i]->lock.try_lock_for(std::chrono::seconds(1))) {
			tiles.push_back(loading[i]->result);
			loading[i]->lock.unlock();
			delete loading[i];
			loading[i] = nullptr;
		}
	}
	return tiles;
}

void ThreadManager::close() {
	render_running = false;
	//cout << "joining rendering thread" << endl;
	rendering_thread.join();
	tick_thread.join();
	for (int i = 0; i < num_threads; i ++) {
		load_running[i] = false;
		//cout << "joining load " << i << endl;
		loading_threads[i].join();
		del_running[i] = false;
		//cout << "joining del " << i << endl;
		deleting_threads[i].join();
	}
}
		




LoadingThread::LoadingThread(GLFWwindow* nwindow, int newindex, ThreadManager* newparent): window(nwindow), index(newindex), parent(newparent) {
	
}

void LoadingThread::operator()() {
	//glfwMakeContextCurrent(window);
	while (parent->load_running[index]) {
		//cout << parent->loading[index] << endl;
		
		if (world != nullptr) {
			if (parent->loading[index] != nullptr and parent->loading[index]->lock.try_lock_for(std::chrono::seconds(1))) {
				//cout << parent->loading[index] << endl;
				parent->loading[index]->result = new Tile(parent->loading[index]->pos, world);
				parent->loading[index]->complete = true;
				parent->loading[index]->lock.unlock();
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		//cout << "hello from thread " << index << endl;
	}
	cout << "loading thread " << index << " exited" << endl;
}


DeletingThread::DeletingThread(GLFWwindow* nwindow, int newindex, ThreadManager* newparent): window(nwindow), index(newindex), parent(newparent) {
	
}

void DeletingThread::operator()() {
	//glfwMakeContextCurrent(window);
	while (parent->del_running[index]) {
		if (parent->deleting[index] != nullptr) {
			world->del_chunk(*parent->deleting[index], true);
			delete parent->deleting[index];
			parent->deleting[index] = nullptr;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	cout << "deleting thread " << index << " exited" << endl;
}



RenderingThread::RenderingThread(GLFWwindow* nwindow, ThreadManager* newparent): window(nwindow), parent(newparent) {
	
	
}

void RenderingThread::operator()() {
	glfwMakeContextCurrent(window);
	while (parent->render_running) {
		if (parent->rendering) {
			world->render();
			parent->render_num_verts = world->glvecs.num_verts;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	cout << "rendering thread exited" << endl;
}

TickThread::TickThread(GLFWwindow* nwindow, ThreadManager* newparent): window(nwindow), parent(newparent) {
	
}

void TickThread::operator()() {
	//glfwMakeContextCurrent(window);
	while (parent->render_running) {
		double start_time = glfwGetTime();
		if (world != nullptr) {
			world->tick();
		}
		ms = (glfwGetTime() - start_time) * 1000;
		parent->tick_ms = ms;
		if (ms < 50) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50 - ms));
		}
	}
	cout << "tick thread exited" << endl;
}



/*
atomic<bool> thread_render_flag;

GLFWwindow* background_window;

atomic<bool> vecs_locked;

atomic<bool> thread_render_flag;



void other_thread() {
	glfwMakeContextCurrent(background_window);
	
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds((int)(5)));
		if (thread_render_flag.load() and !vecs_locked) {
			cout << "drawing" << endl;
			
			thread_render_flag.store(false);
		}
	}
}

std::thread* t;


GLuint fb[2] = {std::numeric_limits<GLuint>::max(), std::numeric_limits<GLuint>::max()}; //framebuffers
GLuint rb[2] = {std::numeric_limits<GLuint>::max(), std::numeric_limits<GLuint>::max()}; //renderbuffers, color and depth




/********************************************** TEST ********************************
static void thread_____() {
	glfwMakeContextCurrent(window_slave);
	//create new shared framebuffer object
	mutexGL.lock();
	createFrameBuffer();
	isFBOready = true;
	mutexGL.unlock();
	
	for(;;){
		mutexGL.lock();
		if(!threadShouldRun){
			mutexGL.unlock();
			break;
		}
		if(isFBOdirty){
			glBindFramebuffer(GL_FRAMEBUFFER, fb[1]);
			float r = (float)rand() / (float)RAND_MAX;
			glClearColor(r,r,r,1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glFlush();
			isFBOdirty = false;
		}
		mutexGL.unlock();
	}
	printf("Exiting thread..\n");
	return;
}

void other_thread() {
	
	//glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(display_window);
	
	glGenFramebuffers(1, &fb[0]);
	glViewport(0, 0, screen_x, screen_y;
	
  while (threadShouldRun) {
		glfwPollEvents(); //get key input
		if(!isFBOsetupOnce) {
			createFrameBufferMain(); //isFBOsetupOnce = true when FBO can be used
		} else {
			if(checkFrameBuffer(fb[0])){
				if(!mutexGL.try_lock_for(std::chrono::seconds(1))) {
					continue;
				}
				if(!isFBOdirty){
					copyFrameBuffer(fb[0]);
					glfwSwapBuffers(window);
					isFBOdirty = true;
					//printf("Framebuffer OK\n");
				} else {
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				}
				mutexGL.unlock();
			} else {
				printf("Framebuffer not ready!\n");
				GLenum e = glGetError();
				printf("OpenGL error: %X\n", e);
			}
		}
  }
  threadShouldRun = false; //other thread only reads this
  gl_thread.join();
  glfwDestroyWindow(window);
  glfwDestroyWindow(window_slave);
  glfwTerminate();
  return 0;
}
*/

#endif
