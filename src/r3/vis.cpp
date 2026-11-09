#include "vis.h"
#include "grid.h"
#include <cassert>

using namespace std;

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

float distance(float first_y, float first_x, float second_y, float second_x) {
  float dy = second_y - first_y;
  float dx = second_x - first_x;
  return sqrt(dy * dy + dx * dx);
}

float slope(float viewpoint_y, float viewpoint_x, float viewpoint_height,
            float target_y, float target_x, float target_height) {

  float d = distance(viewpoint_y, viewpoint_x, target_y, target_x);
  // This has to be here because if the distance is zero (viewpoint ~ target),
  // then we have a division by zero below
  if (d == 0) {
    return 0;
  }

  return (target_height - viewpoint_height) / d;
}

// Interpolates between two neighboring cells. value is the fractional position
// along the interpolated axis; coordinate is the exact position on the other.
float interpolate(int interpolate_y, float value, int coordinate,
                  const Grid* dem) {
  // any other value would step diagonally or skip a cell below
  assert(interpolate_y == 0 || interpolate_y == 1);
  // a negative value truncates toward 0, silently giving the wrong cell
  assert(value >= 0);
  int below = static_cast<int>(value);
  float t = value - below;

  int r = interpolate_y ? below : coordinate;
  int c = interpolate_y ? coordinate : below;
  float h1 = grid_get(dem, r, c);
  // The next cell along the interpolated axis. ! flips 0 and 1, so exactly one
  // of r and c moves by 1: r if interpolate_y is 1, c if it's 0.
  float h2 = grid_get(dem, r + interpolate_y, c + !interpolate_y);

  if (h1 == dem->nodata_value || h2 == dem->nodata_value) {
    return dem->nodata_value;
  }

  // 1.0f avoids unnecessary conversion to double
  return h1 * (1.0f - t) + h2 * t;
}

// Checks where the ray crosses one family of grid lines. "line" is the axis we
// step along and "other" is the axis we solve for. Returns VISIBLE if the ray
// gets through, otherwise NOT_VISIBLE or VIS_NODATA.
static uint8_t check_crossings(const Grid* elev_grid, int interpolate_y,
                               int start, int end, int other_start,
                               float other_step, float vz, float los_slope) {
  int dir = end > start ? 1 : -1;
  int steps = abs(end - start);
  for (int k = 1; k < steps; k++) {
    int line = start + k * dir;
    float other = other_start + static_cast<float>(line - start) * other_step;
    float height = interpolate(interpolate_y, other, line, elev_grid);
    if (height == elev_grid->nodata_value) {
      return VIS_NODATA;
    }

    float s = slope(start, other_start, vz, line, other, height);
    if (s > los_slope) {
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

  // slope from viewpoint to target
  float los_slope = slope(vr, vc, vz, r, c, pz);

  // horizontal angle from viewpoint to target
  float dy = r - vr;
  float dx = c - vc;

  // Vertical intersections, then horizontal ones
  uint8_t result =
      check_crossings(elev_grid, 1, vc, c, vr, dy / dx, vz, los_slope);
  if (result == VISIBLE) {
    result = check_crossings(elev_grid, 0, vr, r, vc, dx / dy, vz, los_slope);
  }
  return result;
}

long compute_viewshed_grid(const Grid* elev_grid, VisGrid* vis_grid, int vr,
                           int vc, float vh) {

  // a nodata viewpoint has no height
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
