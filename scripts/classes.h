#ifndef CLASSES
#define CLASSES

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::ivec3;
using glm::vec2;
using glm::ivec2;
using std::cout;
using std::endl;
using std::string;

void crash(long long);
void hard_crash(long long);

void print(vec3 v);

////////////////// CLASSES ///////////////////////////
class Block;
class Collider;
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
class TerrainLoader;
class TerrainObject;
class TerrainObjectMerger;

//////////////////////// GLOBAL VARIABLES ////////////////////////
bool render_flag;
std::stringstream debugstream;
GLFWwindow* window;
World* world;
Menu* menu;
#endif
