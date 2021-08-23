#include "game.h"
#include "scrolls/graphics.h"
#include "scrolls/world.h"
#include "scrolls/player.h"
#include "scrolls/cross-platform.h"
#include "scrolls/debug.h"

SingleGame::SingleGame() {
	
	ifstream ifile(SAVES_PATH "latest.txt");
	if (!ifile.good()) {
		cout << "making saves dir " << endl;
		create_dir(SAVES_PATH);
		cout << "made dir " << endl;
		ofstream ofile(SAVES_PATH "saves.txt");
		ofile << "";
		world.init(string("Starting-World"));
	} else {
		cout << " getting latest" << endl;
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
	
	static double lastTime = getTime();
	double curtime = getTime();\
	double deltatime = curtime - lastTime;
	lastTime = curtime;
	
	world->load_nearby_chunks();
	
	world->timestep();
	viewbox->timestep(deltatime, world->daytime);
	
	graphics->block_draw_call();
	
	world->player->render_ui(graphics->uivecs());
	logger->render(graphics->uivecs());
	graphics->ui_draw_call();
	
	graphics->swap();
	
	if (controls->key_pressed('Q') and controls->key_pressed(controls->KEY_CTRL)) {
		playing = false;
	} else if (controls->key_pressed('X') and controls->key_pressed(controls->KEY_CTRL)) {
		std::terminate();
	} else if (controls->key_pressed('X')) {
		exit(1);
	} else if (controls->key_pressed(controls->KEY_CTRL) and controls->key_pressed('S')) {
		logger->pause();
	} else if (controls->key_pressed(controls->KEY_CTRL) and controls->key_pressed('A')) {
		logger->unpause();
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
