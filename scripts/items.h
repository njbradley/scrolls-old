#include <iostream>
using std::cout;
using std::endl;
#include <functional>
using std::function;

class CharArray { public:
    char* arr;
    int sx;
    int sy;
    int sz;
    CharArray( char* newarr, int x, int y, int z): arr(newarr), sx(x), sy(y), sz(z) { }
    
    char get(int x, int y, int z) {
        //cout << x << ' ' << y << ' ' << z << ' ' << x*sy*sz+ y*sz+ z << endl;
        return arr[x*sy*sz+ y*sz+ z];
    }
    
    void set(char val, int x, int y, int z) {
        arr[x*sy*sz, y*sy, z] = val;
    }
    
    void place(World* world, int x, int y, int z, int dx, int dy, int dz) {
        for (int i = 0; i < sx; i ++) {
            for (int j = 0; j < sy; j ++) {
                for (int k = 0; k < sz; k ++) {
                    int px = i;
                    int py = j;
                    int pz = k;
                    world->set(get(i,j,k), x+(px*dx + py*dy + pz*dz), y+(px*dy + py*dz + pz*dx), z+(px*dz + py*dx + pz*dy));
                }
            }
        }
    }
};

class Item {
    public:
        int id;
        CharArray* onplace;
        int textureid;
        
        Item(int nid, CharArray* onpl, int tex):
            id(nid), onplace(onpl), textureid(tex) {}
        
        void ondig(World* world, int x, int y, int z) {
            
        }
        
        
};