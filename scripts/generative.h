#ifndef GENERATIVE
#define GENERATIVE

#include <iostream>
    using std::cout;
    using std::endl;
#include <math.h>

int rseed = 4094240;
int seed = rseed;

double lerp(double a0, double a1, double w) {
    return (1.0f - w)*a0 + w*a1;
}

int cash(int x, int y){
    int h = seed + x*374761393 + y*668265263; //all constants are prime
    h = (h^(h >> 13))*1274126177;
    return h^(h >> 16);
}

double randfloat(int x, int y) {
    return (double)cash(x,y) / INT_MAX;
}

// Computes the dot product of the distance and gradient vectors.
double dotGridGradient(int ix, int iy, double x, double y) {

    // Precomputed (or otherwise) gradient vectors at each grid node
    //extern float Gradient[IYMAX][IXMAX][2];
    double pvecx = randfloat(ix*2, iy)*2-1;
    double pvecy = randfloat(ix*2+1, iy)*2-1;
    
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
double perlin(double x, double y) {
    
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
    
    n0 = dotGridGradient(x0, y0, x, y);
    n1 = dotGridGradient(x1, y0, x, y);
    ix0 = lerp(n0, n1, sx);
    
    //cout << n0 << ' ' << n1 << endl;
    
    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    ix1 = lerp(n0, n1, sx);
    
    value = lerp(ix0, ix1, sy);
    //cout << value << endl;
    //exit(0);
    seed ++;
    return value;
}

double scaled_perlin(double x, double y, double height, double width) {
    return perlin((float)x/(width), (float)y/(width)) *height + height/2;
}

double get_height(int x, int y) {
    return (scaled_perlin(x,y,16,20) + scaled_perlin(x,y,8,10))*scaled_perlin(x,y,2,40) + scaled_perlin(x,y,4,5);
}

char generative_function( int x, int y, int z ) {
    seed = rseed;
    double height = get_height(x, z);
    //cout << height << endl;
    //double treepos = scaled_perlin(x,z,1,30);
    //if (treepos > 0.9f) {
    if (y < 4) {
        return 7;
    } if (y < 8 and y < height) {
        return 5;
    } if (y < height-1) {
        return 2;
    } if (y < height) {
        return 3;
    } //if (y < trees+height+10 and y > height + 10) {
      //  return 4;
    //}
    return 0;
}


#endif
