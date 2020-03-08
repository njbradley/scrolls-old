#ifndef MENU
#define MENU

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

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "cross-platform.h"
#include "ui.h"

#include "menu-predef.h"

int last_num_ui_verts_menu;

vector<string> worlds;


void Menu::start() {
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_FALSE);
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Menu::end() {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPos(window, screen_x/2, screen_y/2);
}


//////////////////////////////////// InventoryMenu ////////////////////////////////////////

InventoryMenu::InventoryMenu(string head, ItemContainer* start_other, function<void()> after_func): header(head), after(after_func), other(start_other), in_hand(nullptr, 0) {
  start();
}

void InventoryMenu::render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs) {
  /////rendering
  
  glfwGetCursorPos(window, &xpos, &ypos);
  xpos = xpos/screen_x*2-1;
  ypos = 1-ypos/screen_y*2;
  
  player->inven.render(uivecs, -0.5f, -1);
  player->backpack.render(uivecs, -0.5f, -0.5f);
  if (other != nullptr) {
    other->render(uivecs, -0.5f, 0);
  }
  if (in_hand.item != nullptr) {
    in_hand.render(uivecs, float(xpos)-0.05f, float(ypos)-0.05f);
  }
  
  /////input precesseing
  
  int last_button = button;
  button = 0;
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    button = 1;
  } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
    button = 2;
  }
  if (button != 0 and last_button == 0) {
    ItemContainer* container = nullptr;
    int index = -1;
    pair<ItemContainer*,int> location(nullptr,-1);
    if (ypos < -1+0.1f*aspect_ratio and ypos > -1) {
      container = &player->inven;
    } else if (ypos < -0.5f+0.1f*aspect_ratio and ypos > -0.5f) {
      container = &player->backpack;
    } else if (ypos < 0.1f*aspect_ratio and ypos > 0) {
      container = other;
    }
    
    if (xpos < 0.5f and xpos > -0.5f) {
      index = int(xpos*10+5);
    }
    if (container != nullptr and index != -1) {
      if (button == 1) {
        if (in_hand.item != container->items[index].item) {
          ItemStack tmp = in_hand;
          in_hand = container->items[index];
          container->items[index] = tmp;
        } else {
          container->items[index].count += in_hand.count;
          in_hand = ItemStack(nullptr, 0);
        }
      } else if (button == 2) {
        if (in_hand.item == nullptr) {
          in_hand = container->items[index];
          int total = in_hand.count;
          in_hand.count = total/2;
          container->items[index].count = total/2 + total%2;
        }
      }
    }
  }
  
  if (glfwGetKey(window, GLFW_KEY_R ) == GLFW_PRESS) {
    end();
    after();
  }
}

/////////////////////////////////////////craftingh ///////////////////////////////////////

CraftingMenu::CraftingMenu(function<void()> after_func): after(after_func), input(8), output(4), in_hand(nullptr, 0) {
  start();
}

void CraftingMenu::render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs) {
  /////rendering
  
  glfwGetCursorPos(window, &xpos, &ypos);
  xpos = xpos/screen_x*2-1;
  ypos = 1-ypos/screen_y*2;
  
  player->inven.render(uivecs, -0.5f, -1);
  player->backpack.render(uivecs, -0.5f, -0.5f);
  input.render(uivecs, -0.5f, 0.5f);
  output.render(uivecs, -0.5f, 0.0f);
  
  if (in_hand.item != nullptr) {
    in_hand.render(uivecs, float(xpos)-0.05f, float(ypos)-0.05f);
  }
  
  /////input precesseing
  
  int last_button = button;
  button = 0;
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    button = 1;
  } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
    button = 2;
  }
  if (button != 0 and last_button == 0) {
    ItemContainer* container = nullptr;
    int index = -1;
    pair<ItemContainer*,int> location(nullptr,-1);
    if (ypos < -1+0.1f*aspect_ratio and ypos > -1) {
      container = &player->inven;
    } else if (ypos < -0.5f+0.1f*aspect_ratio and ypos > -0.5f) {
      container = &player->backpack;
    } else if (ypos < 0.1f*aspect_ratio and ypos > 0) {
      container = &output;
    } else if (ypos < 0.5f+0.1f*aspect_ratio and ypos > 0.5f) {
      container = &input;
    }
    
    if (xpos < 0.5f and xpos > -0.5f) {
      index = int(xpos*10+5);
    }
    if (container != nullptr and index != -1) {
      bool changed = false;
      if (button == 1) {
        
        if (container == &output) {
          if (container->items[index].item != nullptr and (in_hand.item == nullptr or in_hand.item == container->items[index].item)) {
            if (displayed_recipe != nullptr) {
              output.clear();
              displayed_recipe->craft(&input, &output);
              displayed_recipe = nullptr;
            }
            if (in_hand.item == nullptr) {
              in_hand = container->items[index];
            } else {
              in_hand.count += container->items[index].count;
            }
            container->items[index] = ItemStack(nullptr, 0);
            changed = true;
          }
        } else {
          
          if (in_hand.item != container->items[index].item) { // swap in hand and in container
            ItemStack tmp = in_hand;
            in_hand = container->items[index];
            container->items[index] = tmp;
            changed = true;
          } else { // merge hand into container
            container->items[index].count += in_hand.count;
            in_hand = ItemStack(nullptr, 0);
            changed = true;
          }
        }
        
      } else if (button == 2) {
        if (in_hand.item == nullptr) { // get half of items
          in_hand = container->items[index];
          int total = in_hand.count;
          in_hand.count = total/2;
          container->items[index].count = total/2 + total%2;
          changed = true;
        }
      }
      
      if ( (container == &input or container == &output) and changed) {
        bool clear = true;
        for (ItemStack is : output.items) {
          if (is.item != nullptr) {
            clear = false;
            break;
          }
        }
        if (clear or displayed_recipe != nullptr) {
          displayed_recipe = nullptr;
          int best_num_items = 0;
          output.clear();
          for (Recipe* recipe : recipestorage->recipes) {
            if (recipe->display(&input, nullptr)) {
              int num_items = 0;
              for (ItemStack is : recipe->input.items) {
                if (is.item != nullptr) {
                  num_items ++;
                }
              }
              if (best_num_items < num_items) {
                displayed_recipe = recipe;
                best_num_items = num_items;
              }
            }
          }
          if (displayed_recipe != nullptr) {
            displayed_recipe->display(&input, &output);
          }
        }
      }
    }
  }
  
  if (glfwGetKey(window, GLFW_KEY_R ) == GLFW_PRESS) {
    for (ItemStack is : input.items) {
      player->backpack.add(is.item, is.count);
    }
    end();
    after();
  }
}



////////////////////////////////// select /////////////////////////////////////////////

SelectMenu::SelectMenu(string head, vector<string> & opts, function<void(string)> after_func): header(head), after(after_func) {
    options.swap(opts);
    chosen = "";
    start();
}

void SelectMenu::render(GLFWwindow* window, World* world, Player* player, RenderVecs* uivecs) {
    int i = 0;
    draw_text(uivecs, header, -0.5, 0.75f, 3);
    for (string name : options) {
        draw_text(uivecs, name, -0.4f, 0.5f - i*0.1f, 2);
        i++;
    }
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
	      double xpos, ypos;
	      glfwGetCursorPos(window, &xpos, &ypos);
        int index = (int)((double)ypos/screen_y*20) - 4;
        if (index >= 0 and index < options.size()) {
            chosen = options[index];
        }
    }
    
    if (chosen != "") {
        end();
        after(chosen);
    }
}









#endif
