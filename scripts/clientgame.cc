#ifndef CLIENTGAME
#define CLIENTGAME

#include "clientgame.h"

ClientGame::ClientGame(string ip, int port, string username): socketmanager(ip, port, username) {
	
}

ClientGame::~ClientGame() {
	
}

void ClientGame::gametick() {
	socketmanager.tick();
}



#endif
