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
#include "materials-predef.h"

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


Item::Item(ItemData* newdata): data(newdata), isnull(data == nullptr), modified(false) {
  if (!isnull) {
    sharpness = data->starting_sharpness;
    weight = data->starting_weight;
    reach = data->starting_reach;
    stackable = data->stackable;
  }
}

Item::Item(ItemData* newdata, double newsharpness, double newweight, double newreach): data(newdata), isnull(data == nullptr),
sharpness(newsharpness), weight(newweight), reach(newreach), stackable(false), modified(true) {
  
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
      ifile >> name >> sharpness >> weight >> reach;
      data = itemstorage->items[name];
      modified = true;
      stackable = false;
    } else {
      data = itemstorage->items[name];
      sharpness = data->starting_sharpness;
      weight = data->starting_weight;
      reach = data->starting_reach;
      modified = false;
      stackable = data->stackable;
    }
    while (ifile.peek() == ' ' and !ifile.eof()) {
      ifile.get();
    }
    if (ifile.peek() == '[') {
      ifile.get();
      do {
        addons.emplace_back(ifile);
        while (ifile.peek() == ' ' and !ifile.eof()) {
          ifile.get();
        }
      } while (ifile.peek() != ']');
      ifile.get();
    }
  }
}

pair<Item*,double> Item::get_head(double depth) {
  if (addons.size() > 0) {
    double highest_depth = -1;
    Item* head = nullptr;
    for (Item& item : addons) {
      pair<Item*,int> result = item.get_head(depth + reach);
      if (result.second >= highest_depth) {
        head = result.first;
        highest_depth = result.second;
      }
    }
    return pair<Item*,double>(head, highest_depth);
  } else if (!isnull and data->sharpenable) {
    return pair<Item*,double>(this, depth + reach);
  } else {
    return pair<Item*,int>(nullptr, -1);
  }
}

double Item::get_sharpness() {
  Item* head = get_head().first;
  if (head == nullptr) {
    return 0;
  } else {
    return head->sharpness;
  }
}

double Item::get_weight(double totalreach) {
  double new_weight = 0;
  if (!isnull and data->sharpenable) {
    new_weight = weight;
    totalreach += reach;
  }
  for (Item& addon : addons) {
    new_weight += addon.get_weight(totalreach) * totalreach;
  }
  return new_weight;
}

double Item::get_reach() {
  return get_head().second;
}

string Item::get_name() {
  string name = data->name;
  if (addons.size() > 0) {
    name = addons[0].get_name() + " on a " + name;
  }
  if (addons.size() > 1) {
    for (int i = 1; i < addons.size(); i ++) {
      name = addons[i].get_name() + " and " + name;
    }
  }
  return name;
}

string Item::descript() {
  if (!data->sharpenable) {
    return get_name();
  } else {
    stringstream ss;
    ss.precision(2);
    ss << get_name()
    << "\nSharpness " << get_sharpness()
    << "\nWeight " << get_weight()
    << "\nReach " << get_reach()
    << "\nSpeed " << get_recharge_time();
    return ss.str();
  }
}

// void Item::damage(Player* player, BlockData* blockdata, double time) {
//   if (isnull or !data->sharpenable) {
//     return;
//   }
//   double force = std::max(get_weight(),1.0);
//   Item* head = get_head().first;
//   if (head != nullptr) {
//     double sharp = head->sharpness;
//     double dig_time = blockdata->material->dig_time(head->data->material, sharp, force);
//     double multiplier = 1/(1+std::exp(head->data->material->material_score(blockdata->material))) * 2;
//     double damage_per_sec = force / head->data->material->toughness * multiplier * 0.1;//head->data->starting_weight / dig_time * multiplier;
//     double added_sharpness = sharp - head->data->starting_sharpness;
//     if (added_sharpness > 0) {
//       double decrement = damage_per_sec * time;//std::max(0.1*time, sharp*std::pow(0.9, 1/time));
//       head->sharpness -= decrement;
//       head->weight -= decrement/3;
//     } else {
//       double decrement = damage_per_sec * time;
//       if (head->sharpness > 0) {
//         head->sharpness -= decrement/3;
//       } else {
//         head->sharpness = 0;
//       }
//       head->weight -= decrement;
//     }
//     head->modified = true;
//     trim();
//     if (weight < 0) {
//       isnull = true;
//       weight = 1;
//       sharpness = 1;
//     }
//   }
// }

