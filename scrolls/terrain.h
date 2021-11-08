#ifndef BASE_TERRAIN_H
#define BASE_TERRAIN_H

#include "classes.h"
#include "plugins.h"



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

class TerrainLoader { public:
	BASE_PLUGIN_HEAD(TerrainLoader, (int seed));
	int seed;
	
	TerrainLoader(int newseed);
	virtual ~TerrainLoader() {}
 	char get_biome(ivec3 pos);
	char get_biome(ivec3 pos, ClimateParams params);
	template <typename Elev>
	char get_biome(ivec3 pos, ClimateParams params);
	template <typename Elev, typename Land>
	char get_biome(ivec3 pos, ClimateParams params);
	int get_height(ivec2 pos);
	int gen_func(ivec3 pos);
	
	virtual Block* generate_chunk(ivec3 pos);
	Blocktype gen_block(ostream& ofile, int gx, int gy, int gz, int scale);
};



#endif
