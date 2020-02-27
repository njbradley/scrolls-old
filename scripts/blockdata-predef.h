#ifndef BLOCKDATA_PREDEF
#define BLOCKDATA_PREDEF
#include <fstream>
using std::ifstream;
#include <map>
using std::map;
#include "classes.h"

class BlockExtras { public:
  ItemContainer* inven;
  BlockExtras();
  BlockExtras(Pixel*);
};

class BlockData { public:
    map<string,double> hardness;
    int texture;
    int minscale;
    string name;
    string rcaction;
    float lightlevel;
    string item;
    BlockExtras* extras = nullptr;
    
    BlockData(ifstream & ifile);
    
    void do_rcaction(Pixel* pix);
};

class BlockStorage { public:
    map<char,BlockData*> blocks;
    map<string, char> names_to_chars;
    
    BlockStorage();
};

BlockStorage* blocks;

#endif
