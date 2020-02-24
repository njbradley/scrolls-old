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


BlockExtras::BlockExtras(istream & inp) {
  
}

BlockExtras::BlockExtras(const BlockExtras & other) {
  inven = new ItemContainer(other.inven->size);
}

BlockExtras::BlockExtras() {}


BlockData::BlockData(ifstream & ifile) {
    string buff;
    getline(ifile, buff, ':');
    vector<string> tools;
    getline(ifile, buff, ':');
    stringstream line(buff);
    string tool;
    while (line >> tool) {
        tools.push_back(tool);
    }
    for (string tool : tools) {
        ifile >> hardness[tool];
    }
    getline(ifile, buff, ':');
    ifile >> name >> texture >> minscale >> rcaction >> lightlevel >> item;
    bool is_extra;
    ifile >> is_extra;
    if (is_extra) {
      getline(ifile, buff, ':');
      cout << "name: " << name << endl;
      extras = new BlockExtras();
      int size_inven;
      ifile >> size_inven;
      if (size_inven != 0) {
        extras->inven = new ItemContainer(size_inven);
      }
    }
}

void BlockData::do_rcaction(Pixel* pix) {
    if (rcaction == "crafting") {
        
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



BlockStorage::BlockStorage() {
    vector<string> block_paths;
    get_files_folder("resources/data/blocks", &block_paths);
    int i = 0;
    for (string filename : block_paths) {
        i ++;
        ifstream ifile("resources/data/blocks/" + filename);
        BlockData* data = new BlockData(ifile);
        blocks[(char)i] = data;
        names_to_chars[data->name] = (char)i;
        cout << "loading blockdata " << data->name << " from file" << endl;
    }
}


#endif
