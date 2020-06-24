#ifndef BLOCKDATA
#define BLOCKDATA
#include <iostream>
using std::cout;
using std::endl;
using std::string;
#include <fstream>
using std::ifstream;
#include <map>
using std::map;
#include "blockdata-predef.h"
#include "menu-predef.h"
#include "blocks-predef.h"
#include "cross-platform.h"



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
    inven = new ItemContainer(blocks->blocks[pix->value]->extras->inven->size);
  //}
}

BlockExtra::~BlockExtra() {
  delete inven;
}

BlockExtra::BlockExtra() {}

BlockExtra::BlockExtra(istream& ifile): pixel(nullptr) {
  inven = new ItemContainer(ifile);
  string buff;
  getline(ifile, buff, ':');
}

void BlockExtra::save_to_file(ostream& ofile) {
  inven->save_to_file(ofile);
  ofile << ':';
}



BlockData::BlockData(ifstream & ifile):
default_direction(0), rotation_enabled(false), minscale(1), rcaction("null"), lightlevel(0),
clumpyness(0.9), item("null") {
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
      if (varname == "break_times") {
        getline(ifile, buff, '[');
        string all;
        getline(ifile, all, ']');
        stringstream ss(all);
        string word;
        while (ss >> word) {
          stringstream wordss(word);
          string tool;
          getline(wordss, tool, ':');
          wordss >> hardness[tool];
        }
      } else if (varname == "textures") {
        getline(ifile, buff, '[');
        getline(ifile, tex_str, ']');
      } else if (varname == "default_direction") {
        ifile >> default_direction;
      } else if (varname == "roation_enabled") {
        ifile >> rotation_enabled;
      } else if (varname == "minscale") {
        ifile >> minscale;
      } else if (varname == "rcaction") {
        ifile >> rcaction;
      } else if (varname == "lightlevel") {
        ifile >> lightlevel;
      } else if (varname == "item_dropped") {
        ifile >> item;
      } else if (varname == "clumpyness") {
        ifile >> clumpyness;
      } else if (varname == "clumpy_group") {
        ifile >> clumpy_group;
      }
      getline(ifile, buff, ':');
      getline(ifile, varname, ':');
    }
    
    if (clumpy_group == "") {
      clumpy_group == name + "-group";
    }
    if (tex_str != "") {
      stringstream ss(tex_str);
      vector<string> files;
      get_files_folder("resources/textures/blocks/" + std::to_string(minscale), &files);
      for (int i = 0; i < 6; i ++) {
        string filename;
        ss >> filename;
        if ((ss.eof() and filename == "") and i > 0) {
          texture[i] = texture[i-1];
        } else {
          filename += ".bmp";
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
        cout << "setting menu" << endl;
        menu = new CraftingMenu( 6, [&] () {
          cout << "done! " << endl;
          delete menu;
          menu = nullptr;
        });
      }
    } else if (rcaction == "chest") {
      if (menu == nullptr) {
        cout << "setting menu" << endl;
        menu = new InventoryMenu("chest", pix->extras->inven, [&] () {
  				cout << "done! " << endl;
  				delete menu;
  				menu = nullptr;
  			});
      }
    }
}



BlockStorage::BlockStorage(string path) {
    vector<string> block_paths;
    get_files_folder(path + "resources/data/blocks", &block_paths);
    num_blocks = block_paths.size();
    int i = 0;
    for (string filename : block_paths) {
        i ++;
        ifstream ifile(path + "resources/data/blocks/" + filename);
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


#endif
