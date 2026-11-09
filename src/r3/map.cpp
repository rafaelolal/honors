// Laura Toma
//
#include "map.h"
#include "bmp.h"
#include "grid.h"
#include "pixel_buffer.h"
#include <vector>

#include <stdint.h>
#include <stdio.h>

// used to compare float values
#define EPSILON .001

Color NODATA_COLOR = {0, 0, 1}; // blue

// for hillshading
const float SUN_zenith_deg = 45;
const float SUN_azimuth_deg = 315;

// for viewshed
Color VIS_COLOR = {0, 1, 0};      // green
Color INVIS_COLOR = {.6, .6, .6}; // gray
float VIEWPOINT_RADIUS = .01;
Color VIEWPOINT_COLOR = {1, .2, .3};

static const u8 max_rgb_value = 0xffu;

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
void generate_hillshade(const Grid* grid, Grid* hillshade_grid) {
  printf("sun zenith rad = %.1f\n", sun_zenith_rad());
  printf("sun azimuth rad = %.1f\n", sun_azimuth_rad());

  for (int i = 0; i < grid->nrows; i++)
    for (int j = 0; j < grid->ncols; j++)
      grid_set(hillshade_grid, i, j, hillshade_at(grid, i, j));

  printf("some stats:\n");
  printf("\thillshaderange: [%.1f, %.1f]\n", hillshade_grid->min_value,
         hillshade_grid->max_value);
}

////////////////////////////////////////////////////////////
// color in [0,1] to a byte, truncating like the old pixel buffer code
static u8 to_byte(float v) { return (u8)(u16)(v * max_rgb_value); }

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
  std::vector<u8> hrow(3 * (size_t)w), vrow(3 * (size_t)w), orow(3 * (size_t)w);

  // BMP stores rows bottom-up
  for (int r = h - 1; r >= 0; r--) {
    for (int c = 0; c < w; c++) {
      u8* hp = &hrow[3 * (size_t)c];
      u8* vp = &vrow[3 * (size_t)c];
      u8* op = &orow[3 * (size_t)c];

      float shade = hillshade_at(elev, r, c);
      for (int i = 0; i < 3; i++)
        hp[i] = shade == elev->nodata_value ? to_byte(NODATA_COLOR[i])
                                            : (u8)(u16)shade;

      uint8_t v = visgrid_get(vis, r, c);
      const float* color = (v & VIS_NODATA)           ? NODATA_COLOR
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

///////////////////////////////////////////////////////////
// flow_grid: grid of flow accumulation  values
// nintervals: number of height and color intervals
// color gradient of blue. height intervals are not equal but logarithmic
void grid_vis_to_pixelbuffer(const VisGrid* vis_grid, int vr, int vc,
                             PixelBuffer pb) {

  float rad = (float)VIEWPOINT_RADIUS * (float)vis_grid->nrows;
  printf("rad = %.1f\n", rad);

  // write values in pb
  float color[3];
  for (u16 y = 0; y < pb.height; y++) {
    for (u16 x = 0; x < pb.width; x++) {

      uint8_t vis = visgrid_get(vis_grid, y, x);
      if (vis & VIS_NODATA) {
        for (int i = 0; i < 3; i++)
          color[i] = NODATA_COLOR[i];
      } else if (dist((u16)vr, (u16)vc, (u16)y, (u16)x) < rad) {
        for (int i = 0; i < 3; i++)
          color[i] = VIEWPOINT_COLOR[i];
      } else {
        if (vis == VISIBLE) {
          // visible
          for (int i = 0; i < 3; i++)
            color[i] = VIS_COLOR[i];
        } else {
          // invisible
          for (int i = 0; i < 3; i++)
            color[i] = INVIS_COLOR[i];
        }
      }

      // bring it from [0,1] to [0, 255]
      u16 rgb_color[3] = {(u16)(color[0] * max_rgb_value),
                          (u16)(color[1] * max_rgb_value),
                          (u16)(color[2] * max_rgb_value)};

      write_pixel_to_buffer(&pb, x, y, rgb_color[0], rgb_color[1],
                            rgb_color[2]);
    }
  }
}

////////////////////////////////////////////////////////////
// this grid contains values 0..255, put  those in a pixelbuffer
void grid_to_pixelbuffer(const Grid* grid, PixelBuffer pb) {
  // write values in pb
  u16 rgb_color[3];
  for (u16 y = 0; y < pb.height; y++) {
    for (u16 x = 0; x < pb.width; x++) {

      float shade = grid_get(grid, y, x); // shade  of this pixel
      if (grid_is_nodata(grid, y, x)) {
        // set this pixel to nodata color
        for (int i = 0; i < 3; i++)
          rgb_color[i] = (u16)255 * NODATA_COLOR[i];
      } else {
        // set this pixel to its value
        rgb_color[0] = (u16)shade;
        rgb_color[1] = (u16)shade;
        rgb_color[2] = (u16)shade;
      }
      write_pixel_to_buffer(&pb, x, y, rgb_color[0], rgb_color[1],
                            rgb_color[2]);

    } // for x
  } // for y
}

////////////////////////////////////////////////////////////
// overlay pb2 on top of pb1, with given transparency alpha. when
// alpha=1, show pb1; when alpha=0, show pb2. write the output in pb1
void pixelbuffer_overlay(PixelBuffer pb1, PixelBuffer pb2, float alpha) {

  // outside this range, the blended colors overflow into wrong values
  assert(alpha >= 0 && alpha <= 1);

  // both buffers are read with pb1's loop bounds, so a smaller pb2 reads past
  // its end
  assert(pb1.height == pb2.height && pb1.width == pb2.width);

  for (u16 y = 0; y < pb1.height; y++) {
    for (u16 x = 0; x < pb1.width; x++) {

      const u32 pixel1 = pb1.pixels[y * pb1.width + x];
      const u32 pixel2 = pb2.pixels[y * pb2.width + x];

      // get colors
      // static const u32 alpha_mask = 0xffu << 24u;
      // const u32 pixel = alpha_mask | (u32) (b << 16u) | (u32) (g << 8u) | r;
      u16 color1[3], color2[3];
      u32 red_mask = 0xff;
      u32 green_mask = 0xff << 8u;
      u32 blue_mask = 0xff << 16u;
      color1[0] = pixel1 & red_mask;
      color1[1] = (pixel1 & green_mask) >> 8u;
      color1[2] = (pixel1 & blue_mask) >> 16u;
      // printf("red mask = %ux green mask = %ux blue mask = %ux\n", red_mask,
      // green_mask, blue_mask); printf("pixel1=%x, r=%x g=%x b=%x\n", pixel1,
      // color1[0], color1[1], color1[2] );

      color2[0] = pixel2 & red_mask;
      color2[1] = (pixel2 & green_mask) >> 8u;
      color2[2] = (pixel2 & blue_mask) >> 16u;

      u16 rgb_color[3];
      for (int i = 0; i < 3; i++) {
        rgb_color[i] =
            color2[i] + (u16)(alpha * (float)(color1[i] - color2[i]));
      }

      write_pixel_to_buffer(&pb1, x, y, rgb_color[0], rgb_color[1],
                            rgb_color[2]);
    }
  }
}
