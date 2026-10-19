#ifndef __vis_h
#define __vis_h

#include "grid.h"

#define VISIBLE 1
#define NOT_VISIBLE 0

/* elev_grid: input elevation grid
   r, c:      the point being tested
   vr, vc:    viewpoint row and column
   vh:        viewpoint height ABOVE the terrain at (vr,vc)
   returns VISIBLE if (r,c) is visible from the viewpoint, NOT_VISIBLE otherwise
 */
int is_point_visible(const Grid* elev_grid, int r, int c, int vr, int vc,
                     float vh);

/* fills vis_grid with VISIBLE/NOT_VISIBLE for every cell; returns viewshed size
 */
int compute_viewshed_grid(const Grid* elev_grid, Grid* vis_grid, int vr, int vc,
                          float vh);

#endif
