#ifndef CLIENTGAME
#define CLIENTGAME

#include "clientgame.h"
#include "entity.h"
#include "blockdata.h"
#include "materials.h"
#include "items.h"
#include "crafting.h"
#include "glue.h"

ClientGame::ClientGame(string ip, int port, string username): socketmanager(ip, port, username) {
	
	settings = new Settings();
	graphics = new GraphicsMainContext(settings);
	audio = new AudioMainContext();
	
	matstorage = new MaterialStorage();
	connstorage = new ConnectorStorage();
	blocks = new BlockStorage();
	itemstorage = new ItemStorage();
	recipestorage = new RecipeStorage();
	
	graphics->view_distance = view_dist * World::chunksize * 10;
	
	if (settings->framerate_sync) {
		glfwSwapInterval(1);
	} else {
		glfwSwapInterval(0);
	}
}

ClientGame::~ClientGame() {
	
	delete graphics;
	delete audio;
	delete settings;
	
	delete itemstorage;
	delete blocks;
	delete connstorage;
	delete recipestorage;
	delete matstorage;
	
}

void ClientGame::setup_gameloop() {
	
}

void ClientGame::gametick() {
	socketmanager.tick();
	
	if (glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS and glfwGetKey(window, 	GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS) {
		playing = false;
	} else if (glfwWindowShouldClose(window)) {
		playing = false;
	} else if (glfwGetKey(window, GLFW_KEY_X ) == GLFW_PRESS) {
		cout << "Hard termination, no logging" << endl;
		exit(2);
	}
}



#endif
