#ifndef BLOCKDATA_PREDEF
#define BLOCKDATA_PREDEF

#include "classes.h"

#include <map>
using std::map;


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

class DropTable { public:
  double prob;
  double sharpness;
  double force;
  vector<DropTable> tables;
  string item_name;
  int count;
  DropTable(istream& ifile);
  DropTable();
  void drop(ItemContainer* result, Player* player, Item* break_item);
};

class BlockData { public:
    Material* material;
    int texture[6];
    int default_direction;
    bool rotation_enabled;
    int minscale;
    string name;
    string rcaction;
    int lightlevel;
    DropTable droptable;
    double clumpyness;
    unsigned int clumpy_group;
    bool transparent;
    BlockExtra* extras = nullptr;
    
    BlockData(ifstream & ifile);
    ~BlockData();
    
    void do_rcaction(Pixel* pix);
};

class BlockStorage { public:
    map<char,BlockData*> blocks;
    map<string, char> names;
    int num_blocks;
    
    BlockStorage();
    ~BlockStorage();
    bool clumpy(char first, char second);
};

extern BlockStorage* blocks;

#endif
