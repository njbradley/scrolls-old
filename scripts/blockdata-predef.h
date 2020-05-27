#ifndef BLOCKDATA_PREDEF
#define BLOCKDATA_PREDEF
#include <fstream>
using std::ifstream;
#include <map>
using std::map;
#include "classes.h"
using std::istream;

class BlockExtras { public:
  ItemContainer* inven;
  BlockExtras();
  BlockExtras(Pixel*);
  BlockExtras(istream& ifile);
  void save_to_file(ostream& ofile);
};

class BlockData { public:
    map<string,double> hardness;
    int texture[6];
    int default_direction;
    bool rotation_enabled;
    int minscale;
    string name;
    string rcaction;
    float lightlevel;
    string item;
    double clumpyness;
    string clumpy_group;
    BlockExtras* extras = nullptr;
    
    BlockData(ifstream & ifile);
    
    void do_rcaction(Pixel* pix);
};

class BlockStorage { public:
    map<char,BlockData*> blocks;
    map<string, char> names;
    
    BlockStorage();
};

BlockStorage* blocks;

#endif
