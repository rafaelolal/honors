#include "vis.h"
#include "grid.h"
#include <cassert>
#include <iostream>
#include <math.h>
#include <stdio.h>

extern int DEBUG_LEVEL;

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

// constexpr is const and known at compile time??
// 50 meters
constexpr int SCALE = 50;
constexpr float PI = 3.141592653589793;

struct Point {
  int y;
  int x;
};

Point viewpoint{3, 3};

// Parameters aren't of type Point to keep functions generalizable
float distance(float first_y, float first_x, float second_y, float second_x) {
  // AI recommended this function
  // ??
  return hypot(second_y - first_y, second_x - first_x);
}

float altitude(float viewpoint_y, float viewpoint_x, float viewpoint_height,
               float target_y, float target_x, float target_height) {

  float d = distance(viewpoint_y, viewpoint_x, target_y, target_x);
  // This has to be here because if the distance is zero (viewpoint ~ target),
  // then we have a division by zero below
  if (d == 0) {
    return 0;
  }

  // TOA
  // tan(z) = opposite / adjacent
  // z = atan(opposite / adjacent)
  float height_difference = target_height - viewpoint_height;
  float z = atan(height_difference / d);
  if (DEBUG_LEVEL == 1) {
    cout << "Calculated altitude from (" << viewpoint_y << ", " << viewpoint_x
         << ") to (" << target_y << ", " << target_x << "): distance: " << d
         << " height difference: " << (height_difference) << " altitude: " << z
         << " radians" << endl;
  }
  return z;
}

// This method looks more confusing than it has to be because I made it
// interpolate either direction. Not sure if there is a more elegant way to do
// it. Value is the y or x depending on interpolate_y coordinate is the other
// value in the pair that is exactly known
float interpolate(int interpolate_y, float value, int coordinate,
                  const Grid* dem) {
  assert(interpolate_y == 0 || interpolate_y == 1);
  assert(dem);
  int size = interpolate_y == 1 ? dem->nrows : dem->ncols;
  assert(coordinate >= 0 && coordinate < size);

  assert(value >= 0);
  int v_below = static_cast<int>(value);
  int v_above = v_below + 1;
  assert(v_below >= 0 && v_below < size);
  assert(v_above >= 0 && v_above < size);

  int t = value - v_below;
  assert(t >= 0 && t <= 1);
  int y_below, y_above, x_below, x_above;
  if (interpolate_y == 1) {
    y_below = v_below;
    y_above = v_above;
    x_below = coordinate;
    x_above = coordinate;
  } else {
    y_below = coordinate;
    y_above = coordinate;
    x_below = v_below;
    x_above = v_above;
  }

  if (grid_get_nodata(dem, y_below, x_below) ||
      grid_get_nodata(dem, y_above, x_above)) {
    return dem->nodata_value;
  }

  float ans = grid_get(dem, y_below, x_below) * (1.0 - t) +
              grid_get(dem, y_above, x_above) * t;
  if (DEBUG_LEVEL == 1) {
    cout << "Interpolated between points (" << y_below << ", " << x_below
         << ", " << grid_get(dem, y_below, x_below) << ") and (" << y_above
         << ", " << x_above << ", " << grid_get(dem, y_above, x_above)
         << ") with t: " << t << " to get ans: " << ans << endl;
  }
  return ans;
}

