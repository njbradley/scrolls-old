#ifndef ITEMS_PREDEF
#define ITEMS_PREDEF

#include "classes.h"
#include "plugins.h"

#include <map>
using std::map;

class CharArray { public:
    char* arr;
    int sx;
    int sy;
    int sz;
    CharArray( char* newarr, int x, int y, int z);
    CharArray(ifstream & ifile);
    CharArray(): arr(nullptr) {}
    ~CharArray();
    char get(int x, int y, int z);
    void set(char val, int x, int y, int z);
    bool place(World* world, Item* item, ivec3 pos, ivec3 dir);
};

class Item {
public:
  ItemData* data;
  double sharpness = 0;
  double reach = 1;
  double weight = 1;
  double area;
  double durability;
  bool isnull;
  bool modified;
  bool stackable = false;
  vector<Item> addons;
  
  Item(ItemData* newdata);
  Item(istream& ifile);
  Item(ItemData* newdata, double newsharpness, double newweight, double newreach);
  double get_sharpness();
  double get_weight(double reach = 0);
  double get_reach();
  string get_name();
  pair<Item*,double> get_head(double depth = 0);
  double get_recharge_time();
  //void damage(Player* player, BlockData* data, double time);
  //double dig_time(char val);
  double collision(Pixel* pix);
  char ondig(World* world, int x, int y, int z);
  void trim();
  void to_file(ostream& ofile);
  string descript();
  void sharpen(double speed, double force);
  void render(UIVecs* vecs, vec2 pos, float scale = 0.1f);
  UIRect getrect();
  
  static char ondig_null(World* world, int x, int y, int z);
};
  

struct ItemDataParams {
  string name;
  int texture = 0;
  bool stackable = true;
  CharArray onplace;
  Blocktype block = 0;
  double toughness = 1;
  double starting_weight = 1;
  double starting_sharpness = 1;
  double starting_reach = 1;
  bool sharpenable = false;
  Material* material;
  int lightlevel = 0;
  bool isgroup = false;
};

class ItemData : public ItemDataParams { public:
  PLUGIN_HEAD(ItemData, ());
  
  ItemData(ItemDataParams&& params);
  void init();
};

class ItemStorage : public Storage<ItemData> { public:
  void init();
};

extern ItemStorage itemstorage;

class ItemStack {
public:
  Item item;
  int count;
  vector<Item> unique;
  ItemStack(Item,int);
  ItemStack(istream& ifile);
  void to_file(ostream& ofile);
  void render(UIVecs* vecs, vec2 pos);
  bool add(Item newitem);
  bool add(ItemStack otherstack);
  Item take();
  ItemStack take(int num);
  ItemStack half();
  Item* get_unique();
  bool isnull();
  void trim();
};




class ItemContainer { public:
  vector<ItemStack> items;
  int size;
  
  ItemContainer(int newsize);
  ItemContainer(istream&);
  ItemContainer(ItemContainer*, ItemContainer*);
  bool add(ItemStack itemstack);
  Item* get(int index);
  Item use(int index);
  bool contains(ItemStack);
  bool take(ItemStack);
  void make_single(int index);
  void render(UIVecs* vecs, vec2 pos);
  void save_to_file(ostream&);
  void clear();
};


namespace items {
  extern ItemData dirt;
  extern ItemData wood;
  extern ItemData leaves;
};



#endif
