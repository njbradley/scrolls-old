// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <chrono>
#include <thread>
#include <iomanip>
//#include <boost/asio.hpp>

using std::stringstream;

//#include <nvwa/debug_new.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.h"
#include "texture.h"
#include "entity.h"
#include "menu.h"
#include "world.h"
#include "blockdata.h"
#include "tiles.h"
#include "blocks.h"
#include "items.h"
#include "crafting.h"
#include "terrain.h"
#include "blockgroups.h"
#include "multithreading.h"
#include "mobs.h"
#include "commands.h"
#include "materials.h"
#include "ui.h"
#include "text.h"
#include "graphics.h"
#include "game.h"
#include "audio.h"


#include "cross-platform.h"
#include "dwarfstack.h"


#include <future>


int main()
{
	
	setup_backtrace();
	
	matstorage = new MaterialStorage();
	blocks = new BlockStorage();
	connstorage = new ConnectorStorage();
	itemstorage = new ItemStorage();
	recipestorage = new RecipeStorage();
	entitystorage = new EntityStorage();
	mobstorage = new MobStorage();
	
	
	
	delete game;
}
