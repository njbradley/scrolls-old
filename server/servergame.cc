#ifndef SERVERGAME
#define SERVERGAME

#include "servergame.h"
#include "multiplayer/multiplayer.h"
#include "scrolls/world.h"

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
	
}

void ServerGame::setup_gameloop() {
	
}

void ServerGame::gametick() {
	socketmanager.tick();
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

EXPORT_PLUGIN(ServerGame);


#endif