double Item::collision(Pixel* pix) {
  BlockData* blockdata = blocks->blocks[pix->value];
  double force = get_weight();
  Item* head = get_head().first;
  double sharp = get_sharpness();
  Material* mat;
  if (head == nullptr or head->isnull or !head->data->sharpenable) {
    sharpness = 1;
    force = 1;
    mat = matstorage->materials["fist"];
  } else {
    mat = head->data->material;
  }
  //cout << 's' << sharp << " f" << force << endl;
  double mat_collision = blockdata->material->collision_force(mat, sharp, force);
  if (mat_collision < 1) {
    mat_collision = std::pow(0.8, (1-mat_collision));
  }
  //cout << "mat coll " << mat_collision << endl;
  double leftover_sharp = sharp - blockdata->material->elastic - mat->elastic;
  //cout << "leftover sharp " << leftover_sharp << endl;
	double score = blockdata->material->material_score(mat);// + leftover_sharp;
	double multiplier = 1/(1+exp(0.4*score));
  //cout << "multiplier " << multiplier << endl;
  
  double block_damage = (mat_collision * multiplier) / blockdata->material->toughness;
  double item_damage = 1-multiplier;
  //cout << "bni damage " << block_damage << ' ' << item_damage << endl;
  
  if (head != nullptr and !head->isnull and head->data->sharpenable) {
    item_damage /= head->weight;
    head->weight -= item_damage;
    head->modified = true;
    head->stackable = false;
  }
  
  trim();
  return block_damage;
}

double Item::get_recharge_time() {
  if (isnull or !data->sharpenable) {
    return 0.25;
  } else {
    return 0.25 + get_weight() / 8;
  }
}

void Item::trim() {
  for (int i = addons.size() - 1; i >= 0; i --) {
    if (addons[i].data->sharpenable and addons[i].weight <= 0) {
      addons.erase(addons.begin() + i);
    } else {
      addons[i].trim();
    }
  }
  
  if (!isnull and data->sharpenable and weight <= 0) {
    isnull = true;
    data = nullptr;
    addons.clear();
  }
}

// double Item::dig_time(char val) {
//   double time;
//   Item* head = get_head().first;
//   Material* mater;
//   double sharp;
//   double force;
//   if (head != nullptr) {
//     mater = head->data->material;
//     sharp = head->sharpness;
//     force = std::max(get_weight(),1.0);
//   } else {
//     mater = matstorage->materials["fist"];
//     sharp = 1;
//     force = 1;
//   }
//   BlockData* blockdata = blocks->blocks[val];
//   time = blockdata->material->dig_time(mater, sharp, force);
//   return time;
// }

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
    if (modified) {
      ofile << "~ " << data->name << ' ' << sharpness << ' ' << weight << ' ' << reach << ' ';
    } else {
      ofile << data->name << ' ';
    }
    if (addons.size() > 0) {
      ofile << "[ ";
      for (Item& item : addons) {
        item.to_file(ofile);
      }
      ofile << "] ";
    }
  }
}

char Item::ondig(World* world, int x, int y, int z) {
    char type = world->get(x,y,z);
    world->set(x,y,z,0);
    return type;
}

char Item::ondig_null(World* world, int x, int y, int z) {
    char type = world->get(x,y,z);
    world->set(x,y,z,0);
    return type;
}

void Item::sharpen(double speed, double force) {
  Item* head = get_head().first;
  if (head != nullptr) {
    head->sharpness += speed*0.1;
    head->weight -= speed*0.01;
    head->modified = true;
    head->stackable = false;
  }
}

void Item::render(MemVecs* vecs, float x, float y, float scale) {
  if (isnull) {
    draw_image_uv(vecs, "items.bmp", x, y, scale, scale*aspect_ratio, 0, 1, 0.0f, 1.0f/itemstorage->total_images);
  } else {
    draw_image_uv(vecs, "items.bmp", x, y, scale, scale*aspect_ratio, 0, 1, float(data->texture)/itemstorage->total_images, float(data->texture+1)/itemstorage->total_images);
    int i = 0;
    for (Item& item : addons) {
      item.render(vecs, x + scale*i*0.3f, y + scale, scale);
      i ++;
    }
  }
}


/////////////////////////////////////// ITEMdada ////////////////////////////////

ItemData::ItemData(ifstream & ifile):
stackable(true), onplace(nullptr), rcaction("null"), starting_weight(0),
starting_sharpness(0), starting_reach(0), sharpenable(false) {
  
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
      } else if (varname == "material") {
        string matname;
        ifile >> matname;
        material = matstorage->materials[matname];
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
      } else if (varname == "reach") {
        ifile >> starting_reach;
        sharpenable = true;
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
  item.render(vecs, x, y);
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


ItemContainer::ItemContainer(int newsize): size(newsize) {
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
    if (itemstack.item.stackable) {
      for (int i = 0; i < items.size(); i ++) {
        if (itemstack.item.data == items[i].item.data and items[i].item.stackable) {
          //cout << "add " << items[i].item.stackable << ' ' << items[i].item.get_name() << endl;
          //and itemstack.item.addons.size() < 0 and items[i].item.addons.size() < 0) {
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

void ItemContainer::make_single(int index) {
  if (items[index].count > 1) {
    ItemStack newstack = items[index];
    newstack.count --;
    items[index].item.stackable = false;
    add(newstack);
    newstack.count = 1;
    items[index] = newstack;
  }
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
