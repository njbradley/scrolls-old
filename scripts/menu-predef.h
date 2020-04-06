#ifndef MENU_PREDEF
#define MENU_PREDEF

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <chrono>
#include <thread>
using std::stringstream;
using std::vector;
using std::string;
#include <fstream>
using std::ifstream;

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
#include "classes.h"


// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "crafting-predef.h"

class Menu { public:
    virtual void render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs) = 0;
    virtual void close(World*) = 0;
    void start();
    void end(World*);
};

class InventoryMenu: public Menu { public:
    string header;
    ItemStack in_hand;
    function<void()> after;
    ItemContainer* other;
    int button;
    double xpos, ypos;
    
    InventoryMenu(string head, ItemContainer* start_other, function<void()> after_func);
    void render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs);
    void close(World*);
};

class CraftingMenu: public Menu { public:
  ItemStack in_hand;
  function<void()> after;
  int button;
  double xpos, ypos;
  vector<Recipe*> recipes;
  int num_possible;
  int page;
  static const int len_page = 8;
  ItemContainer outputs[len_page];
  ItemContainer inputs[len_page];
  
  CraftingMenu(function<void()> after_func);
  void get_recipes();
  void render_page();
  void render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs);
  void close(World*);
};

class SelectMenu : public Menu { public:
    string header;
    vector<string> options;
    string chosen;
    function<void(string)> after;
    bool click;
    
    SelectMenu(string head, vector<string> & opts, function<void(string)> after_func);
    void render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs);
    void close(World*);
};

class TextInputMenu: public Menu { public:
  string header;
  string text;
  function<void(string)> after;
  
  TextInputMenu(string head, bool is_filter, function<void(string)> after_func);
  void render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs);
  void close(World*);
};

#endif
