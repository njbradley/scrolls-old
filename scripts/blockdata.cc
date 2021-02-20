#ifndef BLOCKDATA
#define BLOCKDATA

#include "blockdata.h"

#include "menu.h"
#include "blocks.h"
#include "materials.h"
#include "cross-platform.h"
#include "game.h"

BlockStorage* blocks;

BlockExtra::BlockExtra(Pixel* pix): pixel(pix) {
  // std::stringstream filename;
  // int gx, gy, gz;
  // pix->global_position(&gx, &gy, &gz);
  // filename << "saves/" << world->name << "/blockdata" << gx << 'x' << gy << 'y' << gz << "z.txt";
  // ifstream ifile(filename.str());
  // if (ifile.good()) {
  //   cout << "loading " << filename.str() << " file for block data" << endl;
  //   inven = new ItemContainer(ifile);
  // } else {
    //cout << "generating " << filename.str() << " file cause there is no file" << endl;
    //inven = new ItemContainer(blocks->blocks[pix->value]->extras->inven->size);
  //}
}

BlockExtra::~BlockExtra() {
  //delete inven;
}

BlockExtra::BlockExtra() {}

BlockExtra::BlockExtra(istream& ifile): pixel(nullptr) {
  
}

void BlockExtra::save_to_file(ostream& ofile) {
  //inven->save_to_file(ofile);
  //ofile << ':';
}



DropTable::DropTable(istream& ifile): item_name("null"), count(0), force(0), sharpness(0), prob(1) {
  char lett;
  ifile >> lett;
  if (lett == '(') {
    ifile >> lett;
    while (lett != ')') {
      if (lett == '%') {
        ifile >> prob;
      } else if (lett == 'F') {
        ifile >> force;
      } else if (lett == 'S') {
        ifile >> sharpness;
      }
      ifile >> lett;
    }
    ifile >> lett;
  }
  if (lett == '[') {
    do {
      tables.emplace_back(ifile);
      ifile >> lett;
    } while (lett == ',');
  } else {
    ifile >> item_name >> count;
    //cout << item_name << ',' << lett << ',' << endl;
    item_name = lett + item_name;
  }
}

DropTable::DropTable(): item_name("null"), count(0), force(0), sharpness(0), prob(1) {
  
}

void DropTable::drop(ItemContainer* result, Player* player, Item* break_item) {
  bool is_prob = prob == 1;
  if (!is_prob) {
    double random = rand()%10000/10000.0;
    //cout << random << endl;
    is_prob = prob >= random;
  }
  if (is_prob) {
    if (force == 0 or force <= break_item->get_weight()) {
      if (sharpness == 0 or sharpness <= break_item->get_sharpness()) {
        if (tables.size() > 0) {
          for (int i = 0; i < tables.size(); i ++) {
            tables[i].drop(result, player, break_item);
          }
        } else if (item_name != "null" and count != 0) {
          result->add(ItemStack(Item(itemstorage->items[item_name]), count));
        }
      }
    }
  }
}



BlockData::BlockData(ifstream & ifile):
default_direction(0), rotation_enabled(false), minscale(1), rcaction("null"), lightlevel(0),
clumpyness(0.9), clumpy_group(0), transparent(false) {
  string buff;
  ifile >> buff;
  if (buff != "block") {
    cout << "ERR: this file is not a block data file " << endl;
  } else {
    string tex_str;
    
    ifile >> name;
    getline(ifile, buff, '{');
    string varname;
    getline(ifile, buff, ':');
    getline(ifile, varname, ':');
    while (!ifile.eof() and varname != "") {
      if (varname == "material") {
        string matname;
        ifile >> matname;
        material = matstorage->materials[matname];
      } else if (varname == "textures") {
        getline(ifile, buff, '[');
        getline(ifile, tex_str, ']');
      } else if (varname == "default_direction") {
        ifile >> default_direction;
      } else if (varname == "rotation_enabled") {
        rotation_enabled = true;
      } else if (varname == "minscale") {
        ifile >> minscale;
      } else if (varname == "rcaction") {
        ifile >> rcaction;
      } else if (varname == "lightlevel") {
        ifile >> lightlevel;
      } else if (varname == "item_dropped") {
        droptable = DropTable(ifile);
      } else if (varname == "clumpyness") {
        ifile >> clumpyness;
      } else if (varname == "clumpy_group") {
        string str_group;
        ifile >> str_group;
        clumpy_group = std::hash<string>()(str_group);
        if (clumpy_group == 0) {
          clumpy_group ++;
        }
      } else if (varname == "transparent") {
        transparent = true;
      }
      getline(ifile, buff, ':');
      getline(ifile, varname, ':');
    }
    if (tex_str != "") {
      stringstream ss(tex_str);
      vector<string> files;
      if (transparent) {
        get_files_folder("resources/textures/blocks/transparent/" + std::to_string(minscale), &files);
      } else {
        get_files_folder("resources/textures/blocks/" + std::to_string(minscale), &files);
      }
      for (int i = 0; i < 6; i ++) {
        string filename;
        ss >> filename;
        if ((ss.eof() and filename == "") and i > 0) {
          texture[i] = texture[i-1];
        } else {
          filename += transparent ? ".png" : ".bmp";
          texture[i] = 0;
          for (int j = 0; j < files.size(); j ++) {
            if (files[j] == filename) {
              texture[i] = j;
            }
          }
        }
      }
    } else {
      cout << "ERR: textures is a requred block data field" << endl;
    }
  }
}

BlockData::~BlockData() {
  if (extras != nullptr) {
    delete extras;
  }
}

void BlockData::do_rcaction(Pixel* pix) {
    if (rcaction == "crafting") {
      if (menu == nullptr) {
        
      }
    } else if (rcaction == "chest") {
      if (menu == nullptr) {
        
      }
    }
}



BlockStorage::BlockStorage() {
    vector<string> block_paths;
    get_files_folder("resources/data/blocks", &block_paths);
    num_blocks = block_paths.size();
    int i = 0;
    for (string filename : block_paths) {
        i ++;
        ifstream ifile("resources/data/blocks/" + filename);
        BlockData* data = new BlockData(ifile);
        blocks[(char)i] = data;
        names[data->name] = (char)i;
        //cout << "loading blockdata " << data->name << " from file" << endl;
    }
    cout << "loaded " << i << " blockdata files" << endl;
}

BlockStorage::~BlockStorage() {
  for (pair<char,BlockData*> kv : blocks) {
    delete kv.second;
  }
}

bool BlockStorage::clumpy(char first, char second) {
  if (first != 0 and second != 0) {
    if (first == second) {
      return true;
    }
    unsigned int fgroup = blocks[first]->clumpy_group;
    unsigned int sgroup = blocks[second]->clumpy_group;
    if (fgroup == 0 or sgroup == 0) {
      return false;
    }
    return fgroup == sgroup;
  }
  return false;
}


#endif
