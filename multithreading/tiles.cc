#include "tiles.h"
#include "base/world.h"
#include "base/game.h"



ThreadJob::ThreadJob(ivec3 npos): pos(npos), result(nullptr), complete(false) {
	
}

template <typename T>
void JobQueue<T>::push_back(T newitem) {
	if (std::find(queue.begin(), queue.end(), newitem) == queue.end()) {
		queue.push_back(newitem);
	}
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

ThreadedTileLoader::ThreadedTileLoader(World* nworld): world(nworld) {
	
}

void ThreadedTileLoader::begin_serving() {
	rendering = false;
	render_running = true;
	
	rendering_thread = thread(RenderingThread(this));
	
	tick_thread = thread(TickThread(this));
	
	for (int i = 0; i < num_threads; i ++) {
		loading[i] = nullptr;
		deleting[i] = nullptr;
		load_running[i] = true;
		del_running[i] = true;
		
		loading_threads[i] = thread(LoadingThread(i, this));
		deleting_threads[i] = thread(DeletingThread(i, this));
	}
}

void ThreadedTileLoader::request_load_tile(ivec3 pos) {
	load_queue.push_back(pos);
}

void ThreadedTileLoader::request_del_tile(ivec3 pos) {
	del_queue.push_back(pos);
}

void ThreadedTileLoader::end_serving() {
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
		




LoadingThread::LoadingThread(int newindex, ThreadedTileLoader* newparent): index(newindex), parent(newparent) {
	
}

void LoadingThread::operator()() {
	while (parent->load_running[index]) {
		//cout << parent->loading[index] << endl;
		if (parent->world != nullptr and !parent->load_queue.empty()) {
			bool sucess;
			ivec3 pos = parent->load_queue.pop_front(&sucess);
			if (sucess) {
				double start = getTime();
				Tile* tile = new Tile(pos, parent->world);
				double mid = getTime();
				parent->world->add_tile(tile);
				//for (int i = 0; i < 5; i ++) {
					tile->lighting_update();
				//}
			}
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
			
		// if (game != nullptr and parent->world != nullptr and parent->loading[index] != nullptr and parent->loading[index]->complete == false) {
		// 		parent->loading[index]->lock.lock();
		// 		//cout << parent->loading[index] << endl;
		// 		Tile* tile = new Tile(parent->loading[index]->pos, parent->world);
		// 		parent->loading[index]->result = tile;
		// 		parent->world->add_tile(tile);
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


DeletingThread::DeletingThread(int newindex, ThreadedTileLoader* newparent): index(newindex), parent(newparent) {
	
}

void DeletingThread::operator()() {
	while (parent->del_running[index]) {
		if (!parent->del_queue.empty()) {
			bool sucess;
			ivec3 pos = parent->del_queue.pop_front(&sucess);
			if (sucess) {
				parent->world->del_tile(pos, true);
			}
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	cout << "deleting thread " << index << " exited" << endl;
}



RenderingThread::RenderingThread(ThreadedTileLoader* newparent): parent(newparent) {
	start_time = getTime();
	
}

void RenderingThread::operator()() {
	while (parent->render_running) {
		bool wait = true;
		wait = !parent->world->render();
		if (wait) {
			if (busy and parent->rendering and parent->world != nullptr) {
				times ++;
				if (times > 5) {
					cout << "rendered done " << getTime() - start_time << endl;
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

TickThread::TickThread(ThreadedTileLoader* newparent): parent(newparent) {
	
}

void TickThread::operator()() {
	while (parent->render_running) {
		double start_time = getTime();
		if (parent->world != nullptr) {
			parent->world->tick();
		}
		ms = (getTime() - start_time) * 1000;
		parent->tick_ms = ms;
		if (ms < 50) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50 - ms));
		}
	}
	cout << "tick thread exited" << endl;
}



template class JobQueue<ivec3>;

EXPORT_PLUGIN(ThreadedTileLoader);
