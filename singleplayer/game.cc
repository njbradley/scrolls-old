#include "game.h"
#include "scrolls/graphics.h"
#include "scrolls/world.h"
#include "scrolls/player.h"
#include "scrolls/cross-platform.h"
#include "scrolls/debug.h"

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
	} else {
		cout << "completed without errors" << endl;
	}
}


void SingleGame::setup_gameloop() {
	
}

void SingleGame::gametick() {
	
	world->load_nearby_chunks();
	
	world->timestep();
	
	graphics->block_draw_call();
	
	world->player->render_ui(graphics->uivecs());
	logger->render(graphics->uivecs());
	graphics->ui_draw_call();
	
	graphics->swap();
	
	if (controls->key_pressed('Q') and controls->key_pressed(controls->KEY_CTRL)) {
		playing = false;
	} else if (controls->key_pressed('X')) {
		exit(1);
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
