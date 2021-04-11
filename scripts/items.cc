#ifndef ITEMS
#define ITEMS

#include "items.h"

#include "ui.h"
#include "world.h"
#include "blockdata.h"
#include "cross-platform.h"
#include "materials.h"
#include "blocks.h"
#include "entity.h"
#include "text.h"
#include "blockgroups.h"
#include "glue.h"



ItemStorage* itemstorage;

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

bool CharArray::place(World* world, Item* item, ivec3 pos, ivec3 dir) {
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    int dx = dir.x;
    int dy = dir.y;
    int dz = dir.z;
    int direction = dy+dz*2;
    if (dx < 0 or dy < 0 or dz < 0) {
      direction = direction*-1+3;
    }
    vector<Pixel*> pixels;
    
    for (int i = 0; i < sx; i ++) {
        for (int j = 0; j < sy; j ++) {
            for (int k = 0; k < sz; k ++) {
              int px = i;
              int py = j - (sy/2);
              int pz = k - (sz/2);
              char worldval = world->get(x+(px*dx + py*dy + pz*dz), y+(px*dy + py*dz + pz*dx), z+(px*dz + py*dx + pz*dy));
              if (worldval != 0 and worldval != 7) {
                return false;
              }
            }
        }
    }
    for (int i = 0; i < sx; i ++) {
        for (int j = 0; j < sy; j ++) {
            for (int k = 0; k < sz; k ++) {
                int px = i;
                int py = j - (sy/2);
                int pz = k - (sz/2);
                world->set(x+(px*dx + py*dy + pz*dz), y+(px*dy + py*dz + pz*dx), z+(px*dz + py*dx + pz*dy), get(i,j,k), direction);
                
                pixels.push_back(world->get_global(x+(px*dx + py*dy + pz*dz), y+(px*dy + py*dz + pz*dx), z+(px*dz + py*dx + pz*dy), 1)->pixel);
            }
        }
    }
    
    
    if (item->data->isgroup) {
      BlockGroup* group = new BlockGroup(world, pixels);
      delete group;
      //group->item = *item;
    }
    return true;
}

///


Item::Item(ItemData* newdata): data(newdata), isnull(data == nullptr), modified(false) {
  if (!isnull) {
    sharpness = data->starting_sharpness;
    weight = data->starting_weight;
    reach = data->starting_reach;
    stackable = data->stackable;
    durability = 1;
  }
}

Item::Item(ItemData* newdata, double newsharpness, double newweight, double newreach): data(newdata), isnull(data == nullptr),
sharpness(newsharpness), weight(newweight), reach(newreach), stackable(false), modified(true), durability(1) {
  
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
      durability = 1;
    }
    while (ifile.peek() == ' ' and !ifile.eof()) {
      ifile.get();
    }
    
    if (ifile.peek() == '%') {
      ifile.get();
      ifile >> durability;
    } else {
      durability = 1;
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
    head->durability -= item_damage;
    //head->modified = true;
  }
  
  if (!stackable) {
    trim();
  }
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
    if (addons[i].data->sharpenable and addons[i].durability <= 0) {
      addons.erase(addons.begin() + i);
    } else {
      addons[i].trim();
    }
  }
  
  if (!isnull and data->sharpenable and durability <= 0) {
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
    if (durability < 1) {
      ofile << '%' << durability << ' ';
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
  return ondig_null(world, x, y, z);
}

char Item::ondig_null(World* world, int x, int y, int z) {
  char val = world->get(x, y, z);
  world->set(x, y, z, 0);
  return val;
  /*
  Pixel* pix = world->get_global(x, y, z, 1)->get_pix();
  while (pix->scale != 1) {
    pix->subdivide();
    pix = world->get_global(x, y, z, 1)->get_pix();
  }
  if (pix->group != nullptr) {
    if (pix->group->final) {
      for (int i = 0; i < pix->group->connections.size(); i ++) {
        if (pix->group->connections[i].data == nullptr) {
          pix->group->connections[i].damage ++;
          if (pix->group->connections[i].damage > 6) {
            pix->group->del_connection(i);
            ivec3 gpos;
            pix->global_position(&gpos.x, &gpos.y, &gpos.z);
            world->block_update(gpos.x, gpos.y, gpos.z);
          }
          for (Pixel* grouppix : pix->iter_group()) {
            grouppix->render_update();
          }
          return 0;
        }
      }
      return 0;
    }
  }
  BlockGroup* oldgroup = pix->group;
  BlockGroup* group = new BlockGroup(world);
  if (pix->group != nullptr) {
    pix->group->del(pix);
  }
  group->spread(pix, 2, -1, 0.5f);
  cout << group << ' ' << pix->group << endl;
  group->final = true;
  for (Pixel* grouppix : pix->iter_group()) {
    grouppix->render_update();
  }
  if (oldgroup != nullptr) {
    group->connect_to(GroupConnection(oldgroup, nullptr));
    group->get_connection(oldgroup)->damage = 1;
  }
  return 0;*/
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
    if (durability < 1) {
      draw_icon(vecs, 12, x, y, scale, durability*scale*aspect_ratio);
    }
    int i = 0;
    for (Item& item : addons) {
      item.render(vecs, x + scale*i*0.3f, y + scale, scale);
      i ++;
    }
  }
}


