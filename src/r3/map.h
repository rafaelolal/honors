// Laura Toma
//
#ifndef __map_h
#define __map_h

#include "grid.h"
#include "visgrid.h"

#define PI 3.1415
#define FLAT_ASPECT 0
#define Z_FACTOR 5

typedef float Color[3];

////////////////////////////////////////////////////////////
// returns the distance between (x,y) and (a,b)
float dist(int x, int y, int a, int b);

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

// hillshade of one cell in [0, 255], or the grid's nodata value
float hillshade_at(const Grid* grid, int r, int c);

////////////////////////////////////////////////////////////
// Writes the hillshade, viewshed and viewshed-over-hillshade bitmaps at full
// resolution in one pass, computing each pixel as its row is written, so no
// image is held in memory. alpha=1 shows the hillshade, alpha=0 the viewshed.
void write_bitmaps(const Grid* elev, const VisGrid* vis, int vr, int vc,
                   float alpha, const char* hillshade_path,
                   const char* viewshed_path, const char* overlay_path);

#endif
