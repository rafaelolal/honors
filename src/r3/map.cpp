// Laura Toma
//
#include "map.h"
#include "bmp.h"
#include "grid.h"
#include <vector>

#include <math.h>
#include <stdint.h>
#include <stdio.h>

Color NODATA_COLOR = {0, 0, 1}; // blue

// for hillshading
const float SUN_zenith_deg = 45;
const float SUN_azimuth_deg = 315;

// for viewshed
Color VIS_COLOR = {0, 1, 0};      // green
Color INVIS_COLOR = {.6, .6, .6}; // gray
float VIEWPOINT_RADIUS = .01;
Color VIEWPOINT_COLOR = {1, .2, .3};

static const uint8_t max_rgb_value = 0xffu;

float dist(int x1, int y1, int x2, int y2) {
  // 64-bit: on big grids like Knox, the sum of squares overflows an int
  int64_t dx = (int64_t)x1 - x2, dy = (int64_t)y1 - y2;
  return sqrt((double)(dx * dx + dy * dy));
}

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

  float hillshade =
      cos(zenith) * cos(terrain_slope_rad) +
      sin(zenith) * sin(terrain_slope_rad) * cos(azimuth - terrain_aspect_rad);
  // should not be negative
  if (hillshade < 0)
    hillshade = 0;

  return hillshade * 255.0;
}

////////////////////////////////////////////////////////////
// color in [0,1] to a byte, truncating like the old pixel buffer code
static uint8_t to_byte(float v) {
  return (uint8_t)(uint16_t)(v * max_rgb_value);
}

void write_bitmaps(const Grid* elev, const VisGrid* vis, int vr, int vc,
                   float alpha, const char* hillshade_path,
                   const char* viewshed_path, const char* overlay_path) {
  // outside this range, the blended colors overflow into wrong values
  assert(alpha >= 0 && alpha <= 1);
  // the viewshed is computed on the elevation grid, so they line up
  assert(vis->nrows == elev->nrows && vis->ncols == elev->ncols);

  const int w = elev->ncols, h = elev->nrows;
  float rad = (float)VIEWPOINT_RADIUS * (float)h;

  BmpWriter hb, vb, ob;
  bmp_open(&hb, hillshade_path, w, h);
  bmp_open(&vb, viewshed_path, w, h);
  bmp_open(&ob, overlay_path, w, h);
  std::vector<uint8_t> hrow(3 * (size_t)w), vrow(3 * (size_t)w), orow(3 * (size_t)w);

  // BMP stores rows bottom-up
  for (int r = h - 1; r >= 0; r--) {
    for (int c = 0; c < w; c++) {
      uint8_t* hp = &hrow[3 * (size_t)c];
      uint8_t* vp = &vrow[3 * (size_t)c];
      uint8_t* op = &orow[3 * (size_t)c];

      float shade = hillshade_at(elev, r, c);
      for (int i = 0; i < 3; i++)
        hp[i] = shade == elev->nodata_value ? to_byte(NODATA_COLOR[i])
                                            : (uint8_t)(uint16_t)shade;

      uint8_t v = visgrid_cell(vis, elev, r, c);
      const float* color = v == VIS_NODATA           ? NODATA_COLOR
                           : dist(vr, vc, r, c) < rad ? VIEWPOINT_COLOR
                           : v == VISIBLE             ? VIS_COLOR
                                                      : INVIS_COLOR;
      for (int i = 0; i < 3; i++)
        vp[i] = to_byte(color[i]);

      // alpha=1 shows the hillshade, alpha=0 the viewshed
      for (int i = 0; i < 3; i++)
        op[i] = vp[i] + (int)(alpha * (float)(hp[i] - vp[i]));
    }
    bmp_write_row(&hb, hrow.data());
    bmp_write_row(&vb, vrow.data());
    bmp_write_row(&ob, orow.data());
  }
  bmp_close(&hb);
  bmp_close(&vb);
  bmp_close(&ob);
}
