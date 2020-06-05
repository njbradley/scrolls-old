#ifndef ITEMS
#define ITEMS
#include <iostream>
using std::cout;
using std::endl;
#include <functional>
using std::function;
#include "ui.h"
#include "items-predef.h"
#include "world-predef.h"
#include "blockdata-predef.h"
#include "cross-platform.h"


//////////////////////////// CHARARRAY ////////////////////////////////////

CharArray::CharArray( char* newarr, int x, int y, int z): arr(newarr), sx(x), sy(y), sz(z) { }

CharArray::CharArray(ifstream & ifile) {
    string buff;
    getline(ifile, buff, ':');
    ifile >> sx >> sy >> sz;
    getline(ifile, buff, ':');
    arr = new char[sx*sy*sz];
    for (int i = 0; i < sx*sy*sz; i ++) {
        int j;
        ifile >> j;
        arr[i] = (char)j;
    }
}

char CharArray::get(int x, int y, int z) {
    //cout << x << ' ' << y << ' ' << z << ' ' << x*sy*sz+ y*sz+ z << endl;
    return arr[x*sy*sz+ y*sz+ z];
}

void CharArray::set(char val, int x, int y, int z) {
    arr[x*sy*sz + y*sz + z] = val;
}

void CharArray::place(World* world, int x, int y, int z, int dx, int dy, int dz) {
    int direction = dy+dz*2;
    if (dx < 0 or dy < 0 or dz < 0) {
      direction = direction*-1+3;
    }
    cout << direction << ' ' << dx << ' ' << dy << ' ' << dz << endl;
    for (int i = 0; i < sx; i ++) {
        for (int j = 0; j < sy; j ++) {
            for (int k = 0; k < sz; k ++) {
                int px = i;
                int py = j - (sy/2);
                int pz = k - (sz/2);
                world->set(x+(px*dx + py*dy + pz*dz), y+(px*dy + py*dz + pz*dx), z+(px*dz + py*dx + pz*dy), get(i,j,k), direction);
            }
        }
    }
}

///


Item::Item(ItemData* newdata): data(newdata), isnull(data == nullptr) {
  if (!isnull) {
    sharpness = data->starting_sharpness;
    weight = data->starting_weight;
  }
}

Item::Item(ItemData* newdata, double newsharpness, double newweight): data(newdata), isnull(data == nullptr),
sharpness(newsharpness), weight(newweight) {
  
}

Item::Item(istream& ifile) {
  string name;
  ifile >> name;
  if (name == "null") {
    data = nullptr;
    isnull = true;
  } else {
    isnull = false;
    if (name == "~") {
      ifile >> name >> sharpness >> weight;
      data = itemstorage->items[name];
    } else {
      data = itemstorage->items[name];
      sharpness = data->starting_sharpness;
      weight = data->starting_weight;
    }
  }
}

string Item::descript() {
  if (data->tool == "null") {
    return data->name;
  } else {
    return data->name + "\nSharpness " + std::to_string(sharpness) + "\nWeight " + std::to_string(weight);
  }
}

void Item::damage(double time) {
  sharpness -= 0.1/data->damage;
  if (sharpness < 0) {
    sharpness = 0;
  }
}

double Item::dig_time(char val) {
  double time;
  if (isnull) {
    return blocks->blocks[val]->hardness["null"];
  } else {
    if (data->tool == "null") {
      time = blocks->blocks[val]->hardness[data->tool];
    } else {
      if (sharpness <= 0) {
        time = 999999999;
      }
      time = blocks->blocks[val]->hardness[data->tool]/(sharpness*weight); //balance point
    }
    double nulltime = blocks->blocks[val]->hardness["null"];
    if (nulltime < time) {
      return nulltime;
    } else {
      return time;
    }
  }
}

bool Item::do_rcaction(World* world) {
  if (!isnull and data->rcaction != "null") {
    stringstream ss(data->rcaction);
    string command;
    cout << data->rcaction << ' ';
    getline(ss, command, '-');
    cout << command << endl;
    if (command == "heal") {
      int amount;
      ss >> amount;
      world->player->health += amount;
    }
    return true;
  }
  return false;
}

void Item::to_file(ostream& ofile) {
  if (data == nullptr) {
    ofile << "null ";
  } else {
    if (data->tool != "null") {
      ofile << "~ " << data->name << ' ' << sharpness << ' ' << weight << ' ';
    } else {
      ofile << data->name << ' ';
    }
  }
}

char Item::ondig(World* world, int x, int y, int z) {
    char type = world->get(x,y,z);
    world->set(x,y,z,0);
    return type;
    //double random = (rand()%1000/1000.0);
    //double prob = blocks->blocks[type]->hardness[tool];// - ((damage-1)-time)*0.2;
    //cout << prob << ' ' << random << endl;
    //if (prob > random) {
    //    world->set(x,y,z,0);
    //}
    //dig_pickaxe(world, x, y, z, random);
}

char Item::ondig_null(World* world, int x, int y, int z) {
    char type = world->get(x,y,z);
    world->set(x,y,z,0);
    return type;
}

