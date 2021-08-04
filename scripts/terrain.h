#ifndef TERRAIN_PREDEF
#define TERRAIN_PREDEF

#include "classes.h"

#define BLOCK_DIVIDE -2
#define BLOCK_VOID -1

enum ShapeResult {
	SHAPE_OUTSIDE,
	SHAPE_INSIDE,
	SHAPE_INTERSECTS
};

class TerrainShape { public:
	virtual ShapeResult collide(ivec4 pos) = 0;
};

class TerrainGenerator { public:
	virtual int gen_func(ivec4 pos) = 0;
};

template <typename ShapeType, typename Generator>
class TerrainObject : public TerrainGenerator { public:
	virtual int gen_func(ivec4 pos);
};

template <typename ... Objs>
class TerrainObjectGroup : public TerrainGenerator { public:
	virtual int gen_func(ivec4 pos);
};

class TerrainLoader { public:
	Block* block;
	ivec3 offset;
	
	vector<TerrainGenerator*> generators;
	
	TerrainLoader(ivec3 off);
};
	















class HeightMap { public:
	virtual int get_height(TerrainLoader* loader, ivec2 pos) = 0;
};

class LandGen { public:
	virtual char gen_func(TerrainLoader* loader, ivec3 pos) = 0;
};

template <typename Elev1, typename Elev2, double (*ratio)(TerrainLoader* loader, ivec2 pos)>
class HeightMapMerger : public HeightMap { public:
	int get_height(TerrainLoader* loader, ivec2 pos);
};

class TerrainObjectPlacer { public:
	virtual ivec3 get_nearest(TerrainLoader* loader, ivec3 pos) = 0;
	ivec3 get_nearest_3d(TerrainLoader* loader, ivec3 pos, ivec3 size, int radius, int position_offset = 0);
	ivec3 get_nearest_2d(TerrainLoader* loader, ivec3 pos, ivec3 size, int radius, int position_offset = 0);
};
	

class TerrainObject: public TerrainObjectPlacer { public:
	virtual char gen_func(TerrainLoader* loader, ivec3 offset, ivec3 pos) = 0;
};

template <typename ... Objs> class TerrainObjectMerger { public:
	template <typename Firstobj, typename Secondobj, typename ... Otherobjs> char gen_func(TerrainLoader* loader, ivec3 pos);
	template <typename Lastobj> char gen_func(TerrainLoader* loader, ivec3 pos);
	char gen_func(TerrainLoader* loader, ivec3 pos);
};

class BiomeBase { public:
	virtual int get_height(TerrainLoader* loader, ivec2 pos) = 0;
	virtual char gen_func(TerrainLoader* loader, ivec3 pos) = 0;
};

template <typename Elev, typename Land, typename ... Objs>
class Biome : public BiomeBase { public:
	static_assert(std::is_base_of<HeightMap,Elev>::value, "");
	static_assert(std::is_base_of<LandGen,Land>::value, "");
	TerrainObjectMerger<Objs ...> merger;
	int get_height(TerrainLoader* loader, ivec2 pos);
	char gen_func(TerrainLoader* loader, ivec3 pos);
};

class ClimateParams { public:
	double wetness;
	double temp;
	ClimateParams(TerrainLoader* loader, ivec3 pos);
};

class TerrainLoader { public:
	int seed;
	TerrainLoader(int newseed);
 	char get_biome(ivec3 pos);
	char get_biome(ivec3 pos, ClimateParams params);
	template <typename Elev>
	char get_biome(ivec3 pos, ClimateParams params);
	template <typename Elev, typename Land>
	char get_biome(ivec3 pos, ClimateParams params);
	int get_height(ivec2 pos);
	int gen_func(ivec4 pos);
};

template <int sx, int sy, int sz, int radius, int pos_offset>
class Placer2D : public TerrainObjectPlacer { public:
	ivec3 get_nearest(TerrainLoader* loader, ivec3 pos);
};

