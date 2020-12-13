#ifndef CLIENTGAME
#define CLIENTGAME

#include "clientgame.h"
#include "entity.h"
#include "mobs.h"
#include "blockdata.h"
#include "materials.h"
#include "items.h"
#include "crafting.h"

ClientGame::ClientGame(string ip, int port, string username): socketmanager(ip, port, username), graphics(&settings) {
	
	matstorage = new MaterialStorage();
	blocks = new BlockStorage();
	itemstorage = new ItemStorage();
	recipestorage = new RecipeStorage();
	entitystorage = new EntityStorage();
	mobstorage = new MobStorage();
	
	graphics.view_distance = view_dist * World::chunksize * 10;
	
	if (settings.framerate_sync) {
		glfwSwapInterval(1);
	} else {
		glfwSwapInterval(0);
	}
}

ClientGame::~ClientGame() {
	
	delete itemstorage;
	delete blocks;
	delete recipestorage;
	delete entitystorage;
	delete mobstorage;
	delete matstorage;
	
}

void ClientGame::setup_gameloop() {
	
}

void ClientGame::gametick() {
	socketmanager.tick();
}



#endif
