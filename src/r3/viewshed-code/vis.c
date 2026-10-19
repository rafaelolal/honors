#include <stdio.h>
#include <math.h>
#include "grid.h"
#include "vis.h"

/* ---------------------------------------------------------------------------
   is_point_visible

   THIS IS THE FUNCTION YOU WRITE. Everything else in this project is
   bookkeeping; this is the algorithm.

   The straightforward algorithm, for one target point p = (r,c):

     - the viewpoint sits at height  H_v = elev(vr,vc) + vh
     - the target sits at height     H_p = elev(r,c)
     - walk the horizontal segment from (vc,vr) to (c,r) in grid coordinates
       and find every point q where it crosses a grid line
     - at each q, interpolate the terrain height from the two grid points q
       falls between, then compute the vertical angle of q as seen from v
     - p is visible iff every q has vertical angle strictly less than p's

   Vertical angle of a point at planar distance d and height H:
       angle = atan((H - H_v) / d)
   Since atan is monotone and expensive, compare the SLOPES (H - H_v)/d
   directly and never call atan at all.

   Suggested build order (Toma's, and it is the right one):
     1. only the vertical grid lines (x = integer), only quadrant 1
     2. all four quadrants, still vertical lines only
     3. add the horizontal grid lines (y = integer); the viewshed will SHRINK
   Return 1 unconditionally to start, look at the output, then tighten it.
--------------------------------------------------------------------------- */
int is_point_visible(Grid* elev_grid, int r, int c, int vr, int vc, float vh) {
  float vz = grid_get(elev_grid, vr, vc) + vh;
  float pz = grid_get(elev_grid, r, c);

  if (r == vr && c == vc) return VISIBLE;

  /* TODO: replace this with the real test. */
  (void) vz; (void) pz;
  return VISIBLE;
}

int compute_viewshed_grid(Grid* elev_grid, Grid* vis_grid, int vr, int vc, float vh) {
  int r, c, count = 0;
  for (r = 0; r < elev_grid->nrows; r++) {
    for (c = 0; c < elev_grid->ncols; c++) {
      if (grid_get_nodata(elev_grid, r, c)) {
        grid_set(vis_grid, r, c, NOT_VISIBLE);
        continue;
      }
      if (is_point_visible(elev_grid, r, c, vr, vc, vh) == VISIBLE) {
        grid_set(vis_grid, r, c, VISIBLE);
        count++;
      } else {
        grid_set(vis_grid, r, c, NOT_VISIBLE);
      }
    }
  }
  return count;
}
