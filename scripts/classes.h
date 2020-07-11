#ifndef CLASSES
#define CLASSES

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>
using glm::vec3;
using glm::ivec3;
using glm::vec2;
using glm::ivec2;
using std::cout;
using std::endl;
using std::string;
using std::function;

void crash(long long);
void hard_crash(long long);

void print(vec3 v);

struct ivec3_comparator {
  bool operator() (const ivec3& lhs, const ivec3& rhs) const
  {
      return lhs.x < rhs.x || lhs.x == rhs.x && (lhs.y < rhs.y || lhs.y == rhs.y && lhs.z < rhs.z);
  }
};

struct vec3_comparator {
  bool operator() (const vec3& lhs, const vec3& rhs) const
  {
      return lhs.x < rhs.x || lhs.x == rhs.x && (lhs.y < rhs.y || lhs.y == rhs.y && lhs.z < rhs.z);
  }
};


struct ivec3_hash {
	size_t operator() (const ivec3& pos) const {
		string all = std::to_string(pos.x) + std::to_string(pos.y) + std::to_string(pos.z);
		return std::hash<string>{}(all);
	}
};

////////////////// CLASSES ///////////////////////////
class Block;
class BlockContainer;
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
class BlockGroup;
class AxleInterface;
class PipeInterface;
class AxleGroup;
class CharArray;
class Item;
class ItemData;
class ItemStorage;
class ItemContainer;
class RenderVecs;
class MemVecs;
class GLVecs;
class BlockExtra;
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
DisplayEntity* debugentity;
Pixel* debugblock;
#endif
