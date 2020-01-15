#ifndef BLOCKS
#define BLOCKS

#include <iostream>
#include <functional>
#include <cmath>
#include <fstream>
#include <windows.h>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
using namespace glm;

#include "generative.h"

#define csize 2

/*
 *this is the file with classes for storing blocks. the main idea is blocks are stored in power of two containers. Block is the base virtual class,
 *and Pixel and Chunk derive from Block. 
 *
 */

const float M_PI = 3.1415f;

using namespace std;

class Block; class Pixel; class Chunk; class World;

struct RenderVecs {
    vector<GLfloat> verts;
	vector<GLfloat> uvs;
	vector<GLfloat> light;
	vector<GLint> mats;
    int num_verts = 0;
};

bool render_flag;

float fastSin( float x ){
    x = fmod(x + M_PI, M_PI * 2) - M_PI; // restrict x so that -M_PI < x < M_PI
    const float B = 4.0f/M_PI;
    const float C = -4.0f/(M_PI*M_PI);

    float y = B * x + C * x * std::abs(x);

    const float P = 0.225f;

    return P * (y * std::abs(y) - y) + y; 
}

class Block { public:
    
    int px; // parent coordinates, bool because only need to go up to 1
    int py;
    int pz;
    int scale; //scale of the block, number of units on one side length. 
    Chunk * parent; //the parent of this block. if this block is the root block, then the parent should be set to nullptr.
    bool continues; // whether the block is a chunk or a pixel. a hacky way of telling, but nessicary because pixels can be at different depths
    virtual Block * get(int, int, int) = 0; //get a block at coords
    virtual char get() = 0;                 //get the value of the block. these two methods are exclusive for pixel/chunk, but same reaseon as above
    virtual void set(char) = 0;             
    virtual Chunk* subdivide(function<void(int,int,int,Pixel*)>) = 0; //same as get,
    
    Block(int x, int y, int z, int newscale, Chunk* newparent) :
        px(x), py(y), pz(z), scale(newscale), parent(newparent) {}
    
    void world_to_local(int x, int y, int z, int* lx, int* ly, int* lz) {
        *lx = x-(x/scale*scale);
        *ly = y-(y/scale*scale);
        *lz = z-(z/scale*scale);
        //cout << scale << " -----scale ------" << endl;
    }
    void global_position(int*, int*, int*);
    
    virtual void render(RenderVecs*) = 0;
        
    virtual bool is_air(int, int, int) = 0;
    
    virtual Block * get_global(int,int,int,int) = 0;
    
    virtual void save_to_file(ofstream*) = 0;
    
    Block* raycast(double* x, double* y, double* z, double dx, double dy, double dz, double time);
    
    Block* get_world();
    
    void update_chunk();
}; 

class Pixel: public Block { public:
    char value;
    bool continues = false;
    
    Pixel(int x, int y, int z, int nscale, Chunk* nparent):
        Block(x, y, z, nscale, nparent) {}
    
    char get() {
        //cout << "pixel get px" << px << " py" << py << " pz" << pz << " s" << scale << " value" << value  << endl;
        return value;
    }
    
    Block * get(int x, int y, int z) {
        cout << "error: get(int,int,int) called on pixel" << endl;
        return nullptr;
    }
    
    void set(char val);
    
    bool is_air(int dx, int dy, int dz) {
        //cout << "pix is air reaturning:" << (value == 0) << endl;
        return value == 0;
    }
    
    Block* get_global(int x, int y, int z, int scale) {
        //cout << "get global at pixel retunring self\n\n";
        return this;
    }
    
    void render(RenderVecs* vecs);
    
    Chunk* subdivide(function<void(int,int,int,Pixel*)> func);
    
    void save_to_file(ofstream* of) {
        *of << value;
    }
};

class Chunk: public Block { public:
    Block * blocks[csize][csize][csize];
    bool continues = true;
    
    Chunk(int x, int y, int z, int nscale, Chunk* nparent):
        Block(x, y, z, nscale, nparent) {}
    
    
    Block * get(int x, int y, int z) {
        //cout << "chunk get px" << px << " py" << py << " pz" << pz << " s" << scale << " x" << x << " y" << y << " z" << z << endl;
        return blocks[x][y][z];
    }
    char get() {
        cout << "error: get() called on chunk object" << endl;
    }
    void set(char c) {
        cout << "error: set called on chunk object" << endl;
    }
    Chunk* subdivide(function<void(int,int,int,Pixel*)> func) {
        cout << "error: subdividing already subdivided block" << endl;
        return nullptr;
    }
    
