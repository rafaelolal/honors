// Copied from r3's map.cpp (Laura Toma), hillshade part only.
#include "hillshade.h"
#include <math.h>

// for hillshading
const float SUN_zenith_deg = 45;
const float SUN_azimuth_deg = 315;

////////////////////////////////////////////////////////////
/*
a b c
d e f
g h i

if e is nodata :           return nodata
if any neighbor is nodata: return nodata
if (r,c) on the byoundary: return nodata

else return (c + 2f + i - a - 2d -g)/(8*cell)

 */
float dzdx(const Grid* grid, int r, int c) {

  if (grid_is_nodata(grid, r, c))
    return grid->nodata_value;

  // if on the boundary, set it to 0
  if (r == 0 || r == grid->nrows - 1 || c == 0 || c == grid->ncols - 1)
    return grid->nodata_value;

  float res = 0;
  for (int l = -1; l <= 1; l++) {
    for (int j = -1; j <= 1; j++) {

      if (!inside_grid(grid, r + l, c + j))
        continue;
      if (l == 0 && j == 0)
        continue;

      if (grid_is_nodata(grid, r + l, c + j))
        return grid->nodata_value;

      if (l == -1 && j == -1)
        res -= grid_get(grid, r + l, c + j); // a
      if (l == 0 && j == -1)
        res -= 2 * grid_get(grid, r + l, c + j); // 2d
      if (l == 1 && j == -1)
        res -= grid_get(grid, r + l, c + j); // g

      if (l == -1 && j == 1)
        res += grid_get(grid, r + l, c + j); // c
      if (l == 0 && j == 1)
        res += 2 * grid_get(grid, r + l, c + j); // 2f
      if (l == 1 && j == 1)
        res += grid_get(grid, r + l, c + j); // i
    }
  }
  return res / (8 * grid->cellsize);
}

////////////////////////////////////////////////////////////
/*
a b c
d e f
g h i

if e is nodata :           return nodata
if any neighbor is nodata: return nodata
if (r,c) on the byoundary: return nodata

else return  (g + 2h + i)- (a + 2b + c) )/(8*cell)

NOTE WE GO DY = SOUTH - NORTH TO GO CLOCKWISE, BECAUSE  SUN AZIMUTH IS MEASURED
CLOCKWISE WITH 315 BEING NW if we do dy  = north - south then we need to set sun
azimuth as 135 instead of 315
 */
float dzdy(const Grid* grid, int r, int c) {

  if (grid_is_nodata(grid, r, c))
    return grid->nodata_value;

  // if on the boundary, set it to 0
  if (r == 0 || r == grid->nrows - 1 || c == 0 || c == grid->ncols - 1)
    return grid->nodata_value;

  float res = 0;
  for (int l = -1; l <= 1; l++) {
    for (int j = -1; j <= 1; j++) {
      if (!inside_grid(grid, r + l, c + j))
        continue;
      if (l == 0 && j == 0)
        continue;

      // if any neighbor is nodata, return 0
      if (grid_is_nodata(grid, r + l, c + j))
        return grid->nodata_value;

      if (l == 1 && j == -1)
        res += grid_get(grid, r + l, c + j); // g
      if (l == 1 && j == 0)
        res += 2 * grid_get(grid, r + l, c + j); // 2h
      if (l == 1 && j == 1)
        res += grid_get(grid, r + l, c + j); ////i

      if (l == -1 && j == -1)
        res -= grid_get(grid, r + l, c + j); // a
      if (l == -1 && j == 0)
        res -= 2 * grid_get(grid, r + l, c + j); // 2b
      if (l == -1 && j == 1)
        res -= grid_get(grid, r + l, c + j); // c
    }
  }
  return res / (8 * grid->cellsize);
}

////////////////////////////////////////////////////////////
float slope_rad(const Grid* grid, float dzdx, float dzdy) {

  if ((dzdx == grid->nodata_value) || (dzdy == grid->nodata_value))
    return grid->nodata_value;

  float slope = atan(Z_FACTOR * sqrt(dzdx * dzdx + dzdy * dzdy));
  // printf("%.1f ", slope);
  return slope;
}

////////////////////////////////////////////////////////////
// The direction the steepest downslope direction is facing is the
// aspect. Aspect in radians is defined in the range of 0 to 2pi, with
// 0 toward east. The aspect is determined under the rules in the
// following algorithm:
float aspect_rad(const Grid* grid, float dzdx, float dzdy) {

  if ((dzdx == grid->nodata_value) || (dzdy == grid->nodata_value))
    return grid->nodata_value;

  float aspect = 0;
  if (dzdx != 0) {
    aspect = atan2(dzdy, dzdx);
    if (aspect < 0)
      aspect += 2 * PI;
  } else {
    if (dzdy > 0)
      aspect = PI / 2;
    else if (dzdy < 0)
      aspect = 2 * PI - PI / 2;
    else
      aspect = FLAT_ASPECT; // slope zero, flat
  }
  return aspect;
}

////////////////////////////////////////////////////////////
// sun angles in radians, for hillshading
static float sun_zenith_rad() { return (90 - SUN_zenith_deg) * PI / 180; }

// the hillshade formula wants the azimuth as a math angle, not a compass one
static float sun_azimuth_rad() {
  float sun_azimuth_math = 360 - SUN_azimuth_deg + 90;
  if (sun_azimuth_math > 360)
    sun_azimuth_math -= 360;
  return sun_azimuth_math * PI / 180;
}

////////////////////////////////////////////////////////////
// hillshade of one cell in [0, 255], or the grid's nodata value
float hillshade_at(const Grid* grid, int r, int c) {
  static const float zenith = sun_zenith_rad();
  static const float azimuth = sun_azimuth_rad();

  float dx = dzdx(grid, r, c);
  float dy = dzdy(grid, r, c);
  float terrain_slope_rad = slope_rad(grid, dx, dy);
  float terrain_aspect_rad = aspect_rad(grid, dx, dy);
  if ((terrain_slope_rad == grid->nodata_value) ||
      (terrain_aspect_rad == grid->nodata_value))
    return grid->nodata_value;

  // flat ==> cos (0) = 1, so no shading
  if (terrain_aspect_rad == FLAT_ASPECT)
    terrain_aspect_rad = 0;

  float hillshade = cos(zenith) * cos(terrain_slope_rad) +
                    sin(zenith) * sin(terrain_slope_rad) *
                        cos(azimuth - terrain_aspect_rad);
  // should not be negative
  if (hillshade < 0)
    hillshade = 0;

  return hillshade * 255.0;
}

