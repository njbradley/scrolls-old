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
        return arr[x*sy*sz, y*sy, z];
    }
    
    void set(char val, int x, int y, int z) {
        arr[x*sy*sz, y*sy, z] = val;
    }
};

class Item {
    public:
        int id;
        CharArray* onplace;
        CharArray* ondig;
        int textureid;
        
        Item(int nid, CharArray* onpl, CharArray* ond, int tex):
            id(nid), onplace(onpl), ondig(ond), textureid(tex) {}
        
        void onplace_func(World* world, int x, int y, int z) {
            for (int i = 0; i < onplace->sx; i ++) {
                for (int j = 0; j < onplace->sy; j ++) {
                    for (int k = 0; k < onplace->sz; k ++) {
                        world->get_global(x-onplace->sx/2+i, y-onplace->sy/2+j, z-onplace->sz/2+k, 1)->set(onplace->get(i,j,k));
                    }
                }
            }
        }
};