#ifndef BLOCKDATA_PREDEF
#define BLOCKDATA_PREDEF
#include <fstream>
using std::ifstream;
#include <map>
using std::map;
#include "classes.h"
using std::istream;

// class BlockExtra { public:
//   Pixel* pixel;
//   BlockExtra(Pixel*);
//   virtual ItemContainer* get_inven();
//   virtual save_to_file(ostream& ofile) = 0;
//
//   static BlockExtra* from_file(istream& ifile);
//   static BlockExtra* from_name(
// };

class BlockExtra { public:
  ItemContainer* inven;
  Pixel* pixel;
  BlockExtra();
  ~BlockExtra();
  BlockExtra(Pixel*);
  BlockExtra(istream& ifile);
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
    BlockExtra* extras = nullptr;
    
    BlockData(ifstream & ifile);
    ~BlockData();
    
    void do_rcaction(Pixel* pix);
};

class BlockStorage { public:
    map<char,BlockData*> blocks;
    map<string, char> names;
    int num_blocks;
    
    BlockStorage(string path = "");
    ~BlockStorage();
};

BlockStorage* blocks;

#endif
