#include <thread>
#include "classes.h"
#include "ui.h"
#include "rendervec-predef.h"
#include "world-predef.h"
#include <atomic>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <cstdio>
#include <limits>
#include <chrono>
#include <thread>
#include <mutex>

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




/********************************************** TEST *********************************/
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
