#include "blocks.h"
#include <iostream>
#include <time.h>

using namespace std;

void iterative(int x, int y, int z, Pixel* pix) {
    if (rand()%2 == 0 and pix->scale > 2) {
        pix->subdivide(iterative);
    } else {
        pix->value = (rand()%2)*(rand()%5);
    }
}

void init() {
    
    int seed = 1577333634;//time(NULL);//1577057246;
    /*
    srand(seed);
    cout << "seed: " << seed << endl;
    
    c = new Chunk;
    c->scale = 8;
    c->px = 0;
    c->py = 0;
    c->pz = 0;
    c->parent = NULL;
    int * index = new int(0);
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                Pixel * pix = new Pixel;
                pix->scale = 4;
                pix->px = x;
                pix->py = y;
                pix->pz = z;
                pix->parent = c;
                pix->set(0);
                c->blocks[x][y][z] = pix;
                if (!false){//(x==0 and y==0 and z==0)) {
                    
                    pix->subdivide([=] (int x, int y, int z, Pixel* pix) {
                        if (rand()%2 == 0) {
                            pix->subdivide([=] (int x, int y, int z, Pixel* pix) {
                                pix->value = rand()%5;
                            });
                        } else {
                            pix->value = rand()%2;
                        }
                    });
                }
                
            }
        }
    }
    //cout << c << endl;
    int lx, ly, lz;
    //c->world_to_local(11,9,2,&lx,&ly,&lz);
    //cout << lx << ' ' << ly << ' ' << lz << ' ' << c->scale << endl;
    //exit(5);
    //c->get(0,1,0)->set('h');
    Block * b = c->blocks[0][0][0];
    
    //b->subdivide([] (int x, int y, int z) { return 0; });
    b = c->get(0,0,0);
    
    Pixel* p = new Pixel(0, 0, 0, 128, NULL);
    c = p->subdivide(iterative);
    cout << (int)c->get_global(0,0,0,1)->get() << endl;
    cout << "competed without errors" << endl;
    */
    //world = new World("scrolls/save.world");
    world = new World("terrain");//, seed);
    //world->load_chunk(make_pair(0,0));
    //world->save_chunk(make_pair(0,0));
    
}

void render_terrain() {
    
    world->render();
    
}