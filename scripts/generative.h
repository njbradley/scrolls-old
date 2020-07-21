#ifndef GENERATIVE
#define GENERATIVE

#include <iostream>
    using std::cout;
    using std::endl;
#include <math.h>
#include "blockdata-predef.h"
//#include "texture.h"



vector<pair<int,int> > paths;

double lerp(double a0, double a1, double w) {
    return (1.0f - w)*a0 + w*a1;
}

// int cash(int x, int y){
//     int h = seed + x*374761393 + y*668265263; //all constants are prime
//     h = (h^(h >> 13))*1274126177;
//     return h^(h >> 16);
// }

int hash4(int seed, int a, int b, int c, int d) {
	int sum = seed + 13680553*a + 47563643*b + 84148333*c + 80618477*d;
	sum = (sum ^ (sum >> 13)) * 750490907;
  return sum ^ (sum >> 16);
}

double randfloat(int seed, int x, int y, int a, int b) {
    return (double)(hash4(seed,x,y,a,b)%1000000) / 1000000;
}

// Computes the dot product of the distance and gradient vectors.
double dotGridGradient(int ix, int iy, double x, double y, int seed, int layer) {

    // Precomputed (or otherwise) gradient vectors at each grid node
    //extern float Gradient[IYMAX][IXMAX][2];
    double pvecx = randfloat(seed, ix, iy, layer, 0)*2-1;
    double pvecy = randfloat(seed, ix, iy, layer, 1)*2-1;
    
    //cout << pvecx << '-' << pvecy << endl;
    double dist = sqrt( pvecx*pvecx + pvecy*pvecy );
    pvecx /= dist;
    pvecy /= dist;
    //cout << pvecx << '.' << pvecy << endl;
    // Compute the distance vector
    double dx = x - (double)ix;
    double dy = y - (double)iy;

    // Compute the dot-product
    return (dx*pvecx + dy*pvecy);
}

// Compute Perlin noise at coordinates x, y
double perlin(double x, double y, int seed, int layer) {
    
    x += INT_MAX/2;
    y += INT_MAX/2;
    
    // Determine grid cell coordinates
    int x0 = (int)x;
    int x1 = x0 + 1;
    
    int y0 = (int)y;
    int y1 = y0 + 1;
    
    
    // Determine interpolation weights
    // Could also use higher order polynomial/s-curve here
    double sx = x - (double)x0;
    double sy = y - (double)y0;
    
    // Interpolate between grid point gradients
    double n0, n1, ix0, ix1, value;
    
    n0 = dotGridGradient(x0, y0, x, y, seed, layer);
    n1 = dotGridGradient(x1, y0, x, y, seed, layer);
    ix0 = lerp(n0, n1, sx);
    
    //cout << n0 << ' ' << n1 << endl;
    
    n0 = dotGridGradient(x0, y1, x, y, seed, layer);
    n1 = dotGridGradient(x1, y1, x, y, seed, layer);
    ix1 = lerp(n0, n1, sx);
    
    value = lerp(ix0, ix1, sy);
    //cout << value << endl;
    //crash(0);
    return value;
}

double scaled_perlin(double x, double y, double height, double width, int seed, int layer) {
    return perlin((float)x/(width), (float)y/(width), seed, layer) *height + height/2;
}


double get_heightmap(int x, int y, int seed) {
    return (scaled_perlin(x,y,16*2,20*2,seed,0) + scaled_perlin(x,y,8*2,10*2,seed,1))*scaled_perlin(x,y,2*2,40*2,seed,2)
     + scaled_perlin(x,y,4*2,5*2,seed,3) + scaled_perlin(x,y,500,500,seed,4);
}

void set_seed(int nseed) {
  //seed = nseed;
}

#endif
