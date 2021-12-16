#include "terrainshapes.h"
















typedef ShapeResolver<
	SolidType<FlatTerrain<32>, blocktypes::stone>
> FlatWorld;
EXPORT_PLUGIN_TEMPLATE(FlatWorld);

typedef ShapeResolver<
	SolidType<Shift<FractalPerlin2d<64,8,0>, 0,0,0>, blocktypes::stone>,
	SolidType<Shift<FractalPerlin2d<64,8,0>, 0,5,0>, blocktypes::dirt>,
	SolidType<Shift<FractalPerlin2d<64,8,0>, 0,6,0>, blocktypes::grass>
> Simple2D;
EXPORT_PLUGIN_TEMPLATE(Simple2D);

typedef ShapeResolver<
	SolidType<FractalPerlin3d<64,8,0>, blocktypes::stone>
> Simple3D;
EXPORT_PLUGIN_TEMPLATE(Simple3D);


// typedef HeightFalloff<RidgePerlin3d<64,0>, 1> LandLevel;
typedef HeightFalloff<Add<Perlin3d<64,0>, Perlin3d<9,1>, RidgePerlin3d<32,4>, Perlin2d<512,9>>, 1> LandLevel;


typedef ShapeResolver<
	SolidType<LandLevel, blocktypes::stone>,
	SolidType<Shift<AddVal<LandLevel, -3>, 0,2,0>, blocktypes::dirt>,
	SolidType<Shift<AddVal<LandLevel, -5>, 0,5,0>, blocktypes::grass>
> TestWorld;
EXPORT_PLUGIN_TEMPLATE(TestWorld);
