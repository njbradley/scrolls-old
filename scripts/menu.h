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

void Menu::end(World* world) {
  world->player->drop_ticks();
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_FALSE);
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPos(window, screen_x/2, screen_y/2);
}


//////////////////////////////////// InventoryMenu ////////////////////////////////////////

InventoryMenu::InventoryMenu(string head, ItemContainer* start_other, function<void()> after_func): header(head), after(after_func), other(start_other), in_hand(nullptr, 0), button(1) {
  start();
}

void InventoryMenu::render(GLFWwindow* window, World* world, Player* player, MemVecs* uivecs) {
  /////rendering
  
  glfwGetCursorPos(window, &xpos, &ypos);
  xpos = xpos/screen_x*2-1;
  ypos = 1-ypos/screen_y*2;
  
  player->inven.render(uivecs, -0.5f, -1);
  player->backpack.render(uivecs, -0.5f, -0.5f);
  if (other != nullptr) {
    other->render(uivecs, -0.5f, 0);
  }
  if (!in_hand.item.isnull) {
    in_hand.render(uivecs, float(xpos)-0.05f, float(ypos)-0.05f);
    draw_text(uivecs, in_hand.item.descript(), float(xpos)+0.05f, float(ypos)+0.05f);
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
        if (in_hand.item.data != container->items[index].item.data or !in_hand.item.data->stackable or ! container->items[index].item.data->stackable) {
          ItemStack tmp = in_hand;
          in_hand = container->items[index];
          container->items[index] = tmp;
        } else {
          container->items[index].count += in_hand.count;
          in_hand = ItemStack(nullptr, 0);
        }
      } else if (button == 2) {
        if (in_hand.item.isnull) {
          in_hand = container->items[index];
          int total = in_hand.count;
          in_hand.count = total/2;
          container->items[index].count = total/2 + total%2;
        }
      }
    }
  }
}

void InventoryMenu::close(World* world) {
  end(world);
  after();
}
/////////////////////////////////////////craftingh ///////////////////////////////////////

CraftingMenu::CraftingMenu(function<void()> after_func): after(after_func), in_hand(nullptr, 0), page(0), button(1) {
  start();
  get_recipes();
  render_page();
}

void CraftingMenu::get_recipes() {
  vector<Recipe*> possible;
  vector<Recipe*> others;
  recipes.clear();
  ItemContainer inven(&world->player->inven, &world->player->backpack);
  for (Recipe* recipe : recipestorage->recipes) {
    if (recipe->display(&inven, nullptr)) {
      possible.push_back(recipe);
    } else {
      others.push_back(recipe);
    }
  }
  num_possible = possible.size();
  for (Recipe* recipe : possible) {
    recipes.push_back(recipe);
  }
  for (Recipe* recipe : others) {
    recipes.push_back(recipe);
  }
}

void CraftingMenu::render_page() {
  ItemContainer inven(&world->player->inven, &world->player->backpack);
  for (int i = 0; i < len_page; i ++) {
    if (i+page*len_page < recipes.size()) {
      outputs[i] = recipes[i+page*len_page]->output;//ItemContainer(4);
      //cout << recipes[i+page*len_page]->display(&inven, &outputs[i]) << endl;
      inputs[i] = recipes[i+page*len_page]->input;
    } else {
      inputs[i] = ItemContainer(0);
      outputs[i] = ItemContainer(0);
    }
  }
}

