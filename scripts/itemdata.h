#include <iostream>
using std::cout;
using std::endl;
using std::string;
#include <fstream>
using std::ifstream;
using std::ofstream;
#include <sstream>
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
        
        
        void ondig(World* world, int x, int y, int z) {
            char type = world->get(x,y,z);
            dig_pickaxe(world, x, y, z, damage, type);
        }
        
        void dig_pickaxe(World* world, int x, int y, int z, int time, char type) {
            time --;
            if (time < 0 or world->get(x,y,z) != type) {
                return;
            }
            world->set(0,x,y,z);
            int prob = 100-20*time;
            if (rand()%100 > prob) {
                dig_pickaxe(world, x+1, y, z, time, type);
            } if (rand()%100 > prob) {
                dig_pickaxe(world, x-1, y, z, time, type);
            } if (rand()%100 > prob) {
                dig_pickaxe(world, x, y+1, z, time, type);
            } if (rand()%100 > prob) {
                dig_pickaxe(world, x, y-1, z, time, type);
            } if (rand()%100 > prob) {
                dig_pickaxe(world, x, y, z+1, time, type);
            } if (rand()%100 > prob) {
                dig_pickaxe(world, x, y, z-1, time, type);
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
    
    