#ifndef ITEMDATA
#define ITEMDATA

#include <iostream>
using std::cout;
using std::endl;
using std::string;
#include <fstream>
using std::ifstream;
using std::ofstream;
#include <sstream>
#include "blocks.h"
using std::stringstream;
#include "win.h"

class CharArray { public:
    char* arr;
    int sx;
    int sy;
    int sz;
    CharArray( char* newarr, int x, int y, int z): arr(newarr), sx(x), sy(y), sz(z) { }
    
    CharArray(ifstream & ifile) {
        string buff;
        getline(ifile, buff, ':');
        ifile >> sx >> sy >> sz;
        getline(ifile, buff, ':');
        arr = new char[sx*sy*sz];
        for (int i = 0; i < sx*sy*sz; i ++) {
            int j;
            ifile >> j;
            arr[i] = (char)j;
        }
    }
    
    char get(int x, int y, int z) {
        //cout << x << ' ' << y << ' ' << z << ' ' << x*sy*sz+ y*sz+ z << endl;
        return arr[x*sy*sz+ y*sz+ z];
    }
    
    void set(char val, int x, int y, int z) {
        arr[x*sy*sz + y*sz + z] = val;
    }
    
    void place(World* world, int x, int y, int z, int dx, int dy, int dz) {
        for (int i = 0; i < sx; i ++) {
            for (int j = 0; j < sy; j ++) {
                for (int k = 0; k < sz; k ++) {
                    int px = i;
                    int py = j - (sy/2);
                    int pz = k - (sz/2);
                    world->set(get(i,j,k), x+(px*dx + py*dy + pz*dz), y+(px*dy + py*dz + pz*dx), z+(px*dz + py*dx + pz*dy));
                }
            }
        }
    }
};

class Item {
    public:
        int texture;
        string name;
        CharArray* onplace;
        int damage;
        string tool;
            
        Item(ifstream & ifile) {
            int isarray;
            ifile >> isarray;
            if (isarray) {
                onplace = new CharArray(ifile);
            } else {
                onplace = nullptr;
            }
            string buff;
            getline(ifile, buff, ':');
            ifile >> name >> texture >> tool >> damage;
        }
        
        
        char ondig(World* world, int x, int y, int z) {
            char type = world->get(x,y,z);
            world->set(0,x,y,z);
            return type;
            //double random = (rand()%1000/1000.0);
            //double prob = blocks->blocks[type]->hardness[tool];// - ((damage-1)-time)*0.2;
            //cout << prob << ' ' << random << endl;
            //if (prob > random) {
            //    world->set(0,x,y,z);
            //}
            //dig_pickaxe(world, x, y, z, random);
        }
        
        void dig_area(World* world, int x, int y, int z, double random) {
            int size = damage/2;
            for (int ox = -size; ox <= size; ox ++) {
                for (int oy = -size; oy <= size; oy ++) {
                    for (int oz = -size; oz <= size; oz ++) {
                        char val = world->get(x+ox, y+oy, z+oz);
                        double distance = sqrt(ox*ox+oy*oy+oz*oz);
                        if (val != 0) {
                            double prob = blocks->blocks[val]->hardness[tool] - distance*0.2;
                            if (prob > random) {
                                world->set(0,x+ox, y+oy, z+oz);
                            }
                        }
                    }
                }
            }
        }
            
        
        void dig_pickaxe(World* world, int x, int y, int z, int time, double random) {
            time --;
            char val = world->get(x,y,z);
            if (time < 0 or val == 0) {
                return;
            }
            BlockData* data = blocks->blocks[val];
            double prob = data->hardness[tool] - ((damage-1)-time)*0.2;
            cout << prob << ' ' << random << endl;
            if (prob > random) {
                world->set(0,x,y,z);
                dig_pickaxe(world, x+1, y, z, time, random);
                dig_pickaxe(world, x-1, y, z, time, random);
                dig_pickaxe(world, x, y+1, z, time, random);
                dig_pickaxe(world, x, y-1, z, time, random);
                dig_pickaxe(world, x, y, z+1, time, random);
                dig_pickaxe(world, x, y, z-1, time, random);
            }
        }
};

class ItemStorage { public:
    map<string,Item*> items;
    
    ItemStorage() {
        vector<string> block_paths;
        get_files_folder("resources/data/items", &block_paths);
        for (string filename : block_paths) {
            stringstream name_stream(filename);
            string name;
            getline(name_stream, name, '.');
            ifstream ifile("resources/data/items/" + filename);
            cout << "loading item " << name << " from file" << endl;
            items[name] = new Item(ifile);
        }
    }
};

ItemStorage* items;
    
#endif
