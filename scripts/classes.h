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
using glm::vec4;
using glm::ivec4;
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

void wait();
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

struct ivec4_hash {
	size_t operator() (const ivec4& pos) const;
};

template <class Friend, typename T>
class readonly {
	friend Friend;
	T data;
	T operator=(const T& arg) {
		data = arg;
		return data;
	}
public:
  readonly(T newval): data(newval) {}
	operator const T&() const {
		return data;
	}
};

ostream& operator<<(ostream& out, ivec3 pos);
ostream& operator<<(ostream& out, vec3 pos);

//ofstream dfile("debug.txt");
extern int dloc;

////////////////// CLASSES ///////////////////////////
class Block;
class BlockContainer;
class BlockIter;
class ConstBlockIter;
class BlockTouchIter;
class BlockGroupIter;
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
class Settings;
class RenderIndex;
class BlockStorage;
class BlockGroup;
class UnlinkedGroup;
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
class AsyncGLVecs;
class GLVecsDestination;
class BlockExtra;
class Command;
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
class Game;
class AudioContext;
class Sound;
class PlayingSound;
class ClientSocketManager;
class ServerSocketManager;
class ClientGame;
class ServerGame;
class GroupConnection;
class ConnectorData;
class ConnectorStorage;
template <typename T> class CommandConst;
template <typename T> class CommandVar;
template <typename T, typename ... Tparams> class CommandMethod;

//////////////////////// GLOBAL VARIABLES ////////////////////////
extern GLFWwindow* window;
extern Game* game;
extern Menu* menu;

#endif
