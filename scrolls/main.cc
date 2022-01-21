#include "audio.h"
#include "debug.h"
#include "graphics.h"
#include "settings.h"
#include "game.h"
#include "cross-platform.h"
#include "debug.h"
#include "rendervecs.h"
#include "materials.h"
#include "blockdata.h"
#include "items.h"
#include "player.h"
#include "tests.h"
#include "world.h"

#include <filesystem>

int test_main(int numargs, char** args) {
	Storage<Test> tests;
	tests.init();
	
	cout << "Loaded " << tests.size() << " tests "<< endl;
	if (numargs > 2) {
		for (int i = 2; i < numargs; i ++) {
			tests[args[i]]->init();
		}
		for (int i = 2; i < numargs; i ++) {
			tests[args[i]]->run_tests();
		}
		for (int i = 2; i < numargs; i ++) {
			tests[args[i]]->print_summary(cout, std::cerr);
		}
	} else {
		for (int i = 0; i < tests.size(); i ++) {
			tests[i]->init();
		}
		for (int i = 0; i < tests.size(); i ++) {
			tests[i]->run_tests();
		}
		std::stringstream mainout;
		std::stringstream errout;
		for (int i = 0; i < tests.size(); i ++) {
			tests[i]->print_summary(mainout, errout);
		}
		cout << "------------------------------------" << endl;
		cout << mainout.str();
		cout << "------------------------------------" << endl;
		std::cerr << errout.str();
	}
	cout << "Done" << endl;
	return 0;
}

int game_main() {
	if (stat("AUTO_DELETE_SAVES")) {
		cout << "revmoing "<< endl;
		std::filesystem::remove_all("saves");
	}
	
	cout << "Starting " << endl;
	logger.init();
	debugger.init();
	settings.init();
	audio.init();
	graphics.init();
	viewbox.init();
	controls.init();
	debuglines.init();
	
	materialstorage.init();
	blockstorage.init();
	itemstorage.init();
	
	game.init();
	
	logger->sel_page = 3;
	logger->log(1) << "Starting game! " << endl;
	
	game->load_world();
	
	game->setup_gameloop();
	while (game->playing) {
		game->gametick();
	}
	
	logger->log(1) << " ending game" << endl;
	game.close();
	
	itemstorage.close();
	blockstorage.close();
	materialstorage.close();
	controls.close();
	viewbox.close();
	graphics.close();
	audio.close();
	settings.close();
	debugger.close();
	logger.close();
	
	
	cout << "Completed without errors! " << endl;
	return 0;
}

int main(int numargs, char** args) {
	if (numargs > 1 and string(args[1]) != "test") {
		pluginloader.load_reqs(args[1]);
		numargs --;
		args ++;
	}
	pluginloader.load_reqs("plugins.txt");
	pluginloader.load();
	
	if (numargs > 1 and string(args[1]) == "test") {
		return test_main(numargs, args);
	} else {
		return game_main();
	}
}


/*
All other scripts in this repository are considered part of the
game and are under this same license
void
Copyright 2020 Nicholas Bradley. All rights reserved.

I grant the right to download and modify the game for your own personal use.
Please do not distribute or sell original or modified copies of this game without express permission.

This software is provided 'as is' with no expressed or implied warranties.
*/
