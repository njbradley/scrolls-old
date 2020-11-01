#ifndef ITEMS_PREDEF
#define ITEMS_PREDEF

#include "classes.h"

#include <map>
using std::map;

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
  bool do_rcaction(World* world);
  char ondig(World* world, int x, int y, int z);
  void trim();
  void to_file(ostream& ofile);
  string descript();
  void sharpen(double speed, double force);
  void render(MemVecs* vecs, float x, float y, float scale = 0.1f);
  
  static char ondig_null(World* world, int x, int y, int z);
};
  

class ItemData {
    public:
        int texture;
        bool stackable;
        string name;
        CharArray* onplace;
        string rcaction;
        double toughness;
        double starting_weight;
        double starting_sharpness;
        double starting_reach;
        bool sharpenable;
        Material* material;
            
        ItemData(ifstream & ifile);
        ~ItemData();
};

class ItemStack {
public:
  Item item;
  int count;
  vector<Item> unique;
  ItemStack(Item,int);
  ItemStack(istream& ifile);
  void to_file(ostream& ofile);
  void render(MemVecs* vecs, float x, float y);
  bool add(Item newitem);
  bool add(ItemStack otherstack);
  Item take();
  ItemStack take(int num);
  ItemStack half();
  Item* get_unique();
  bool isnull();
  void trim();
};

class ItemStorage { public:
    map<string,ItemData*> items;
    int total_images;
    ItemStorage();
    ~ItemStorage();
};

extern ItemStorage* itemstorage;



class ItemContainer {
    public:
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
        void render(MemVecs* vecs, float x, float y);
        void save_to_file(ostream&);
        void clear();
};

#endif