    void render(RenderVecs* vecs) {
        for (int x = 0; x < csize; x ++) {
            for (int y = 0; y < csize; y ++) {
                for (int z = 0; z < csize; z ++) {
                    blocks[x][y][z]->render(vecs);
                }
            }
        }
    }
    
    Block* get_global(int x, int y, int z, int nscale) {
        //cout << "blocks get_global " << x << ' ' << y << ' ' << z << " scale:" << nscale << endl;
        if (nscale == scale) {
            //cout << " at scale return this\n\n";
            return this;
        } else {
            if (x < 0 or x >= scale or y < 0 or y >= scale or z < 0 or z >= scale) {
                //cout << "ret null ---s -- - - -- -" << endl;
                //return nullptr;
            }
            int lx, ly, lz;
            //cout << 206 << endl;
            world_to_local(x, y, z, &lx, &ly, &lz);
            //cout << lx << ' ' << ly << ' ' << lz << endl;
            if (lx < 0 or lx >= csize*scale or ly < 0 or ly >= csize*scale or lz < 0 or lz >= csize*scale) {
                //cout << "returning null -------------------------------\n\n";
                return nullptr;
            }
            int nlx = lx/(scale/csize);
            int nly = ly/(scale/csize);
            int nlz = lz/(scale/csize);
            //cout << nlx << ' ' << nly << ' ' << nlz << endl;
            //cout << "sdfkhaskdfjhaksldfhalkjsdfhalksfh 229 ksjdfl;akjfd;alksdjf;alksdfja;sd" << endl;
            return get(nlx, nly, nlz)->get_global(lx, ly, lz, nscale);
        }
    }
                
    
    bool is_air(int dx, int dy, int dz) {
        //cout << "is_air(dx" << dx << " dy" << dy << " dz" << dz << ")\n";
        int gx, gy, gz;
        global_position(&gx, &gy, &gz);
        //cout << gx << ' ' << gy << ' ' << gz << endl;
        bool air = false;
        int x_start = 0;
        int x_end = csize;
        int y_start = 0;
        int y_end = csize;
        int z_start = 0;
        int z_end = csize;
        if (dx != 0) {
            x_start = (int)( dx/2.0f + 0.5f );
            x_end = 1 + x_start;
        }
        if (dy != 0) {
            y_start = (int)( dy/2.0f + 0.5f );
            y_end = 1 + y_start;
        }
        if (dz != 0) {
            z_start = (int)( dz/2.0f + 0.5f );
            z_end = 1 + z_start;
        }
        //cout << "for loop:" << endl;
        for (int x = x_start; x < x_end; x ++) {
            for (int y = y_start; y < y_end; y ++) {
                for (int z = z_start; z < z_end; z ++) {
                    //cout << x << ' ' << y << ' ' << z << ' ' << get(x, y, z)->is_air(dx, dy, dz) << endl;
                    air = air or get(x, y, z)->is_air(dx, dy, dz);
                }
            }
        }
        //cout << "returning:" << air << endl << endl;
        return air;
    }
    
    void save_to_file(ofstream* of) {
        *of << "{";
        for (int x = 0; x < csize; x ++) {
            for (int y = 0; y < csize; y ++) {
                for (int z = 0; z < csize; z ++) {
                    blocks[x][y][z]->save_to_file(of);
                }
            }
        }
        *of << "}";
    }
};

class World {
    map<pair<int,int>, Block*> chunks;
    map<pair<int,int>, RenderVecs*> chunk_vecs;
    public:
        int seed;
        string name;
        static const int chunksize = 64;
        function<void(int,int,int,Pixel*)> iter_gen_func;
        
        World(string newname, int newseed): seed(newseed), name(newname) {
            setup_files();
            startup();
        }
        
        World(string oldname): name(oldname) {
            load_data_file();
            startup();
        }
        
        void load_data_file() {
            string path = "saves/" + name + "/worlddata.txt";
            ifstream ifile(path);
            string buff;
            getline(ifile, buff);
            while ( !ifile.eof() ) {
                getline(ifile,buff,':');
                if (buff == "seed") {
                    ifile >> seed;
                }
                getline(ifile, buff);
            }
        }
        
