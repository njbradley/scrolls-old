#ifndef ITEMS_PREDEF
#define ITEMS_PREDEF
#include <iostream>
using std::cout;
using std::endl;
#include <functional>
using std::function;
#include <map>
using std::map;
#include "classes.h"


class CharArray { public:
    char* arr;
    int sx;
    int sy;
    int sz;
    CharArray( char* newarr, int x, int y, int z);
    CharArray(ifstream & ifile);
    ~CharArray();
    char get(int x, int y, int z);
    void set(char val, int x, int y, int z);
    void place(World* world, int x, int y, int z, int dx, int dy, int dz);
};

class Item {
public:
  ItemData* data;
  double durability;
  double sharpness;
  double weight;
  bool isnull;
  
  Item(ItemData* newdata);
  Item(istream& ifile);
  Item(ItemData* newdata, double newsharpness, double newweight);
  void damage(double time);
  double dig_time(char val);
  bool do_rcaction(World* world);
  char ondig(World* world, int x, int y, int z);
  void to_file(ostream& ofile);
  string descript();
  
  static char ondig_null(World* world, int x, int y, int z);
};
  

class ItemData {
    public:
        int texture;
        bool stackable;
        string name;
        CharArray* onplace;
        string rcaction;
        int damage;
        int starting_weight;
        int starting_sharpness;
        string tool;
            
        ItemData(ifstream & ifile);
        ~ItemData();
};

class ItemStack {
public:
  Item item;
  int count;
  ItemStack(Item,int);
  void render(MemVecs* vecs, float x, float y);
};

class ItemStorage { public:
    map<string,ItemData*> items;
    ItemStorage();
    ~ItemStorage();
};

ItemStorage* itemstorage;



class ItemContainer {
    public:
        vector<ItemStack> items;
        int size;
        
        ItemContainer(int newsize);
        ItemContainer(istream&);
        ItemContainer(ItemContainer*, ItemContainer*);
        bool add(ItemStack itemstack);
        Item* get(int index);
        Item* use(int index);
        bool contains(ItemStack);
        bool take(ItemStack);
        void render(MemVecs* vecs, float x, float y);
        void save_to_file(ostream&);
        void clear();
};

#endif