/////////////////////////////////////// ITEMdada ////////////////////////////////

ItemData::ItemData(ifstream & ifile) {
    int isarray;
    ifile >> isarray;
    if (isarray) {
        onplace = new CharArray(ifile);
    } else {
        onplace = nullptr;
    }
    string buff;
    getline(ifile, buff, ':');
    ifile >> name >> texture >> tool >> damage >> stackable >> rcaction;
    if (tool != "null") {
      cout << name << ' ' << tool << ' ';
      getline(ifile, buff, ':');
      ifile >> starting_sharpness >> starting_weight;
      cout << starting_sharpness << ' ' << starting_weight << endl;
    }
}


/////////////////////////////////////////////////////////////////////////////////////

ItemStack::ItemStack(Item newitem, int newcount): item(newitem), count(newcount) {
  
}

void ItemStack::render(MemVecs* vecs, float x, float y) {
  draw_image_uv(vecs, "items.bmp", x, y, 0.1f, 0.1f*aspect_ratio, item.data->texture/64.0f, (item.data->texture+1)/64.0f);
  draw_text(vecs, std::to_string(count), x+0.02, y+0.02f);
}

/////////////////////////////// ITEMSTRAGE//////////////////////////////////////

    ItemStorage::ItemStorage() {
        vector<string> block_paths;
        get_files_folder("resources/data/items", &block_paths);
        for (string filename : block_paths) {
            stringstream name_stream(filename);
            string name;
            getline(name_stream, name, '.');
            ifstream ifile("resources/data/items/" + filename);
            cout << "loading item " << name << " from file" << endl;
            items[name] = new ItemData(ifile);
        }
    }


////////////////////////////////////// ITEMKCINTAINER ///////////////////////////////////


ItemContainer::ItemContainer(int newsize = 10): size(newsize) {
    for (int i = 0; i < newsize; i ++) {
        items.push_back(ItemStack(Item(nullptr),0));
    }
}

ItemContainer::ItemContainer(istream& ifile) {
  ifile >> size;
  for (int i = 0; i < size; i ++) {
    int count;
    Item item(ifile);
    ifile >> count;
    items.push_back(ItemStack(item, count));
  }
}

ItemContainer::ItemContainer(ItemContainer* first, ItemContainer* second) {
  size = first->size + second->size;
  items.insert(items.end(), first->items.begin(), first->items.end());
  items.insert(items.end(), second->items.begin(), second->items.end());
}

bool ItemContainer::add(ItemStack itemstack) {
    if (itemstack.item.data->stackable) {
      for (int i = 0; i < items.size(); i ++) {
        if (itemstack.item.data == items[i].item.data) {
          items[i].count += itemstack.count;
          return true;
        }
      }
    }
    for (int i = 0; i < items.size(); i ++) {
        if (items[i].item.data == nullptr) {
            items[i] = itemstack;
            return true;
        }
    }
    return false;
    //items.push_back(pair<Item*,int>(item,num));
}

Item* ItemContainer::get(int index) {
    if (index < items.size()) {
        return &items[index].item;
    }
    return nullptr;
}

Item* ItemContainer::use(int index) {
    if (index < items.size()) {
        Item* ret = &items[index].item;
        items[index].count--;
        if (items[index].count <= 0) {
          items[index] = ItemStack(nullptr,0);
        }
        return ret;
    }
    return nullptr;
}

bool ItemContainer::contains(ItemStack itemstack) {
  for (ItemStack is : items) {
    if (is.item.data == itemstack.item.data) {
      if (is.count >= itemstack.count) {
        return true;
      } else {
        itemstack.count -= is.count;
      }
    }
  }
  return false;
}

bool ItemContainer::take(ItemStack itemstack) {
  for (int i = 0; i < items.size(); i ++) {
    if (items[i].item.data == itemstack.item.data) {
      if (items[i].count > itemstack.count) {
        items[i].count -= itemstack.count;
        return true;
      } else if (items[i].count == itemstack.count) {
        items[i] = ItemStack(nullptr, 0);
        return true;
      } else {
        itemstack.count -= items[i].count;
        items[i] = ItemStack(nullptr, 0);
      }
    }
  }
  return false;
}

void ItemContainer::render(MemVecs* vecs, float x, float y) {
  //draw_image(vecs, "inven_select.bmp", x, y, 1, 0.1f*aspect_ratio);
  for (int i = 0; i < items.size(); i ++) {
    draw_icon(vecs, 4, x + i*0.1f, y);
    if (items[i].item.data != nullptr) {
      items[i].render(vecs, x + i*0.1f, y+0.02f);
    }
  }
}

void ItemContainer::clear() {
  for (int i = 0; i < size; i ++) {
    items[i] = ItemStack(nullptr, 0);
  }
}

void ItemContainer::save_to_file(ostream& ofile) {
  ofile << size << ' ';
  for (ItemStack itemstack : items) {
    itemstack.item.to_file(ofile);
    ofile << itemstack.count << ' ';
  }
  ofile << endl;
}

#endif