void CraftingMenu::render(GLFWwindow* window, World* world, Player* player, MemVecs* uivecs) {
  /////rendering
  
  glfwGetCursorPos(window, &xpos, &ypos);
  xpos = xpos/screen_x*2-1;
  ypos = 1-ypos/screen_y*2;
  
  player->inven.render(uivecs, -0.5f, -1);
  player->backpack.render(uivecs, -0.5f, -0.5f);
  for (int i = 0; i < len_page; i ++) {
    if (inputs[i].size > 0) {
      outputs[i].render(uivecs, -0.5f+(inputs[i].size+1)*0.1f, 0.8f-0.12f*i*aspect_ratio);
      inputs[i].render(uivecs, -0.5f, 0.8f-0.12f*i*aspect_ratio);
      draw_icon(uivecs, 0, -0.5f+(inputs[i].size)*0.1f, 0.8f-0.12f*i*aspect_ratio, 0.1f, 0.1f*aspect_ratio);
      if (page*len_page + i >= num_possible) {
        draw_icon(uivecs, 5, -0.5f, 0.83f-0.12f*i*aspect_ratio, (inputs[i].size+outputs[i].size+1)*0.1f);
      }
    }
  }
  
  if (!in_hand.item.isnull) {
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
    }
    
    if (xpos < 0.5f and xpos > -0.5f) {
      index = int(xpos*10+5);
    }
    bool changed = false;
    if (container != nullptr and index != -1) {
      if (button == 1) {
        if (in_hand.item.data != container->items[index].item.data or !in_hand.item.data->stackable or ! container->items[index].item.data->stackable) {
          // different items in hand and spot, swap
          ItemStack tmp = in_hand;
          in_hand = container->items[index];
          container->items[index] = tmp;
          changed = true;
        } else {
          container->items[index].count += in_hand.count;
          in_hand = ItemStack(nullptr, 0);
          changed = true;
        }
      } else if (button == 2) {
        if (in_hand.item.isnull) {
          in_hand = container->items[index];
          int total = in_hand.count;
          in_hand.count = total/2;
          container->items[index].count = total/2 + total%2;
          changed = true;
        }
      }
    }
    if (container == nullptr and index != -1) {
      for (int i = 0; i < len_page; i ++) {
        if (ypos >  0.8f-0.12f*i*aspect_ratio and ypos <=  (0.8f-0.12f*i+0.1f)*aspect_ratio) {
          ItemContainer inven(&world->player->inven, &world->player->backpack);
          if (recipes[page*len_page+i]->display(&inven, nullptr)) {
            recipes[page*len_page+i]->craft(&inven, &inven);
            for (int i = 0; i < world->player->inven.size; i ++) {
              world->player->inven.items[i] = inven.items[i];
            }
            for (int i = 0; i < world->player->backpack.size; i ++) {
              world->player->backpack.items[i] = inven.items[world->player->backpack.size + i];
            }
            changed = true;
            break;
          }
        }
      }
    }
    if (container == nullptr and index == -1) {
      if (xpos < -0.5f and page > 0) {
        page --;
        changed = true;
      } else if (xpos > 0.5f) {
        page ++;
        changed = true;
      }
    }
    if (changed) {
      get_recipes();
      render_page();
    }
  }
}

void CraftingMenu::close(World* world) {
  end(world);
  after();
}

////////////////////////////////// select /////////////////////////////////////////////

SelectMenu::SelectMenu(string head, vector<string> & opts, function<void(string)> after_func): header(head), after(after_func) {
    options.swap(opts);
    chosen = "";
    start();
    click = true;
}

void SelectMenu::render(GLFWwindow* window, World* world, Player* player, MemVecs* uivecs) {
    int i = 0;
    draw_text(uivecs, header, -0.5, 0.75f, 3);
    for (string name : options) {
        draw_text(uivecs, name, -0.4f, 0.5f - i*0.1f, 2);
        i++;
    }
    bool last_click = click;
    click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (click and !last_click) {
	      double xpos, ypos;
	      glfwGetCursorPos(window, &xpos, &ypos);
        int index = (int)((double)ypos/screen_y*20) - 4;
        if (index >= 0 and index < options.size()) {
            chosen = options[index];
        }
    }
    last_click = click;
    if (chosen != "") {
        close(world);
    }
}

void SelectMenu::close(World* world) {
  end(world);
  after(chosen);
}

///////////////////////////////////////////////////////////

string text_buff;
bool filter;

void special_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
    cout << "backspace" << endl;
    text_buff += '\b';
  }
}

void key_callback(GLFWwindow* window, unsigned int codepoint)
{
  char lett(codepoint);
  cout << codepoint << endl;
  if (filter) {
    if ((codepoint >= 65 and codepoint <= 90) or (codepoint >= 97 and codepoint <= 122)) {
      text_buff += lett;
    }
    if (lett == ' ') {
      text_buff += '-';
    }
  } else {
    text_buff += lett;
  }
}

TextInputMenu::TextInputMenu(string head, bool have_filter, function<void(string)> after_func): header(head), after(after_func) {
  start();
  filter = have_filter;
  glfwSetCharCallback(window, key_callback);
  glfwSetKeyCallback(window, special_callback);
}

void TextInputMenu::render(GLFWwindow* window, World* world, Player* player, MemVecs* uivecs) {
  draw_text(uivecs, header, -0.5, 0.75f, 3);
  draw_text(uivecs, text + '|', -0.5, 0.0f, 2);
  if (text_buff != "") {
    for (char c : text_buff) {
      if (c == '\b') {
        text = text.substr(0, text.length()-1);
      } else {
        text += c;
      }
    }
    text_buff = "";
  }
  if (glfwGetKey(window, GLFW_KEY_ENTER ) == GLFW_PRESS) {
    close(world);
  }
}

void TextInputMenu::close(World* world) {
  glfwSetCharCallback(window, NULL);
  glfwSetKeyCallback(window, NULL);
  end(world);
  after(text);
}






#endif
