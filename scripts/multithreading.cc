#ifndef MULITTHREADING
#define MULITTHREADING

#include "multithreading.h"

#include "classes.h"
#include "ui.h"
#include "rendervec.h"
#include "world.h"
#include "tiles.h"
#include "game.h"
#include "blocks.h"

#include <atomic>

ThreadJob::ThreadJob(ivec3 npos): pos(npos), result(nullptr), complete(false) {
	
}

template <typename T>
void JobQueue<T>::push_back(T newitem) {
	queue.push_back(newitem);
}

template <typename T>
T JobQueue<T>::pop_front(bool* sucess) {
	*sucess = false;
	getlock.lock();
	T item;
	if (!empty()) {
		item = queue.front();
		*sucess = true;
		queue.pop_front();
	}
	getlock.unlock();
	return item;
}

template <typename T>
bool JobQueue<T>::empty() {
	return queue.empty();
}


ThreadManager::ThreadManager() {
	rendering = false;
	render_running = true;
	// glfwWindowHint(GLFW_SAMPLES, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	// GLFWwindow* renderwindow = glfwCreateWindow( screen_x, screen_y, "back", nullptr, window);
	
	rendering_thread = thread(RenderingThread(nullptr, this));
	
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
	load_queue.push_back(pos);
	return true;
	// for (int i = 0; i < num_threads; i ++) {
	// 	if (loading[i] == nullptr) {
	// 		loading[i] = new ThreadJob(pos);
	// 		return true;
	// 	}
	// }
	// return false;
}

bool ThreadManager::add_deleting_job(ivec3 pos) {
	del_queue.push_back(pos);
	return true;
	// for (int i = 0; i < num_threads; i ++) {
	// 	if (deleting[i] == nullptr) {
	// 		deleting[i] = new ivec3(pos);
	// 		return true;
	// 	}
	// }
	// return false;
}

vector<Tile*> ThreadManager::get_loaded_tiles() {
	vector<Tile*> tiles;
	for ( int i = 0; i < num_threads; i ++) {
		if (loading[i] != nullptr and loading[i]->complete and loading[i]->lock.try_lock_for(std::chrono::seconds(1))) {
			//tiles.push_back(loading[i]->result);
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
		if (game != nullptr and game->world != nullptr and !parent->load_queue.empty()) {
			bool sucess;
			ivec3 pos = parent->load_queue.pop_front(&sucess);
			if (sucess) {
				double start = glfwGetTime();
				Tile* tile = new Tile(pos, game->world);
				double mid = glfwGetTime();
				game->world->add_tile(tile);
				tile->lighting_update();
				int top_val = tile->chunk->get_global(32, 63, 32, 1)->get_pix()->sunlight;
				if (tile->fully_loaded) {
					//cout << pos << endl;
				}
				if (mid - start > 0.0001) {
					//cout << glfwGetTime() - mid << "l " << mid - start << "g --- " << pos << ' ' << top_val << endl;
				}
			}
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
			
		// if (game != nullptr and game->world != nullptr and parent->loading[index] != nullptr and parent->loading[index]->complete == false) {
		// 		parent->loading[index]->lock.lock();
		// 		//cout << parent->loading[index] << endl;
		// 		Tile* tile = new Tile(parent->loading[index]->pos, game->world);
		// 		parent->loading[index]->result = tile;
		// 		game->world->add_tile(tile);
		// 		parent->loading[index]->complete = true;
		// 		parent->loading[index]->lock.unlock();
		// 		tile->lighting_update();
		// 		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		// } else {
		// 	std::this_thread::sleep_for(std::chrono::milliseconds(100));
		// }
		//cout << "hello from thread " << index << endl;
	}
	cout << "loading thread " << index << " exited" << endl;
}


DeletingThread::DeletingThread(GLFWwindow* nwindow, int newindex, ThreadManager* newparent): window(nwindow), index(newindex), parent(newparent) {
	
}

void DeletingThread::operator()() {
	//glfwMakeContextCurrent(window);
	while (parent->del_running[index]) {
		if (!parent->del_queue.empty()) {
			bool sucess;
			ivec3 pos = parent->del_queue.pop_front(&sucess);
			if (sucess) {
				game->world->del_chunk(pos, true);
			}
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	cout << "deleting thread " << index << " exited" << endl;
}



RenderingThread::RenderingThread(GLFWwindow* nwindow, ThreadManager* newparent): window(nwindow), parent(newparent) {
	start_time = glfwGetTime();
	
}

void RenderingThread::operator()() {
	//glfwMakeContextCurrent(window);
	while (parent->render_running) {
		bool wait = true;
		if (parent->rendering) {
			wait = !game->world->render();
			parent->render_num_verts = game->world->glvecs.num_verts;
		}
		if (wait) {
			if (busy and parent->rendering and game->world != nullptr and !game->world->initial_generation) {
				times ++;
				if (times > 5) {
					cout << "rendered done " << glfwGetTime() - start_time << endl;
					busy = false;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		} else if (busy) {
			times = 0;
		}
	}
	
	cout << "rendering thread exited" << endl;
}

TickThread::TickThread(GLFWwindow* nwindow, ThreadManager* newparent): window(nwindow), parent(newparent) {
	
}

void TickThread::operator()() {
	//glfwMakeContextCurrent(window);
	while (parent->render_running) {
		double start_time = glfwGetTime();
		if (game != nullptr and game->world != nullptr) {
			game->world->tick();
		}
		ms = (glfwGetTime() - start_time) * 1000;
		parent->tick_ms = ms;
		if (ms < 50) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50 - ms));
		}
	}
	cout << "tick thread exited" << endl;
}



template class JobQueue<ivec3>;

#endif
