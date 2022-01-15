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

typedef unsigned int uint;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

#define SAFEMOD(a,b) ( ((b) + ((a)%(b))) % (b) )
#define SAFEDIV(a,b) ( (a) / (b) + (((a) % (b)) >> 31) )
#define SAFEFLOOR(a) ( int(a) - ((a) - int(a) < 0) )
#define SAFEFLOOR3(a) ( ivec3(SAFEFLOOR((a).x), SAFEFLOOR((a).y), SAFEFLOOR((a).z)) )

#define ERR(msg) throw std::runtime_error(string(msg) + "\n" + __PRETTY_FUNCTION__ + ": " + __FILE__ + " Line " + std::to_string(__LINE__));
#define dkout if (controls->key_pressed('K')) cout

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

struct ivec3_loop {
  ivec3 min;
  ivec3 max;
  ivec3 step;
  
  ivec3_loop(ivec3 min, ivec3 max, ivec3 step = ivec3(1,1,1));
  ivec3_loop(ivec3 max);
  
  struct iterator {
    ivec3 val;
    ivec3_loop* parent;
    
    iterator operator++();
    bool operator!=(const iterator& other) const;
  };
  
  iterator begin();
  iterator end();
};

double getTime();

ostream& operator<<(ostream& out, ivec3 pos);
ostream& operator<<(ostream& out, vec3 pos);
ostream& operator<<(ostream& out, quat rot);

bool stat(string path);

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
template <typename BaseType>
class PluginDef;
class PathLib;
class UIRect;
class UIObj;
class UIVecs;
class Block;
class FreeBlock;
template <typename Iterator> class BlockIterable;
template <typename BlockT> class BlockIterator;
template <typename BlockT> class PixelIterator;
template <typename BlockT> class DirIterator;
template <typename BlockT> class DirPixelIterator;
template <typename BlockT> class BlockSideIterable;
class Collider;
class PhysicsEngine;
class Pixel;
class World;
class Material;
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
class CharArray;
class Item;
class ItemData;
class ItemStorage;
class ItemContainer;
class RenderVecs;
class Tile;
class TileLoader;
class TickRunner;
class TerrainLoader;
class GraphicsContext;
class AudioContext;
class TerrainGenerator;
class TerrainObject;
class Controls;

//////////////////////// GLOBAL VARIABLES ////////////////////////



#endif
