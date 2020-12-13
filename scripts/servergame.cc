#ifndef SERVERGAME
#define SERVERGAME

#include "servergame.h"

ServerGame::IOthread::IOthread(ServerGame* newgame): game(newgame) {
	
}

void ServerGame::IOthread::operator()() {
	while (game->playing) {
		string command;
		std::cin >> command;
		game->command(command);
	}
}
	

ServerGame::ServerGame(int port): socketmanager(port), iothread(IOthread(this)) {
	
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

void ServerGame::gametick() {
	socketmanager.tick();
}




#endif
