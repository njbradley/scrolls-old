#ifndef BLOCKDATA
#define BLOCKDATA
#include <iostream>
using std::cout;
using std::endl;
using std::string;
#include <fstream>
using std::ifstream;

class BlockData { public:
    map<string,double> hardness;
    int texture;
    string name;
    
    BlockData(ifstream ifile) {
        
    }
};