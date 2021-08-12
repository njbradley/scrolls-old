#include "game.h"
#include "base/graphics.h"
#include "base/world.h"
#include "base/player.h"
#include "base/cross-platform.h"

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
	
	world->player->computeMatricesFromInputs();
	world->timestep();
	
	graphics->block_draw_call();
	
	world->player->render_ui(graphics->uivecs());
	graphics->ui_draw_call();
	
	graphics->swap();
}

EXPORT_PLUGIN(SingleGame);
