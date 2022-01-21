#ifndef CLIENTGAME
#define CLIENTGAME

#include "clientgame.h"
#include "multiplayer/multiplayer.h"
#include "scrolls/world.h"
#include "scrolls/settings.h"

string randusername() {
	stringstream sstr;
	sstr << "name" << time(NULL) % 1000;
	return sstr.str();
}

ClientGame::ClientGame() {
	
}

ClientGame::~ClientGame() {
	
}

void ClientGame::load_world() {
	socketmanager.connect(settings->ip_addr, 40296, randusername());
	world.init("skdjflskdf");
	world->startup();
}

void ClientGame::setup_gameloop() {
	SingleGame::setup_gameloop();
}

void ClientGame::gametick() {
	SingleGame::gametick();
	socketmanager.tick();
	// std::this_thread::sleep_for(std::chrono::milliseconds(50));
	
	if (controls->key_pressed('Q')) {
		playing = false;
	}
}

EXPORT_PLUGIN(ClientGame);

#endif
