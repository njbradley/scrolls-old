#ifndef SERVERGAME
#define SERVERGAME

#include "servergame.h"
#include "multiplayer/multiplayer.h"
#include "scrolls/world.h"
#include "world.h"
#include "scrolls/cross-platform.h"

ServerGame::IOthread::IOthread(ServerGame* newgame): game(newgame) {
	
}

void ServerGame::IOthread::operator()() {
	while (game->playing and !std::cin.eof()) {
		string command;
		std::cin >> command;
		game->command(command);
	}
	if (std::cin.eof()) {
		game->command("stop");
	}
}
	

ServerGame::ServerGame(): socketmanager(40296), iothread(IOthread(this)) {
	
}

ServerGame::~ServerGame() {
	iothread.join();
}

void ServerGame::command(string command) {
	if (command == "list") {
		for (pair<int,ClientEndpoint> kvpair : socketmanager.clients) {
			cout << kvpair.first << ' ' << kvpair.second.username << ' ' << kvpair.second.endpoint << endl;
		}
	} else if (command == "stop") {
		playing = false;
	}
}

void ServerGame::load_world() {
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
	
	world->startup();
	world.as<ServerWorld>()->socketmanager = &socketmanager;
}

void ServerGame::setup_gameloop() {
	
}

void ServerGame::gametick() {
	socketmanager.tick();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

EXPORT_PLUGIN(ServerGame);


#endif