int is_point_visible(const Grid* elev_grid, int r, int c, int vr, int vc,
                     float vh) {
  assert(vr >= 0 && vr < elev_grid->nrows);
  assert(vc >= 0 && vc < elev_grid->ncols);
  assert(r >= 0 && r < elev_grid->nrows);
  assert(c >= 0 && c < elev_grid->ncols);

  if (vr == r && vc == c) {
    return 1;
  }

  // altitude from viewpoint to target
  // phi in math convention
  float los_altitude = altitude(vr, vc, grid_get(elev_grid, vr, vc), r, c,
                                grid_get(elev_grid, r, c));

  // 90 degrees
  assert(los_altitude >= -PI / 2 && los_altitude <= PI / 2);

  // horizontal angle from viewpoint to target
  int dy = r - vr;
  int dx = c - vc;
  // theta in math convention
  float azimuth = atan(static_cast<float>(dy) / static_cast<float>(dx));
  // 90 degrees
  // TODO: double check if this is correct
  // assert(azimuth >= -PI / 2 && azimuth <= PI / 2);

  if (DEBUG_LEVEL == 1) {
    cout << "viewpoint: (" << vr << ", " << vc << ") target: (" << r << ", "
         << c << ") azimuth: " << azimuth << " los_altitude: " << los_altitude
         << endl;
  }

  // Vertical intersections
  int dir_x = dx > 0 ? 1 : -1;
  // Start one away in the direction of target, and move towards it
  if (DEBUG_LEVEL == 1) {
    cout << "y_calcs" << endl;
  }
  // TODO: need to immediately get rid of this if!!!!!
  if (vc != c) {
    // Adding dir_x to initial x only because we don't want to check the
    // viewpoint itself, and we want to check the target up to before the target
    // itself.
    for (int x = vc + dir_x; x != c; x += dir_x) {
      // y is ray vertical intersection point y-value
      // Negative angles means I am going backwards, just take abs instead

      float y_calc = static_cast<float>(x - vc) * tan(azimuth) + vr;
      if (DEBUG_LEVEL == 1) {
        cout << "calculated y: (" << y_calc << ", " << x
             << ") for azimuth: " << azimuth << endl;
      }
      float height = interpolate(1, y_calc, x, elev_grid);
      if (height == elev_grid->nodata_value) {
        return NOT_VISIBLE;
      }
      assert(height >= 0);

      float z =
          altitude(vr, vc, grid_get(elev_grid, vr, vc), y_calc, x, height);
      // 90 degrees
      assert(z >= -PI / 2 && z <= PI / 2);

      if (DEBUG_LEVEL == 1) {
        cout << "altitude between viewpoint and vertical intersection: " << z
             << " los_altitude: " << los_altitude << endl;
      }

      if (z > los_altitude) {
        return NOT_VISIBLE;
      }
    }
  }

  int dir_y = r - vr > 0 ? 1 : -1;
  // Horizontal intersections
  // Start one away in the direction of target, and move towards it
  if (DEBUG_LEVEL == 1) {
    cout << "x_calcs" << endl;
  }
  // TODO: need to immediately get rid of this if!!!!!
  if (vr != r) {
    for (int y = vr + dir_y; y != r; y += dir_y) {
      // x is ray horizontal intersection point x-value
      float x_calc = static_cast<float>(y - vr) / tan(azimuth) + vc;
      if (DEBUG_LEVEL == 1) {
        cout << "calculated x: (" << y << ", " << x_calc
             << ") for azimuth: " << azimuth << endl;
      }
      float height = interpolate(0, x_calc, y, elev_grid);
      if (height == elev_grid->nodata_value) {
        // TODO: what value to return here?
        return NOT_VISIBLE;
      }
      assert(height >= 0);

      float z =
          altitude(vr, vc, grid_get(elev_grid, vr, vc), y, x_calc, height);
      // 90 degrees
      assert(z >= -PI / 2 && z <= PI / 2);

      if (DEBUG_LEVEL == 1) {
        cout << "altitude between viewpoint and horizontal intersection: " << z
             << " los_altitude: " << los_altitude << endl;
      }

      if (z > los_altitude) {
        return NOT_VISIBLE;
      }
    }
  }

  return VISIBLE;
}

int compute_viewshed_grid(const Grid* elev_grid, Grid* vis_grid, int vr, int vc,
                          float vh) {

  assert(vr >= 0 && vr < elev_grid->nrows);
  assert(vc >= 0 && vc < elev_grid->ncols);
  // The viewpoint must be a valid point in the grid, not a nodata value
  assert(!grid_get_nodata(elev_grid, vr, vc));
  assert(vh >= 0);

  int r, c, count = 0;
  for (r = 0; r < elev_grid->nrows; r++) {
    for (c = 0; c < elev_grid->ncols; c++) {
      // A nice reformat!
      int is_visible = !grid_get_nodata(elev_grid, r, c) &&
                       is_point_visible(elev_grid, r, c, vr, vc, vh);
      count += is_visible;
      grid_set(vis_grid, r, c, is_visible);

      // if (grid_get_nodata(elev_grid, r, c)) {
      //   grid_set(vis_grid, r, c, NOT_VISIBLE);
      // } else if (is_point_visible(elev_grid, r, c, vr, vc, vh) == VISIBLE) {
      //   grid_set(vis_grid, r, c, VISIBLE);
      //   count++;
      // } else {
      //   grid_set(vis_grid, r, c, NOT_VISIBLE);
      // }
    }
  }
  return count;
}
