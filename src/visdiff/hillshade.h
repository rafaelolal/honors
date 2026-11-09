// Per-cell hillshade, copied from r3's map.cpp so visdiff draws the same
// terrain as vis without depending on it.
#ifndef __hillshade_h
#define __hillshade_h

#include "grid.h"

#define PI 3.1415
#define FLAT_ASPECT 0
#define Z_FACTOR 5

float dzdx(const Grid* grid, int r, int c);
float dzdy(const Grid* grid, int r, int c);
float slope_rad(const Grid* grid, float dzdx, float dzdy);
float aspect_rad(const Grid* grid, float dzdx, float dzdy);

// hillshade of one cell in [0, 255], or the grid's nodata value
float hillshade_at(const Grid* grid, int r, int c);

#endif