        void save_data_file() {
            string path = "saves/" + name + "/worlddata.txt";
            ofstream ofile(path);
            ofile << "Scrolls data file of world '" + name + "'\n";
            ofile << "seed:" << seed << endl;
            ofile.close();
        }
            
        
        void startup() {
            iter_gen_func = [&] (int x, int y, int z, Pixel* p) {
                iter_gen(x, y, z, p);
            };
            for (int x = -1; x < 1; x ++) {
                for (int y = -1; y < 1; y ++) {
                    load_chunk(pair<int,int>(x,y));
                }
            }       
        }
        
        char gen_func(int x, int y, int z) {
            return generative_function(x, y, z);
        }
        
        void iter_gen(int gx, int gy, int gz, Pixel* pix) {
            char value = gen_func(gx, gy, gz);
            bool cont = true;
            for (int x = gx; x < gx+pix->scale and cont; x ++) {
                for (int y = gy; y < gy+pix->scale and cont; y ++) {
                    for (int z = gz; z < gz+pix->scale and cont; z ++) {
                        char newval = gen_func(x,y,z);
                        if (newval != value) {
                            cont = false;
                        }
                        value = newval;
                    }
                }
            }
            if (cont) {
                pix->value = value;
            } else {
                pix->subdivide(iter_gen_func);
                delete pix;
            }
        }
        
        void generate(pair<int,int> pos) {
            Pixel* pix = new Pixel(pos.first,0,pos.second,chunksize,nullptr);
            chunks[pos] = pix->subdivide(iter_gen_func);
            delete pix;
        }
        
        void render(RenderVecs* vecs) {
            for (pair<pair<int,int>, Block*> kvpair : chunks) {
                //chunks[kvpair.first]->render(vecs);
                render_chunk_vectors(kvpair.first);
            }
            for (pair<pair<int,int>, RenderVecs*>  kvpair : chunk_vecs) {
                
                vecs->verts.insert(vecs->verts.end(), kvpair.second->verts.begin(), kvpair.second->verts.end() );
                vecs->mats.insert(vecs->mats.end(), kvpair.second->mats.begin(), kvpair.second->mats.end() );
                vecs->light.insert(vecs->light.end(), kvpair.second->light.begin(), kvpair.second->light.end() );
                vecs->uvs.insert(vecs->uvs.end(), kvpair.second->uvs.begin(), kvpair.second->uvs.end() );
                vecs->num_verts += kvpair.second->num_verts;
                //cout << kvpair.second->pz << endl;
            }
            //exit(1);
        }
        
        void render_chunk_vectors(pair<int,int> pos) {
            
            //cout << (chunk_vecs.find(pos) == chunk_vecs.end()) << endl;
            if (chunk_vecs.find(pos) == chunk_vecs.end()) {
                //cout << "rendering " << pos.first << ' ' << pos.second << endl;
                RenderVecs* rv = new RenderVecs;
                chunks[pos]->render(rv);
                chunk_vecs[pos] = rv;
                //cout << rv->num_verts << endl;
            }
        }
        
        void mark_render_update(pair<int,int> pos) {
            delete chunk_vecs[pos];
            chunk_vecs.erase(pos);
            render_flag = true;
        }
        
        Block* get_global(int x, int y, int z, int scale) {
            //cout << "world::get global(" << x << ' ' << y << ' ' << z << ' ' << scale << endl;
            int px = x/chunksize;
            int pz = z/chunksize;
            if (x < 0 and -x%chunksize != 0) {
                px --;
            } if (z < 0 and -z%chunksize != 0) {
                pz --;
            }
            pair<int,int> pos = pair<int,int>(px, pz);
            //cout << "pair<" << pos.first << ' ' << pos.second << endl;
            if (chunks.find(pos) == chunks.end() or y > chunksize or y < 0) {
                //cout << "   returning null\n";
                return nullptr;
            }
            //cout << endl;
            int ox = ((x < 0) ? chunksize : 0) + x%chunksize;
            int oz = ((z < 0) ? chunksize : 0) + z%chunksize;
            //cout << ox << ' ' << y << ' ' << oz << endl;
            return chunks[pos]->get_global(ox, y, oz, scale);
        }
        
        Block* raycast(double* x, double* y, double* z, double dx, double dy, double dz, double time) {
            //cout << "world raycast" << *x << ' ' << *y << ' ' << *z << ' ' << dx << ' ' << dy << ' ' << dz << endl;
            Block* b = get_global((int)*x - (*x<0), (int)*y - (*y<0), (int)*z - (*z<0), 1);
            //cout << b << endl;
            return b->raycast(x, y, z, dx, dy, dz, time);
        }
        
