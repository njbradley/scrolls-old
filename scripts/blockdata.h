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
}

void BlockData::do_rcaction() {
    if (rcaction == "crafting") {
        
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
