#ifndef CLASSES
#define CLASSES

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::ivec3;
using std::cout;
using std::endl;
using std::string;

////////////////// CLASSES ///////////////////////////
class Block;
class Chunk;
class Pixel;
class World;
class Item;
class BlockData;
class Entity;
class DisplayEntity;
class Player;
class BlockStorage;
class CharArray;
class ItemStorage;
class ItemContainer;
class RenderVecs;
class MemVecs;
class GLVecs;
class BlockExtras;
class Menu;
class InventoryMenu;
class SelectMenu;
class CraftingMenu;
class ItemStack;
class TerrainObject;
class ChunkLoader;
class ThreadManager;
class Tile;

//////////////////////// GLOBAL VARIABLES ////////////////////////
bool render_flag;
std::stringstream debugstream;
GLFWwindow* window;
World* world;
Menu* menu;
#endif
