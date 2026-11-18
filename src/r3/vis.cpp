#include "vis.h"
#include "grid.h"
#include <cassert>

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

// Interpolates between two neighboring cells. value is the fractional position
// along the interpolated axis while coordinate is the exact position on the
// other.
template <int IY>
static inline float interpolate(float value, int coordinate, const Grid* dem,
                                bool* nodata) {
  // a negative value means something went wrong...
  assert(value >= 0);
  int below = static_cast<int>(value);
  float t = value - below;

  int r = IY ? below : coordinate;
  int c = IY ? coordinate : below;
  float h1 = grid_get(dem, r, c);
  // The next cell along the interpolated axis. ! flips 0 and 1, so exactly one
  // of r and c moves by 1 -> r if IY is 1, c if it's 0.
  float h2 = grid_get(dem, r + IY, c + !IY);

  *nodata = (h1 == dem->nodata_value) | (h2 == dem->nodata_value);

  // 1.0f avoids unnecessary conversion to double
  return h1 * (1.0f - t) + h2 * t;
}

// Checks where the ray crosses one family of grid lines. "line" is the axis we
// step along and "other" is the axis we solve for. Returns VISIBLE if the ray
// gets through, otherwise NOT_VISIBLE or VIS_NODATA.
template <int IY>
static uint8_t check_crossings(const Grid* elev_grid, int start, int end,
                               float other_start, float other_step, float vz,
                               float target_rise) {
  int dir = end > start ? 1 : -1;
  // Multiply by dir to have positive sign
  int steps = dir * (end - start);
  // Float copies of steps and k
  // the int ones run the loop
  // the float ones do the math without need for conversting all the time
  float steps_f = steps;
  float k_f = 1.0f;
  // folding in dir only flips the sign, so it's exact
  other_step *= dir;

  int line = start + dir;
  for (int k = 1; k < steps; k++, k_f++, line += dir) {
    float other = other_start + k_f * other_step;
    bool nodata;
    float height = interpolate<IY>(other, line, elev_grid, &nodata);
    if (nodata) {
      return VIS_NODATA;
    }

    if ((height - vz) * steps_f > target_rise * k_f) {
      return NOT_VISIBLE;
    }
  }

  return VISIBLE;
}

uint8_t is_point_visible(const Grid* elev_grid, int r, int c, int vr, int vc,
                         float vh) {

  if (grid_is_nodata(elev_grid, r, c)) {
    return VIS_NODATA;
  }

  if (vr == r && vc == c) {
    return VISIBLE;
  }

  float vz = grid_get(elev_grid, vr, vc) + vh;
  float pz = grid_get(elev_grid, r, c);

  // height of the target above the viewpoint
  float target_rise = pz - vz;

  // horizontal angle from viewpoint to target
  float dy = r - vr;
  float dx = c - vc;

  // Vertical intersections, then horizontal ones. The two divisions are once
  // per target, for where the ray sits on the other axis.
  uint8_t result =
      check_crossings<1>(elev_grid, vc, c, vr, dy / dx, vz, target_rise);
  if (result == VISIBLE) {
    result = check_crossings<0>(elev_grid, vr, r, vc, dx / dy, vz, target_rise);
  }
  return result;
}

long compute_viewshed_grid(const Grid* elev_grid, VisGrid* vis_grid, int vr,
                           int vc, float vh) {

  // grid_get also checks it's inside the grid
  assert(!grid_is_nodata(elev_grid, vr, vc));

  long count = 0;
  for (int r = 0; r < (int)elev_grid->nrows; r++) {
    for (int c = 0; c < (int)elev_grid->ncols; c++) {
      uint8_t v = is_point_visible(elev_grid, r, c, vr, vc, vh);
      // the grid starts all NOT_VISIBLE, so only other values need writing
      if (v != NOT_VISIBLE) {
        visgrid_set(vis_grid, r, c, v);
      }
      count += v == VISIBLE;
    }
  }

  return count;
}
