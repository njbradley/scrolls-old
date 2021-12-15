#include "game.h"
#include "scrolls/graphics.h"
#include "scrolls/world.h"
#include "scrolls/player.h"
#include "scrolls/cross-platform.h"
#include "scrolls/debug.h"
#include "scrolls/tiles.h"
#include "scrolls/blocks.h"


SingleGame::SingleGame() {
	
	ifstream ifile(SAVES_PATH "latest.txt");
	if (!ifile.good()) {
		create_dir(SAVES_PATH);
		ofstream ofile(SAVES_PATH "saves.txt");
		ofile << "";
		world.init(string("Starting-World"));
	} else {
		string latest;
		ifile >> latest;
		if (latest != "") {
			world.init(latest);
		} else {
			ifstream ifile2(string(SAVES_PATH "saves.txt"));
			ifile2 >> latest;
			if (latest != "") {
				world.init(latest);
			} else {
				world.init(string("Starting-World"));
			}
		}
	}
	
}

SingleGame::~SingleGame() {
	if (errors) {
		cout << "completed WITH errors BADDDDDAAAA!!" << endl;
	}
}


void SingleGame::setup_gameloop() {
	
}

void SingleGame::gametick() {
	
	static double lastTime = getTime();
	static int num_frames = 0;
	static int fps = 0;
	double curtime = getTime();
	double deltatime = curtime - lastTime;
	if (int(curtime) - int(lastTime) > 0) {
		fps = num_frames;
		num_frames = 0;
	}
	
	logger->clear(3);
	logger->log(3) << "fps: " << fps << endl;
	if (world->player != nullptr) {
		logger->log(3) << "playerpos: " << world->player->box.position << endl;
	}
	
	
	num_frames++;
	
	lastTime = curtime;
	
	world->load_nearby_chunks();
	world->timestep(curtime, deltatime);
	viewbox->timestep(deltatime, world->daytime);
	
	graphics->block_draw_call();
	
	if (world->player != nullptr) {
		world->player->render_ui(graphics->uivecs);
	}
	logger->render(graphics->uivecs);
	graphics->ui_draw_call();
	
	graphics->swap();
	
	if (controls->key_pressed('Q')) {
		if (controls->key_pressed(controls->KEY_CTRL)
		or (controls->key_pressed('U') and controls->key_pressed('I') and controls->key_pressed('T'))) {
			playing = false;
			world->saving = !controls->key_pressed(controls->KEY_SHIFT);
		}
	} else if (controls->key_pressed('X') and controls->key_pressed(controls->KEY_CTRL)) {
		// std::terminate();
		int* i = nullptr;
		cout << *i <<  endl;
	} else if (controls->key_pressed('X')) {
		exit(1);
	} else if (controls->key_pressed(controls->KEY_CTRL) and controls->key_pressed('S')) {
		logger->pause();
	} else if (controls->key_pressed(controls->KEY_CTRL) and controls->key_pressed('A')) {
		logger->unpause();
	} else if (controls->key_pressed('L')) {
		debuglines->clear();
	} else if (controls->key_pressed('T')) {
		for (Tile* tile : world->tiles) {
			for (FreeBlock* free = tile->allfreeblocks; free != nullptr;) {
				if (free->entity_cast() == nullptr or free->entity_cast()->get_plugin_id() != Player::plugindef()->id) {
					free->highparent->remove_freechild(free);
					FreeBlock* next = free->allfreeblocks;
					delete free;
					free = next;
				} else {
					free = free->allfreeblocks;
				}
			}
		}
	}
	if (controls->key_pressed(controls->KEY_CTRL)) {
		for (int i = 0; i < 10; i ++) {
			if (controls->key_pressed('0' + i)) {
				logger->sel_page = i;
			}
		}
	}
}

EXPORT_PLUGIN(SingleGame);