        void setup_files() {
            string path = "saves/" + name;
            CreateDirectory(path.c_str(), NULL);
        }
        
        void save_chunk(pair<int,int> pos) {
            stringstream path;
            path << "saves/" << name << "/chunk" << pos.first << "x" << pos.second << "y.dat";
            cout << "saving '" << path.str() << "' to file\n";
            ofstream of(path.str(), ios::binary);
            of << "scrolls chunk file:";
            chunks[pos]->save_to_file(&of);
        }
        
        Block* parse_file(ifstream* ifile, int px, int py, int pz, int scale, Chunk* parent) {
            char c;
            ifile->read(&c,1);
            if (c == '{') {
                Chunk* chunk = new Chunk(px, py, pz, scale, parent);
                for (int x = 0; x < csize; x ++) {
                    for (int y = 0; y < csize; y ++) {
                        for (int z = 0; z < csize; z ++) {
                            chunk->blocks[x][y][z] = parse_file(ifile, x, y, z, scale/csize, chunk);
                        }
                    }
                }
                ifile->read(&c,1);
                if (c != '}') {
                    cout << "error, mismatched {} in save file" << endl;
                }
                return (Block*) chunk;
            } else {
                Pixel* p = new Pixel(px, py, pz, scale, parent);
                p->value = c;
                return (Block*) p;
            }
        }
        
        void load_chunk(pair<int,int> pos) {
            stringstream path;
            path << "saves/" << name << "/chunk" << pos.first << "x" << pos.second << "y.dat";
            ifstream ifile(path.str(), ios::binary);
            if (false and ifile.good()) {
                cout << "loading '" << path.str() << "' from file\n";
                string header;
                getline(ifile, header, ':');
                
                chunks[pos] = parse_file(&ifile, pos.first, 0, pos.second, chunksize, nullptr);
            } else {
                cout << "generating '" << path.str() << "'\n";
                generate(pos);
            }
            //render_chunk_vectors(pos);
        }
        
        void close_world() {
            for (pair<pair<int,int>, Block*> kvpair : chunks) {
                save_chunk(kvpair.first);
            }
            save_data_file();
        }
};

World* world;

















void Block::global_position(int* gx, int* gy, int* gz) {
    //cout << px << ' ' << py << ' ' << pz << ' ' << scale << endl;
    if (parent == nullptr) {
        *gx = px*scale;
        *gy = py*scale;
        *gz = pz*scale;
    } else {
        parent->global_position(gx, gy, gz);
        *gx += px*scale;
        *gy += py*scale;
        *gz += pz*scale;
    }
    //cout << *gx << ' ' << *gy << ' ' << *gz << endl;
}

Block* Block::get_world() {
    if (parent == nullptr) {
        return this;
    } else {
        return parent->get_world();
    }
}

void Block::update_chunk() {
    if (parent == nullptr) {
        world->mark_render_update(pair<int,int>(px,pz));
    } else {
        parent->update_chunk();
    }
}


Block* Block::raycast(double* x, double* y, double* z, double dx, double dy, double dz, double time) {
        //cout << "block raycast(" << *x << ' ' << *y << ' ' << *z << ' ' << dx << ' ' << dy << ' ' << dz << endl;
        if (!continues and get() != 0) {
            //cout << "returning" << continues << get() << endl;
            return this;
        }
        
        
        double lx, ly, lz;
        //world_to_local(*x, *y, *z, &lx, &ly, &lz);
        int gx, gy, gz;
        global_position(&gx, &gy, &gz);
        
        lx = *x - gx;
        ly = *y - gy;
        lz = *z - gz;
        
        //cout << "this " << px <<  ' ' << py << ' ' << pz << ' ' << scale << endl;
        //cout << "local " << lx << ' ' << ly << ' ' << lz << endl;
        //cout << "global " << gx << ' ' << gy << ' ' << gz << endl;
        
        double tx = (dx < 0) ? lx/(-dx) : (scale-lx)/dx;
        double ty = (dy < 0) ? ly/(-dy) : (scale-ly)/dy;
        double tz = (dz < 0) ? lz/(-dz) : (scale-lz)/dz;
        
        //cout << "ts " << tx << ' ' << ty << ' ' << tz << endl;
        double extra = 0.0001;
        
        if (tx < ty and tx < tz) { // hits x wall first
            *x += dx*(tx+extra);
            *y += dy*(tx+extra);
            *z += dz*(tx+extra);
            time -= tx;
        } else if (ty < tx and ty < tz) { // hits y wall first
            *x += dx*(ty+extra);
            *y += dy*(ty+extra);
            *z += dz*(ty+extra);
            time -= ty;
        } else if (tz < tx and tz < ty) { // hits z wall first
            *x += dx*(tz+extra);
            *y += dy*(tz+extra);
            *z += dz*(tz+extra);
            time -= tz;
        }
        //cout << "get(" << (int)*x - (*x<0) << ' ' << (int)*y - (*y<0) << ' ' << (int)*z - (*z<0) << ' ' << (*x < 0) << endl;
        Block* b = world->get_global((int)*x - (*x<0), (int)*y - (*y<0), (int)*z - (*z<0), 1);
        if (b == nullptr or time < 0) {
            return nullptr;
        }
        return b->raycast(x, y, z, dx, dy, dz, time);
    }

