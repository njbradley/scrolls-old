#ifndef CLASSES
#define CLASSES

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>
#include <unordered_map>
#include <unordered_set>
using std::unordered_map;
using glm::vec3;
using glm::ivec3;
using glm::vec2;
using glm::ivec2;
using std::cout;
using std::endl;
using std::string;
using std::function;
using std::unordered_set;

void crash(long long);
void hard_crash(long long);
void set_display_env(vec3,int);

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
    std::hash<int> ihash;
		return ihash(pos.x) ^ ihash(pos.y) ^ ihash(pos.z);
	}
};

//ofstream dfile("debug.txt");
int dloc = -69;

////////////////// CLASSES ///////////////////////////
class Block;
class BlockContainer;
class Collider;
class Chunk;
class Pixel;
class World;
class Material;
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
class Program;
template <typename T> class CommandConst;
template <typename T> class CommandVar;
template <typename T, typename ... Tparams> class CommandMethod;

//////////////////////// GLOBAL VARIABLES ////////////////////////
bool render_flag;
std::stringstream debugstream;
GLFWwindow* window;
World* world;
Menu* menu;
DisplayEntity* debugentity;
Pixel* debugblock;
#endif