/////////////////////////////////////// ITEMdada ////////////////////////////////

ItemData::ItemData(string newname, CharArray* arr): name(newname),
stackable(true), onplace(arr), rcaction("null"), starting_weight(0),
starting_sharpness(0), starting_reach(0), sharpenable(false), lightlevel(0),
isgroup(true), glue(nullptr), texture(0) {
  
}


ItemData::ItemData(ifstream & ifile):
stackable(true), onplace(nullptr), rcaction("null"), starting_weight(0),
starting_sharpness(0), starting_reach(0), sharpenable(false), lightlevel(0),
isgroup(true), glue(nullptr) {
  
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
      } else if (varname == "gluetype") {
        string gluetype;
        ifile >> gluetype;
        glue = connstorage->connectors[gluetype];
      } else if (varname == "notgroup") {
        isgroup = false;
      } else if (varname == "lightlevel") {
        ifile >> lightlevel;
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

ItemStack::ItemStack(istream& ifile): item(ifile) {
  ifile >> count;
  if (ifile.peek() == '(') {
    ifile.get();
    do {
      unique.emplace_back(ifile);
      while (ifile.peek() == ' ' and !ifile.eof()) {
        ifile.get();
      }
    } while (ifile.peek() != ')');
    ifile.get();
  }
}

void ItemStack::to_file(ostream& ofile) {
  item.to_file(ofile);
  ofile << count;
  if (unique.size() > 0) {
    ofile << "( ";
    for (Item ite : unique) {
      ite.to_file(ofile);
    }
    ofile << ")";
  }
  ofile << ' ';
}
  

void ItemStack::render(MemVecs* vecs, float x, float y) {
  if (unique.size() > 0) {
    get_unique()->render(vecs, x, y);
  } else {
    item.render(vecs, x, y);
  }
  draw_text(vecs, std::to_string(count), x+0.02, y+0.02f);
}

bool ItemStack::add(Item newitem) {
  if (isnull()) {
    item = newitem;
  }
  if (newitem.data == item.data and item.stackable and newitem.stackable) {
    if (newitem.durability != 1) {
      unique.push_back(newitem);
    }
    count ++;
    return true;
  }
  return false;
}

bool ItemStack::add(ItemStack newstack) {
  if (isnull()) {
    item = newstack.item;
  }
  if (newstack.item.data == item.data and item.stackable and newstack.item.stackable) {
    count += newstack.count;
    for (Item newunique : newstack.unique) {
      unique.push_back(newunique);
    }
    return true;
  }
  return false;
}

Item ItemStack::take() {
  if (count > 0) {
    count --;
    if (unique.size() > 0) {
      Item tmp = unique.back();
      unique.pop_back();
      return tmp;
    }
    Item ret = item;
    if (count == 0) {
      item = Item(nullptr);
    }
    return ret;
  }
  return Item(nullptr);
}

ItemStack ItemStack::take(int num) {
  if (num >= count) {
    ItemStack result = *this;
    item = Item(nullptr);
    count = 0;
    return result;
  }
  ItemStack result(item, num);
  int i = 0;
  while (i < num and unique.size() > 0) {
    result.unique.push_back(unique.back());
    unique.pop_back();
  }
  count -= num;
  return result;
}

ItemStack ItemStack::half() {
  return take(count/2);
}

Item* ItemStack::get_unique() {
  if (isnull()) {
    return nullptr;
  }
  if (!item.stackable) {
    return &item;
  }
  if (unique.size() == 0) {
    unique.push_back(item);
  }
  return &unique.back();
}

bool ItemStack::isnull() {
  return item.isnull;
}

void ItemStack::trim() {
  for (int i = unique.size() - 1; i >= 0; i --) {
    unique[i].trim();
    if (unique[i].isnull) {
      unique.erase(unique.begin() + i);
      count --;
    }
  }
  if (count == 0) {
    item = Item(nullptr);
  }
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

Item ItemStorage::from_group(Pixel* startpix) {
  BlockGroupIter iter = startpix->iter_group();
  
  bool first_pix = true;
  ivec3 position(0,0,0);
  ivec3 dims(1,1,1);
  
  for (Pixel* pix : iter) {
    ivec3 pos = pix->parbl->globalpos;
    if (first_pix) {
      position = pos;
      first_pix = false;
    } else {
      if (pos.x < position.x) {
        dims.x += position.x - pos.x;
        position.x = pos.x;
      }
      if (pos.y < position.y) {
        dims.y += position.y - pos.y;
        position.y = pos.y;
      }
      if (pos.z < position.z) {
        dims.z += position.z - pos.z;
        position.z = pos.z;
      }
      if (pos.x >= position.x + dims.x) dims.x = pos.x - position.x + 1;
      if (pos.y >= position.y + dims.y) dims.y = pos.y - position.y + 1;
      if (pos.z >= position.z + dims.z) dims.z = pos.z - position.z + 1;
    }
  }
  
  
  char* data = new char[dims.x * dims.y * dims.z];
  for (int i = 0; i < dims.x * dims.y * dims.z; i ++) {
    data[i] = 0;
  }
  
  for (Pixel* pix : iter) {
    ivec3 pos = pix->parbl->globalpos;
    ivec3 lpos = pos - position;
    data[lpos.x*dims.y*dims.z + lpos.y*dims.z + lpos.z] = pix->value;
  }
  
  for (int dirindex = 0; dirindex < 6; dirindex ++) {
    for (pair<string,ItemData*> kv : items) {
      CharArray* carray = kv.second->onplace;
      if (carray != nullptr and carray->sx == dims[(0+dirindex)%3] and carray->sy == dims[(1+dirindex)%3] and carray->sz == dims[(2+dirindex)%3]) {
        bool matching = true;
        for (int x = 0; x < carray->sx and matching; x ++) {
          for (int y = 0; y < carray->sy and matching; y ++) {
            for (int z = 0; z < carray->sz and matching; z ++) {
              ivec3 pos (x,y,z);
              ivec3 npos (
                (dirindex < 3) ? pos[(0+dirindex)%3] : carray->sx - pos[(0+dirindex)%3] - 1,
                (dirindex < 3) ? pos[(1+dirindex)%3] : carray->sy - pos[(1+dirindex)%3] - 1,
                (dirindex < 3) ? pos[(2+dirindex)%3] : carray->sz - pos[(2+dirindex)%3] - 1
              );
              if (carray->get(pos.x, pos.y, pos.z) != data[npos.x*carray->sy*carray->sz + npos.y*carray->sz + npos.z]) {
                matching = false;
              }
            }
          }
        }
        
        if (matching) {
          delete[] data;
          return Item(kv.second);
        }
      }
    }
  }
  
  return Item(nullptr);
  
  int dirindex = 0;
  if (dims.y > dims.z and dims.y > dims.x) {
    dirindex = 1;
  } else if (dims.z > dims.y and dims.z > dims.x) {
    dirindex = 2;
  }
  
  string name = std::to_string(rand());
  
  CharArray* newarr;
  if (dirindex == 0) {
    newarr = new CharArray(data, dims.x, dims.y, dims.z);
    items[name] = new ItemData(name, newarr);
    return Item(items[name]);
  } else {
    ivec3 newdims(dims[dirindex%3], dims[(dirindex+1)%3], dims[(dirindex+2)%3]);
    char* rot_data = new char[newdims.x * newdims.y * newdims.z];
    
    for (int x = 0; x < newdims.x; x ++) {
      for (int y = 0; y < newdims.y; y ++) {
        for (int z = 0; z < newdims.z; z ++) {
          ivec3 pos (x,y,z);
          ivec3 npos (
            (dirindex < 3) ? pos[(0+dirindex)%3] : newdims.x - pos[(0+dirindex)%3] - 1,
            (dirindex < 3) ? pos[(1+dirindex)%3] : newdims.y - pos[(1+dirindex)%3] - 1,
            (dirindex < 3) ? pos[(2+dirindex)%3] : newdims.z - pos[(2+dirindex)%3] - 1
          );
          char val = data[npos.x*newdims.y*newdims.z + npos.y*newdims.z + npos.z];
          rot_data[pos.x*newdims.y*newdims.z + pos.y*newdims.z + pos.z] = val;
        }
      }
    }
    
    newarr = new CharArray(rot_data, newdims.x, newdims.y, newdims.z);
    items[name] = new ItemData(name, newarr);
    delete[] data;
    return Item(items[name]);
  }
  
  delete[] data;
  return Item(nullptr);
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
    items.emplace_back(ifile);
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
      if (!items[i].isnull() and items[i].add(itemstack)) {
        return true;
      }
    }
  }
  for (int i = 0; i < items.size(); i ++) {
    if (items[i].add(itemstack)) {
      return true;
    }
  }
  return false;
}

Item* ItemContainer::get(int index) {
    if (index < items.size()) {
        return &items[index].item;
    }
    return nullptr;
}

Item ItemContainer::use(int index) {
    if (index < items.size()) {
        Item ret = items[index].item;
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
    if (is.item.data == itemstack.item.data and !is.item.modified) {
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
    itemstack.to_file(ofile);
  }
  ofile << endl;
}

#endif
