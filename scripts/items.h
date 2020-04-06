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
    for (int i = 0; i < sx; i ++) {
        for (int j = 0; j < sy; j ++) {
            for (int k = 0; k < sz; k ++) {
                int px = i;
                int py = j - (sy/2);
                int pz = k - (sz/2);
                world->set(get(i,j,k), x+(px*dx + py*dy + pz*dz), y+(px*dy + py*dz + pz*dx), z+(px*dz + py*dx + pz*dy));
            }
        }
    }
}




/////////////////////////////////////// ITEM ////////////////////////////////

Item::Item(ifstream & ifile) {
    int isarray;
    ifile >> isarray;
    if (isarray) {
        onplace = new CharArray(ifile);
    } else {
        onplace = nullptr;
    }
    string buff;
    getline(ifile, buff, ':');
    ifile >> name >> texture >> tool >> damage;
}


char Item::ondig(World* world, int x, int y, int z) {
    char type = world->get(x,y,z);
    world->set(0,x,y,z);
    return type;
    //double random = (rand()%1000/1000.0);
    //double prob = blocks->blocks[type]->hardness[tool];// - ((damage-1)-time)*0.2;
    //cout << prob << ' ' << random << endl;
    //if (prob > random) {
    //    world->set(0,x,y,z);
    //}
    //dig_pickaxe(world, x, y, z, random);
}

char Item::ondig_null(World* world, int x, int y, int z) {
    char type = world->get(x,y,z);
    world->set(0,x,y,z);
    return type;
}

void Item::dig_area(World* world, int x, int y, int z, double random) {
    int size = damage/2;
    for (int ox = -size; ox <= size; ox ++) {
        for (int oy = -size; oy <= size; oy ++) {
            for (int oz = -size; oz <= size; oz ++) {
                char val = world->get(x+ox, y+oy, z+oz);
                double distance = sqrt(ox*ox+oy*oy+oz*oz);
                if (val != 0) {
                    double prob = blocks->blocks[val]->hardness[tool] - distance*0.2;
                    if (prob > random) {
                        world->set(0,x+ox, y+oy, z+oz);
                    }
                }
            }
        }
    }
}
    

void Item::dig_pickaxe(World* world, int x, int y, int z, int time, double random) {
    time --;
    char val = world->get(x,y,z);
    if (time < 0 or val == 0) {
        return;
    }
    BlockData* data = blocks->blocks[val];
    double prob = data->hardness[tool] - ((damage-1)-time)*0.2;
    cout << prob << ' ' << random << endl;
    if (prob > random) {
        world->set(0,x,y,z);
        dig_pickaxe(world, x+1, y, z, time, random);
        dig_pickaxe(world, x-1, y, z, time, random);
        dig_pickaxe(world, x, y+1, z, time, random);
        dig_pickaxe(world, x, y-1, z, time, random);
        dig_pickaxe(world, x, y, z+1, time, random);
        dig_pickaxe(world, x, y, z-1, time, random);
    }
}


/////////////////////////////////////////////////////////////////////////////////////

ItemStack::ItemStack(Item* newitem, int newcount): item(newitem), count(newcount) {
  
}

void ItemStack::render(RenderVecs* vecs, float x, float y) {
  draw_image_uv(vecs, "items.bmp", x, y, 0.1f, 0.1f*aspect_ratio, item->texture/64.0f, (item->texture+1)/64.0f);
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
            items[name] = new Item(ifile);
        }
    }


////////////////////////////////////// ITEMKCINTAINER ///////////////////////////////////


ItemContainer::ItemContainer(int newsize = 10): size(newsize) {
    for (int i = 0; i < newsize; i ++) {
        items.push_back(ItemStack(nullptr,0));
    }
}

ItemContainer::ItemContainer(istream& ifile) {
  ifile >> size;
  for (int i = 0; i < size; i ++) {
    string name;
    int count;
    ifile >> name;
    if (name == "null") {
      items.push_back(ItemStack(nullptr,0));
    } else {
      ifile >> count;
      items.push_back(ItemStack(itemstorage->items[name], count));
    }
  }
}

ItemContainer::ItemContainer(ItemContainer* first, ItemContainer* second) {
  size = first->size + second->size;
  items.insert(items.end(), first->items.begin(), first->items.end());
  items.insert(items.end(), second->items.begin(), second->items.end());
}

bool ItemContainer::add(Item* item, int num) {
    for (int i = 0; i < items.size(); i ++) {
        if (item == items[i].item) {
            items[i].count += num;
            return true;
        }
    }
    for (int i = 0; i < items.size(); i ++) {
        if (items[i].item == nullptr) {
            items[i] = ItemStack(item, num);
            return true;
        }
    }
    return false;
    //items.push_back(pair<Item*,int>(item,num));
}

Item* ItemContainer::get(int index) {
    if (index < items.size()) {
        return items[index].item;
    }
    return nullptr;
}

Item* ItemContainer::use(int index) {
    if (index < items.size()) {
        Item* ret = items[index].item;
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
    if (is.item == itemstack.item) {
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
    if (items[i].item == itemstack.item) {
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

void ItemContainer::render(RenderVecs* vecs, float x, float y) {
  //draw_image(vecs, "inven_select.bmp", x, y, 1, 0.1f*aspect_ratio);
  for (int i = 0; i < items.size(); i ++) {
    draw_icon(vecs, 4, x + i*0.1f, y);
    if (items[i].item != nullptr) {
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
    if (itemstack.item == nullptr) {
      ofile << "null ";
    } else {
      ofile << itemstack.item->name << ' ' << itemstack.count << ' ';
    }
  }
  ofile << endl;
}

#endif
