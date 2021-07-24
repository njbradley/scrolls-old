#ifndef GENERATIVE_PREDEF
#define GENERATIVE_PREDEF

#include "classes.h"

double lerp(double a0, double a1, double w);
int hash4(int seed, int a, int b, int c, int d);
double randfloat(int seed, int x, int y, int a, int b);
// Computes the dot product of the distance and gradient vectors.
double dotGridGradient(int ix, int iy, double x, double y, int seed, int layer);
// Compute Perlin noise at coordinates x, y
double perlin(double x, double y, int seed, int layer);
int voronoi2d(double x, double y, int seed, int layer, double threshold = 0.1);
int voronoi3d(double x, double y, double z, int seed, int layer, double threshold = 0.1);
double scaled_perlin(double x, double y, double height, double width, int seed, int layer);
double get_heightmap(int x, int y, int seed);
void set_seed(int nseed);

#endif