void Pixel::set(char val) {
    //cout << "pixel set " << val << endl;
    value = val;
    int gx, gy, gz;
    global_position(&gx, &gy, &gz);
    world->mark_render_update(pair<int,int>(gx/world->chunksize - (gx<0), gz/world->chunksize - (gz<0)));
    world->mark_render_update(pair<int,int>((gx+1)/world->chunksize - (gx+1<0), gz/world->chunksize - (gz<0)));
    world->mark_render_update(pair<int,int>((gx-1)/world->chunksize - (gx-1<0), gz/world->chunksize - (gz<0)));
    world->mark_render_update(pair<int,int>(gx/world->chunksize - (gx<0), (gz+1)/world->chunksize - (gz+1<0)));
    world->mark_render_update(pair<int,int>(gx/world->chunksize - (gx<0), (gz-1)/world->chunksize - (gz-1<0)));
    
}

void Pixel::render(RenderVecs* vecs) {
    if (value == 0) {
        //cout << "air " << endl;
        return;
    }
    int gx, gy, gz;
    global_position(&gx, &gy, &gz);
    GLfloat x = gx;
    GLfloat y = gy;
    GLfloat z = gz;
    //cout << x << ' ' << y << ' ' << z << "rendering call" << endl;
    GLfloat uv_start = 0.0f;// + (1/32.0f*(int)value);
    GLfloat uv_end = uv_start + 1.0f;///32.0f;
    GLfloat new_uvs[] = {
        0.0f, 0.0f,
        1.0f * scale, 0.0f,
        1.0f * scale, 1.0f * scale,
        1.0f * scale, 1.0f * scale,
        0.0f, 1.0f * scale,
        0.0f, 0.0f
    };
    
    //cout << 85 << endl;
    Block* block;
    //cout << 87 << endl;
    char mat = (int)value - 1;
    block = world->get_global(gx, gy, gz-scale, scale);
    //exit(2);
    //if (block != nullptr) {
    //    cout << scale << ' ' << block->scale << " sd-------------------------------------\n";
    //    int ox, oy, oz;
    //    block->global_position(&ox, &oy, &oz);
    //   cout << ox << ' ' << oy << ' ' << oz << ' ' << block->px << ' ' << block->py << ' ' << block->pz << endl;
    //    //exit(3);
    //
    if (block == nullptr or block->is_air(0, 0, 1)) {
        GLfloat face[] = {
            x, y, z,
            x, y + scale, z,
            x + scale, y + scale, z,
            x + scale, y + scale, z,
            x + scale, y, z,
            x, y, z
        };
        vecs->verts.insert(vecs->verts.end(), begin(face), end(face));
        vecs->uvs.insert(vecs->uvs.end(), begin(new_uvs), end(new_uvs));
        for (int i = 0; i < 6; i ++) {
            vecs->light.push_back(0.4f);
            vecs->mats.push_back(mat);
        }
        vecs->num_verts += 6;
    }
    
    block = world->get_global(gx, gy-scale, gz, scale);
    if (block == nullptr or block->is_air(0, 1, 0)) {
        GLfloat face[] = {
            x, y, z,
            x + scale, y, z,
            x + scale, y, z + scale,
            x + scale, y, z + scale,
            x, y, z + scale,
            x, y, z
        };
        vecs->verts.insert(vecs->verts.end(), begin(face), end(face));
        vecs->uvs.insert(vecs->uvs.end(), begin(new_uvs), end(new_uvs));
        for (int i = 0; i < 6; i ++) {
            vecs->light.push_back(0.2f);
            vecs->mats.push_back(mat);
        }
        vecs->num_verts += 6;
    }
    
    block = world->get_global(gx-scale, gy, gz, scale);
    if (block == nullptr or block->is_air(1, 0, 0)) {
        GLfloat face[] = {
            x, y, z,
            x, y, z + scale,
            x, y + scale, z + scale,
            x, y + scale, z + scale,
            x, y + scale, z,
            x, y, z
        };
        vecs->verts.insert(vecs->verts.end(), begin(face), end(face));
        vecs->uvs.insert(vecs->uvs.end(), begin(new_uvs), end(new_uvs));
        for (int i = 0; i < 6; i ++) {
            vecs->light.push_back(0.6f);
            vecs->mats.push_back(mat);
        }
        vecs->num_verts += 6;
    }
    
    block = world->get_global(gx, gy, gz+scale, scale);
    if (block == nullptr or block->is_air(0, 0, -1)) {
        GLfloat face[] = {
            x + scale, y + scale, z + scale,
            x, y + scale, z + scale,
            x, y, z + scale,
            x, y, z + scale,
            x + scale, y, z + scale,
            x + scale, y + scale, z + scale
        };
        vecs->verts.insert(vecs->verts.end(), begin(face), end(face));
        vecs->uvs.insert(vecs->uvs.end(), begin(new_uvs), end(new_uvs));
        for (int i = 0; i < 6; i ++) {
            vecs->light.push_back(0.6f);
            vecs->mats.push_back(mat);
        }
        vecs->num_verts += 6;
    }
    
    block = world->get_global(gx, gy+scale, gz, scale);
    if (block == nullptr or block->is_air(0, -1, 0)) {
        GLfloat face[] = {
            x + scale, y + scale, z + scale,
            x + scale, y + scale, z,
            x, y + scale, z,
            x, y + scale, z,
            x, y + scale, z + scale,
            x + scale, y + scale, z + scale
        };
        vecs->verts.insert(vecs->verts.end(), begin(face), end(face));
        vecs->uvs.insert(vecs->uvs.end(), begin(new_uvs), end(new_uvs));
        for (int i = 0; i < 6; i ++) {
            vecs->light.push_back(0.8f);
            vecs->mats.push_back(mat);
        }
        vecs->num_verts += 6;
    }
    
    block = world->get_global(gx+scale, gy, gz, scale);
    if (block == nullptr or block->is_air(-1, 0, 0)) {
        GLfloat face[] = {
            x + scale, y + scale, z + scale,
            x + scale, y, z + scale,
            x + scale, y, z,
            x + scale, y, z,
            x + scale, y + scale, z,
            x + scale, y + scale, z + scale
        };
        vecs->verts.insert(vecs->verts.end(), begin(face), end(face));
        vecs->uvs.insert(vecs->uvs.end(), begin(new_uvs), end(new_uvs));
        for (int i = 0; i < 6; i ++) {
            vecs->light.push_back(0.4f);
            vecs->mats.push_back(mat);
        }
        vecs->num_verts += 6;
    }
    //cout << "done rendering" << endl;
    //cout << *num_verts << endl;
    //exit(1);
}

Chunk* Pixel::subdivide(function<void(int,int,int,Pixel*)> func) {
    //cout << "subdivide " << endl;
    if (scale == 1) {
        cout << "error: block alreasy at scale 1" << endl;
        return nullptr;
    }
    Chunk * newchunk = new Chunk(px, py, pz, scale, parent);
    for (int x = 0; x < csize; x ++) {
        for (int y = 0; y < csize; y ++) {
            for (int z = 0; z < csize; z ++) {
                int gx, gy, gz;
                Pixel * pix = new Pixel(x, y, z, scale/csize, newchunk);
                pix->global_position(&gx, &gy, &gz);
                newchunk->blocks[x][y][z] = pix;
                func(gx,gy,gz,pix);
            }
        }
    }
    //cout << '(' << parent->get(px,py,pz)->get() << ' ' << value << ')' << endl;
    //cout << "ysay " << endl;
    if (parent != nullptr) {
        //cout << parent << ' ' << parent->blocks[px][py][pz] << endl;
        parent->blocks[px][py][pz] = newchunk;
    }
    //cout << this << " this" << endl;
    //delete this;
    //cout << 678 << endl;
    return newchunk;
}



#endif
