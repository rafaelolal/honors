#ifndef __vis_h
#define __vis_h

#include "grid.h"

#define VISIBLE 1
#define NOT_VISIBLE 0

// debug output level (0=off, 1=verbose); defined in vismain.cpp, set with -d
extern int DEBUG_LEVEL;

/* elev_grid: input elevation grid
   r, c:      the point being tested
   vr, vc:    viewpoint row and column
   vh:        viewpoint height ABOVE the terrain at (vr,vc)
   returns VISIBLE if (r,c) is visible from the viewpoint, NOT_VISIBLE otherwise
 */
int is_point_visible(const Grid* elev_grid, int r, int c, int vr, int vc,
                     float vh);

// compute viewshed of (r,c,h) and populate  vis_grid with values
int compute_viewshed_grid(const Grid* elev_grid, Grid* vis_grid, int vr, int vc,
                          float vh);

#endif
