#ifndef CLASSES_H
#define CLASSES_H


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
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
using glm::mat4;
using std::cout;
using std::endl;
using std::string;
using std::function;
using std::unordered_set;
using std::stringstream;
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;
using std::vector;
using std::pair;

void crash(long long);
void hard_crash(long long);
void set_display_env(vec3,int);

void print(vec3 v);

struct ivec3_comparator {
  bool operator() (const ivec3& lhs, const ivec3& rhs) const;
};

struct vec3_comparator {
  bool operator() (const vec3& lhs, const vec3& rhs) const;
};


struct ivec3_hash {
	size_t operator() (const ivec3& pos) const;
};

//ofstream dfile("debug.txt");
extern int dloc;

////////////////// CLASSES ///////////////////////////
class Block;
class BlockContainer;
class Collider;
class Chunk;
class Pixel;
class World;
class Material;
class Recipe;
class Item;
class BlockData;
class Entity;
class DisplayEntity;
class FallingBlockEntity;
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
class Program;
class GraphicsContext;
template <typename T> class CommandConst;
template <typename T> class CommandVar;
template <typename T, typename ... Tparams> class CommandMethod;

//////////////////////// GLOBAL VARIABLES ////////////////////////
extern bool render_flag;
extern std::stringstream debugstream;
extern GLFWwindow* window;
extern World* world;
extern Menu* menu;
extern DisplayEntity* debugentity;
extern Pixel* debugblock;
extern int num_pixels;

#endif