template <typename Placer, typename ... Objs> class RareObjects: public TerrainObject { public:
	static_assert(std::is_base_of<TerrainObjectPlacer,Placer>::value, "");
	ivec3 get_nearest(TerrainLoader* loader, ivec3 pos);
	char gen_func(TerrainLoader* loader, ivec3 offset, ivec3 pos);
	template <typename FirstObj, typename SecondObj, typename ... OtherObjs>
	char gen_func(TerrainLoader* loader, ivec3 offset, ivec3 pos, int index);
	template <typename LastObj>
	char gen_func(TerrainLoader* loader, ivec3 offset, ivec3 pos, int index);
};

template <int off> class Tree : public TerrainObject { public:
	ivec3 get_nearest(TerrainLoader* loader, ivec3 pos);
	char gen_func(TerrainLoader* loader, ivec3 offset, ivec3 pos);
};

class TallTree : public TerrainObject { public:
	ivec3 get_nearest(TerrainLoader* loader, ivec3 pos);
	char gen_func(TerrainLoader* loader, ivec3 offset, ivec3 pos);
};

class BigTree : public TerrainObject { public:
	ivec3 get_nearest(TerrainLoader* loader, ivec3 pos);
	char gen_func(TerrainLoader* loader, ivec3 offset, ivec3 pos);
};

class Flat : public HeightMap { public:
	int get_height(TerrainLoader* loader, ivec2 pos);
};

class Shallow : public HeightMap { public:
	int get_height(TerrainLoader* loader, ivec2 pos);
};

class Steep : public HeightMap { public:
	int get_height(TerrainLoader* loader, ivec2 pos);
};

class Plains : public LandGen { public:
	char gen_func(TerrainLoader* loader, ivec3 pos);
};
	


/*
	

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
	int unique_seed;
	int position_offset;
	TerrainLoader* parent;
	TerrainObject(TerrainLoader* loader, ivec3 nsize, int nradius, int new_unique_seed);
	virtual char gen_func(ivec3,ivec3) = 0;
	virtual ivec3 get_nearest(ivec3);
	virtual int priority() = 0;
	virtual bool is_valid(ivec3) = 0;
	ivec3 get_nearest_3d(ivec3);
	ivec3 get_nearest_2d(ivec3);
	int poshash(ivec3 pos, int myseed);
	double poshashfloat(ivec3 pos, int myseed);
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

class Flatworld: public TerrainBase { public:
	char block;
	int height;
	Flatworld(TerrainLoader* newparent, char newblock, int newheight);
	virtual int get_height(ivec2 pos);
	virtual char gen_func(ivec3 pos);
	virtual double valid_score(double wetness, double temp, double elev);
	virtual string name();
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

class BlanketForest: public TerrainBase { public:
	BlanketForest(TerrainLoader* newparent);
	virtual int get_height(ivec2 pos);
	virtual char gen_func(ivec3 pos);
	virtual double valid_score(double wetness, double temp, double elev);
	virtual string name();
};

class Tree: public TerrainObject {
public:
	Tree(TerrainObjectMerger*, int uid = 0);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
	bool is_valid(ivec3);
};

class TallTree: public TerrainObject {
public:
	TallTree(TerrainObjectMerger*, int uid = 0);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
	bool is_valid(ivec3);
};

class BigTree: public TerrainObject {
public:
	BigTree(TerrainObjectMerger*, int uid = 0);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
	bool is_valid(ivec3);
};

class Cave: public TerrainObject {
public:
	Cave(TerrainObjectMerger*, int uid = 0);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
	bool is_valid(ivec3);
};

class Lamp: public TerrainObject { public:
	Lamp(TerrainObjectMerger*, int uid = 0);
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
	FileTerrain(TerrainObjectMerger*, string path, int uid = 0);
	~FileTerrain();
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
	bool is_valid(ivec3);
};
*/
#endif
