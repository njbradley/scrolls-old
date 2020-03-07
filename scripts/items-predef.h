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
    CharArray(ifstream & ifile) ;
    char get(int x, int y, int z);
    void set(char val, int x, int y, int z);
    void place(World* world, int x, int y, int z, int dx, int dy, int dz);
};

class Item {
    public:
        int texture;
        string name;
        CharArray* onplace;
        int damage;
        string tool;
            
        Item(ifstream & ifile);
        char ondig(World* world, int x, int y, int z);
        void dig_area(World* world, int x, int y, int z, double random);
        void dig_pickaxe(World* world, int x, int y, int z, int time, double random);
};

class ItemStorage { public:
    map<string,Item*> items;
    ItemStorage();
};

ItemStorage* itemstorage;



class ItemContainer {
    public:
        vector<pair<Item*,int> > items;
        int size;
        
        ItemContainer(int newsize);
        ItemContainer(istream*);
        bool add(Item* item, int num);
        Item* get(int index);
        Item* use(int index);
        void render(RenderVecs* vecs, float x, float y);
        void save_to_file(ostream*);
};

#endif
