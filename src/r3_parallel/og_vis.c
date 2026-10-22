#include "grid.h"
#include "vis.h"
#include <cassert>
#include <math.h>
#include <stdio.h>

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
constexpr int N = 10;
constexpr double PI = 3.141592653589793;

struct Point {
  int y;
  int x;
};

// Parameters aren't of type Point to keep functions generalizable
double distance(double first_y, double first_x, double second_y,
                double second_x) {
  // AI recommended this function
  return hypot(second_y - first_y, second_x - first_x);
}

double zinuth(double viewpoint_y, double viewpoint_x, double viewpoint_height,
              double target_y, double target_x, double target_height) {

  double d = distance(viewpoint_y, viewpoint_x, target_y, target_x);
  if (d == 0) {
    return 0;
  }

  // TOA
  // tan(z) = opposite / adjacent
  // z = atan(opposite / adjacent)
  double height_difference = target_height - viewpoint_height;
  double z = atan((height_difference) / d);
  cout << "Calculated zinuth from (" << viewpoint_y << ", " << viewpoint_x
       << ") to (" << target_y << ", " << target_x << "): distance: " << d
       << " height difference: " << (height_difference) << " zinuth: " << z
       << " radians" << endl;
  return z;
}

// This method looks more confusing than it has to be because I made it
// interpolate either direction. Not sure if there is a more elegant way to do
// it. Value is the y or x depending on interpolate_y coordinate is the other
// value in the pair that is exactly known
double interpolate(int interpolate_y, double value, int coordinate,
                   const double (&dem)[N][N]) {
  assert(interpolate_y == 0 || interpolate_y == 1);
  assert(coordinate >= 0 && coordinate < N);

  assert(value >= 0);
  int v_below = static_cast<int>(value);
  int v_above = v_below + 1;
  assert(v_below >= 0 && v_below < N);
  assert(v_above >= 0 && v_above < N);

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

  double ans = dem[y_below][x_below] * (1.0 - t) + dem[y_above][x_above] * t;
  cout << "Interpolated between points (" << y_below << ", " << x_below << ", "
       << dem[y_below][x_below] << ") and (" << y_above << ", " << x_above
       << ", " << dem[y_above][x_above] << ") with t: " << t
       << " to get ans: " << ans << endl;

  return ans;
}

int is_point_visible(Grid* elev_grid, int r, int c, int vr, int vc, float vh) {
  float vz = grid_get(elev_grid, vr, vc) + vh;
  float pz = grid_get(elev_grid, r, c);

  if (r == vr && c == vc)
    return VISIBLE;

  assert(viewpoint.y >= 0 && viewpoint.y < N);
  assert(viewpoint.x >= 0 && viewpoint.x < N);
  assert(target.y >= 0 && target.y < N);
  assert(target.x >= 0 && target.x < N);

  // zenith from viewpoint to target
  // phi in math convention
  double main_zenith =
      zenith(viewpoint.y, viewpoint.x, dem[viewpoint.y][viewpoint.x], target.y,
             target.x, dem[target.y][target.x]);
  // 90 degrees
  assert(main_zenith >= -PI / 2 && main_zenith <= PI / 2);

  // horizontal angle from viewpoint to target
  int dy = target.y - viewpoint.y;
  int dx = target.x - viewpoint.x;
  // theta in math convention
  double azimuth = atan(static_cast<double>(dy) / static_cast<double>(dx));
  // 90 degrees
  // TODO: double check if this is correct
  // assert(azimuth >= -PI / 2 && azimuth <= PI / 2);

  cout << "viewpoint: (" << viewpoint.y << ", " << viewpoint.x << ") target: ("
       << target.y << ", " << target.x << ") azimuth: " << azimuth
       << " main_zenith: " << main_zenith << endl;

  // Vertical intersections
  int dir_x = dx > 0 ? 1 : -1;
  // Start one away in the direction of target, and move towards it
  cout << "y_calcs" << endl;
  // TODO: need to immediately get rid of this if!!!!!
  if (viewpoint.x == target.x) {
    cout << "Vertical line, skipping vertical intersections" << endl;
  } else {
    // Adding dir_x to initial x only because we don't want to check the
    // viewpoint itself, and we want to check the target up to before the target
    // itself.
    for (int x = viewpoint.x + dir_x; x != target.x; x += dir_x) {
      // y is ray vertical intersection point y-value
      // Negative angles means I am going backwards, just take abs instead

      double y_calc =
          static_cast<double>(x - viewpoint.x) * tan(azimuth) + viewpoint.y;
      cout << "calculated y: (" << y_calc << ", " << x
           << ") for azimuth: " << azimuth << endl;
      double height = interpolate(1, y_calc, x, dem);
      assert(height >= 0);

      double z = zenith(viewpoint.y, viewpoint.x, dem[viewpoint.y][viewpoint.x],
                        y_calc, x, height);
      cout << "z: " << z << endl;
      // 90 degrees
      assert(z >= -PI / 2 && z <= PI / 2);

      cout << "zenith between viewpoint and vertical intersection: " << z
           << " main_zenith: " << main_zenith << endl;
      if (z > main_zenith) {
        return 0;
      }
    }
  }

  int dir_y = target.y - viewpoint.y > 0 ? 1 : -1;
  // Horizontal intersections
  // Start one away in the direction of target, and move towards it
  cout << "x_calcs" << endl;
  // TODO: need to immediately get rid of this if!!!!!
  if (viewpoint.y == target.y) {
    cout << "Horizontal line, skipping horizontal intersections" << endl;
  } else {
    for (int y = viewpoint.y + dir_y; y != target.y; y += dir_y) {
      // x is ray horizontal intersection point x-value
      double x_calc =
          static_cast<double>(y - viewpoint.y) / tan(azimuth) + viewpoint.x;
      cout << "calculated x: (" << y << ", " << x_calc
           << ") for azimuth: " << azimuth << endl;
      double height = interpolate(0, x_calc, y, dem);
      assert(height >= 0);

      double z = zenith(viewpoint.y, viewpoint.x, dem[viewpoint.y][viewpoint.x],
                        y, x_calc, height);
      cout << "z: " << z << endl;
      // 90 degrees
      assert(z >= -PI / 2 && z <= PI / 2);

      cout << "zenith between viewpoint and horizontal intersection: " << z
           << " main_zenith: " << main_zenith << endl;
      if (z > main_zenith) {
        return 0;
      }
    }
  }

  return 1;

  /* TODO: replace this with the real test. */
  (void)vz;
  (void)pz;
  return VISIBLE;
}

int compute_viewshed_grid(Grid* elev_grid, Grid* vis_grid, int vr, int vc,
                          float vh) {
  int r, c, count = 0;
  for (r = 0; r < elev_grid->nrows; r++) {
    for (c = 0; c < elev_grid->ncols; c++) {
      if (grid_get_nodata(elev_grid, r, c)) {
        grid_set(vis_grid, r, c, NOT_VISIBLE);
      } else if (is_point_visible(elev_grid, r, c, vr, vc, vh) == VISIBLE) {
        grid_set(vis_grid, r, c, VISIBLE);
        count++;
      } else {
        grid_set(vis_grid, r, c, NOT_VISIBLE);
      }
    }
  }
  return count;
}