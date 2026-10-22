// Laura Toma
//
#ifndef __map_h
#define __map_h

#include "grid.h"
#include "pixel_buffer.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PI 3.1415
#define FLAT_ASPECT 0
#define Z_FACTOR 5

typedef float Color[3];

////////////////////////////////////////////////////////////
// returns the distance between (x,y) and (a,b) truncated to u16
float dist(u16 x, u16 y, u16 a, u16 b);

////////////////////////////////////////////////////////////
/*
a b c
d e f
g h i

return nodata if e is nodata
return (c + 2f + i - a - 2d -g)/8
 */
float dzdx(const Grid* grid, int r, int c);

////////////////////////////////////////////////////////////
/*
a b c
d e f
g h i

return nodata if e is nodata
return (g + 2h + i - a - 2b -c)/8
 */
float dzdy(const Grid* grid, int r, int c);

////////////////////////////////////////////////////////////
float slope_rad(const Grid* grid, float dzdx, float dzdy);

////////////////////////////////////////////////////////////
float aspect_rad(const Grid* grid, float dzdx, float dzdy);

// populate with values hillshade_grid
void generate_hillshade(const Grid* grid, Grid* hillshade_grid);

// populate with values slope_grid
void generate_slope(const Grid* grid, Grid* slope_grid);

// populate with values aspect_grid
void generate_aspect(const Grid* grid, Grid* aspect_grid);

////////////////////////////////////////////////////////////
// set the pixel buffer to correpsond to a grayscale image of grid
void grid_grayscale_to_pixelbuffer(const Grid* grid, PixelBuffer pb);

////////////////////////////////////////////////////////////
// helper function
// return the index i of the interval values[i], values[i+1] that contains h
//  nb_intervals: nb intervals
// values: an array of nb_intervals + 1 values
int find_interval(float h, const Grid* grid, float* values, int nb_intervals);

///////////////////////////////////////////////////////////
// set the pixel buffer to correpsond to color discrete intervals image
// of grid
void grid_discretecolorintervals_to_pixelbuffer(const Grid* grid,
                                                PixelBuffer pb);

////////////////////////////////////////////////////////////
// set the pixels in the pixel buffer to correpsond to color gradient
// intervals image of grid
void grid_gradientcolorintervals_to_pixelbuffer(const Grid* grid,
                                                PixelBuffer pb);

////////////////////////////////////////////////////////////
// this grid contains values 0..255, put  those in a pixelbuffer
void grid_to_pixelbuffer(const Grid* grid, PixelBuffer pb);

// viewshed bitmap
void grid_vis_to_pixelbuffer(const Grid* vis_grid, int vr, int vc,
                             PixelBuffer pb);

////////////////////////////////////////////////////////////
// overlay pb2 on top of pb1, with given transparency alpha. when
// alpha=1, show pb1; when alpha=0, show pb2. write the output in pb1
void pixelbuffer_overlay(PixelBuffer pb1, PixelBuffer pb2, float alpha);

#endif
