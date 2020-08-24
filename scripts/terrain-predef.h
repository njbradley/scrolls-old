#ifndef TERRAIN_PREDEF
#define TERRAIN_PREDEF

#include "classes.h"



class TerrainBase {
public:
	TerrainLoader* parent;
	TerrainBase(TerrainLoader* newparent);
	virtual int get_height(ivec2 pos) = 0;
	virtual char gen_func(ivec3 pos) = 0;
	virtual double valid_score(double wetness, double temp, double elev) = 0;
	virtual string name() = 0;
};

class TerrainBaseMerger: public TerrainBase { public:
	TerrainBase* first;
	TerrainBase* second;
	TerrainBaseMerger(TerrainBase* newfirst, TerrainBase* newsecond);
	TerrainBaseMerger();
	int get_height(ivec2 pos);
	char gen_func(ivec3 pos);
	double valid_score(double wetness, double temp, double elev);
	string name();
};

class TerrainObject {
public:
	ivec3 size;
	int radius;
	TerrainLoader* parent;
	TerrainObject(TerrainLoader* loader, ivec3 nsize, int nradius);
	virtual char gen_func(ivec3,ivec3) = 0;
	virtual ivec3 get_nearest(ivec3);
	virtual int priority() = 0;
	virtual bool is_valid(ivec3) = 0;
	ivec3 get_nearest_3d(ivec3);
	ivec2 get_nearest_2d(ivec2);
};

class TerrainObjectMerger {
public:
	TerrainLoader* parent;
	vector<TerrainObject*> objs;
	TerrainObjectMerger(TerrainLoader* newparent);
	~TerrainObjectMerger();
	char gen_func(ivec3);
};

class TerrainLoader {
public:
	int seed;
	TerrainObjectMerger objmerger;
	vector<TerrainBase*> bases;
	TerrainLoader(int nseed);
	~TerrainLoader();
	TerrainBaseMerger get_base(ivec3 pos);
	string get_biome_name(ivec3 pos);
	double get_wetness(ivec3 pos);
	double get_temp(ivec3 pos);
	double get_elev(ivec3 pos);
	int get_height(ivec2 pos);
	char gen_func(ivec3);
};
	
class Plains: public TerrainBase { public:
	Plains(TerrainLoader* newparent);
	virtual int get_height(ivec2 pos);
	virtual char gen_func(ivec3 pos);
	virtual double valid_score(double wetness, double temp, double elev);
	virtual string name();
};

class Forest: public TerrainBase { public:
	Forest(TerrainLoader* newparent);
	virtual int get_height(ivec2 pos);
	virtual char gen_func(ivec3 pos);
	virtual double valid_score(double wetness, double temp, double elev);
	virtual string name();
};

class Mountains: public TerrainBase { public:
	Mountains(TerrainLoader* newparent);
	virtual int get_height(ivec2 pos);
	virtual char gen_func(ivec3 pos);
	virtual double valid_score(double wetness, double temp, double elev);
	virtual string name();
};

class Tree: public TerrainObject {
public:
	Tree(TerrainObjectMerger*);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
	bool is_valid(ivec3);
};

class TallTree: public TerrainObject {
public:
	TallTree(TerrainObjectMerger*);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
	bool is_valid(ivec3);
};

class BigTree: public TerrainObject {
public:
	BigTree(TerrainObjectMerger*);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
	bool is_valid(ivec3);
};

class Cave: public TerrainObject {
public:
	Cave(TerrainObjectMerger*);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
	bool is_valid(ivec3);
};

class FileTerrain: public TerrainObject {
public:
	char* data;
	int priority_level;
	unordered_set<string> biomes;
	FileTerrain(TerrainObjectMerger*, string path);
	~FileTerrain();
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
	bool is_valid(ivec3);
};

#endif
