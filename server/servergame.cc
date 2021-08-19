#ifndef SERVERGAME
#define SERVERGAME

#include "servergame.h"
#include "entity.h"
#include "blockdata.h"
#include "materials.h"
#include "items.h"
#include "crafting.h"
#include "glue.h"

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
	settings = new Settings();
	graphics = new GraphicsNullContext();
	audio = new AudioNullContext();
	
	matstorage = new MaterialStorage();
	connstorage = new ConnectorStorage();
	blocks = new BlockStorage();
	itemstorage = new ItemStorage();
	recipestorage = new RecipeStorage();
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

void ServerGame::setup_gameloop() {
	
}

void ServerGame::gametick() {
	socketmanager.tick();
}




#endif
