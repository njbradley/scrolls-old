#ifndef CLASSES_H
#define CLASSES_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <functional>
#include <unordered_map>
#include <unordered_set>
using std::unordered_map;
using glm::vec3;
using glm::ivec3;
using glm::vec2;
using glm::ivec2;
using glm::vec4;
using glm::ivec4;
using glm::mat4;
using glm::quat;
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

typedef uint8_t uint8;
typedef unsigned int uint;

#define SAFEMOD(a,b) ( ((b) + ((a)%(b))) % (b) )
#define SAFEDIV(a,b) ( (a) / (b) + (((a) % (b)) >> 31) )
#define SAFEFLOOR(a) ( int(a) - ((a) - int(a) < 0) )
#define SAFEFLOOR3(a) ( ivec3(SAFEFLOOR((a).x), SAFEFLOOR((a).y), SAFEFLOOR((a).z)) )

#define ERR(msg) throw std::runtime_error(string(msg) + "\n" + __PRETTY_FUNCTION__ + ": " + __FILE__ + " Line " + std::to_string(__LINE__));
#define dkout if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) cout

struct ivec3_comparator {
  bool operator() (const ivec3& lhs, const ivec3& rhs) const;
};

struct vec3_comparator {
  bool operator() (const vec3& lhs, const vec3& rhs) const;
};


struct ivec3_hash {
	size_t operator() (const ivec3& pos) const;
};

struct ivec4_hash {
	size_t operator() (const ivec4& pos) const;
};

double getTime();

ostream& operator<<(ostream& out, ivec3 pos);
ostream& operator<<(ostream& out, vec3 pos);
ostream& operator<<(ostream& out, quat rot);

// #define RESOURCES_PATH "resources/"
#define SAVES_PATH "saves/"

#define csize 2
#define csize3 8

typedef int Blocktype;

extern const ivec3 dir_array[6];
int dir_to_index(ivec3 dir);

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

////////////////// CLASSES ///////////////////////////
class Game;
class PluginLib;
class PluginLoader;
class PathLib;
class UIRect;
class UIObj;
class UIVecs;
class Block;
class FreeBlock;
class BlockIter;
class ConstBlockIter;
class BlockTouchIter;
class BlockTouchSideIter;
class BlockGroupIter;
class Collider;
class Pixel;
class World;
class Material;
class Recipe;
class BlockData;
class Movingpoint;
class Hitbox;
class Movingbox;
class Entity;
class Player;
class Settings;
class RenderIndex;
class BlockStorage;
class BlockGroup;
class UnlinkedGroup;
class CharArray;
class Item;
class ItemData;
class ItemStorage;
class ItemContainer;
class RenderVecs;
class ThreadManager;
class Tile;
class TileLoader;
class TerrainLoader;
class GraphicsContext;
class AudioContext;

//////////////////////// GLOBAL VARIABLES ////////////////////////



#endif
