#ifndef BLOCKDATA_PREDEF
#define BLOCKDATA_PREDEF

#include "classes.h"

#include <map>
using std::map;




class BlockData { public:
    Material* material;
    int texture[6];
    int default_direction;
    bool rotation_enabled;
    string name;
    int lightlevel;
    bool transparent;
    
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
