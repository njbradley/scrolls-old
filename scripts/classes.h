#ifndef CLASSES
#define CLASSES

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
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
class Player;
class BlockStorage;
class CharArray;
class ItemStorage;
class ItemContainer;
class RenderVecs;
class GLVecs;
class BlockExtras;
class Menu;
class InventoryMenu;
class SelectMenu;

//////////////////////// GLOBAL VARIABLES ////////////////////////
bool render_flag;
std::stringstream debugstream;
GLFWwindow* window;
World* world;
Menu* menu;
#endif
