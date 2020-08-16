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
    ifile >> sx >> sy >> sz;
    getline(ifile, buff, ':');
    arr = new char[sx*sy*sz];
    for (int i = 0; i < sx*sy*sz; i ++) {
        int j;
        ifile >> j;
        arr[i] = (char)j;
    }
}

CharArray::~CharArray() {
  delete[] arr;
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
    stringstream ss;
    ss << data->name << "\nSharpness " << sharpness << "\nWeight " << weight;
    return ss.str();
  }
}

void Item::damage(Player* player, BlockData* data) {
  sharpness -= 0.1;
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
    getline(ss, command, '-');
    if (command == "heal") {
      double amount, speed;
      ss >> amount;
      char lett;
      ss >> lett;
      ss >> speed;
      world->player->heal(amount, speed);
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

void Item::sharpen(double speed, double force) {
  sharpness += speed*0.1;
  weight -= speed*0.01;
}

/////////////////////////////////////// ITEMdada ////////////////////////////////

ItemData::ItemData(ifstream & ifile):
stackable(true), onplace(nullptr), rcaction("null"), starting_weight(0),
starting_sharpness(0), tool("null"), sharpenable(false) {
  
  string buff;
  ifile >> buff;
  if (buff != "item") {
    cout << "ERR: this file is not a item data file " << endl;
  } else {
    string tex_str;
    ifile >> name;
    getline(ifile, buff, '{');
    string varname;
    getline(ifile, buff, ':');
    getline(ifile, varname, ':');
    while (!ifile.eof() and varname != "") {
      if (varname == "onplace") {
        onplace = new CharArray(ifile);
      } else if (varname == "block") {
        string blockname;
        ifile >> blockname;
        char* arr = new char[1];
        arr[0] = blocks->names[blockname];
        onplace = new CharArray(arr, 1, 1, 1);
      } else if (varname == "non_stackable") {
        stackable = false;
      } else if (varname == "rcaction") {
        ifile >> rcaction;
      } else if (varname == "toughness") {
        ifile >> toughness;
      } else if (varname == "weight") {
        ifile >> starting_weight;
        sharpenable = true;
      } else if (varname == "sharpness") {
        ifile >> starting_sharpness;
        sharpenable = true;
      } else if (varname == "tool") {
        ifile >> tool;
      } else if (varname == "texture") {
        ifile >> tex_str;
      }
      getline(ifile, buff, ':');
      getline(ifile, varname, ':');
    }
    if (tex_str == "") {
      tex_str = name;
    }
    vector<string> paths;
    get_files_folder("resources/textures/items", &paths);
    texture = 0;
    for (int i = 0; i < paths.size(); i ++) {
      stringstream ss(paths[i]);
      string filename;
      getline(ss, filename, '.');
      if (filename == tex_str) {
        texture = i;
      }
    }
    //texture = float(tex_id) / paths.size();
    //cout << tex_id << ' ' << paths.size() << ' ' << texture << endl;
  }
}

ItemData::~ItemData() {
  delete onplace;
}


/////////////////////////////////////////////////////////////////////////////////////

ItemStack::ItemStack(Item newitem, int newcount): item(newitem), count(newcount) {
  
}

void ItemStack::render(MemVecs* vecs, float x, float y) {
  draw_image_uv(vecs, "items.bmp", x, y, 0.1f, 0.1f*aspect_ratio, 0, 1, float(item.data->texture)/itemstorage->total_images, float(item.data->texture+1)/itemstorage->total_images);
  draw_text(vecs, std::to_string(count), x+0.02, y+0.02f);
}

/////////////////////////////// ITEMSTRAGE////////////////////////////////////////////////

    ItemStorage::ItemStorage() {
        vector<string> image_paths;
        get_files_folder("resources/textures/items", &image_paths);
        total_images = image_paths.size();
        vector<string> block_paths;
        get_files_folder("resources/data/items", &block_paths);
        for (string filename : block_paths) {
            ifstream ifile("resources/data/items/" + filename);
            //cout << "loading itemdata " << filename << " from file" << endl;
            ItemData* itemdata = new ItemData(ifile);
            items[itemdata->name] = itemdata;
        }
        cout << "loaded " << block_paths.size() << " itemdata files" << endl;
    }

ItemStorage::~ItemStorage() {
  for (pair<string,ItemData*> kv : items) {
    delete kv.second;
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
