// Laura Toma
//
#include "map.h"
#include "grid.h"
#include "pixel_buffer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

float dist(u16 x1, u16 y1, u16 x2, u16 y2) {

  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
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

  assert(inside_grid(grid, r, c));
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

  assert(inside_grid(grid, r, c));
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
void generate_hillshade(const Grid* grid, Grid* hillshade_grid) {

  assert(grid && hillshade_grid);
  // for hillshading
  // sun zenith in radians relative to vertical
  float sun_zenith_rad = (90 - SUN_zenith_deg) * PI / 180;

  // sun azimuth in radians; change the azimuth angle from its
  // geographic unit (compass direction) to a mathematic unit (right
  // angle). The direction of the illumination source, azimuth, is
  // specified in degrees. The hillshade formula requires this angle to
  // be in units of radians. First, change the azimuth angle from its
  // geographic unit (compass direction) to a mathematic unit (right
  // angle). Next, convert the azimuth angle to radians.
  float sun_azimuth_math = 360 - SUN_azimuth_deg + 90;
  if (sun_azimuth_math > 360)
    sun_azimuth_math -= 360;
  float sun_azimuth_rad = sun_azimuth_math * PI / 180;

  printf("sun zenith rad = %.1f\n", sun_zenith_rad);
  printf("sun azimuth rad = %.1f\n", sun_azimuth_rad);

  for (int i = 0; i < grid->nrows; i++) {
    for (int j = 0; j < grid->ncols; j++) {

      float dx = dzdx(grid, i, j);
      float dy = dzdy(grid, i, j);
      float terrain_slope_rad = slope_rad(grid, dx, dy);
      float terrain_aspect_rad = aspect_rad(grid, dx, dy);
      float hillshade;
      if ((terrain_slope_rad == grid->nodata_value) ||
          (terrain_aspect_rad == grid->nodata_value)) {
        hillshade = grid->nodata_value;
      } else {
        if (terrain_aspect_rad == FLAT_ASPECT)
          // terrain_aspect_rad = sun_azimuth_rad;
          // flat ==> cos (0) = 1, so no shading
          // this does not seem to make a difference
          terrain_aspect_rad = 0;

        // hillshade value at (i,j)
        hillshade = cos(sun_zenith_rad) * cos(terrain_slope_rad) +
                    sin(sun_zenith_rad) * sin(terrain_slope_rad) *
                        cos(sun_azimuth_rad - terrain_aspect_rad);

        // can this be bigger than 1?  no
        // if (hillshade > 255) assert(0);
        // printf("shade =%.1f ", hillshade);
        // can this be negative?  should not
        if (hillshade < 0)
          hillshade = 0;

        hillshade *= 255.0;
      }
      grid_set(hillshade_grid, i, j, hillshade);
    }
  }
  printf("some stats:\n");
  printf("\thillshaderange: [%.1f, %.1f]\n", hillshade_grid->min_value,
         hillshade_grid->max_value);
}

///////////////////////////////////////////////////////////
// flow_grid: grid of flow accumulation  values
// nintervals: number of height and color intervals
// color gradient of blue. height intervals are not equal but logarithmic
void grid_vis_to_pixelbuffer(const Grid* vis_grid, int vr, int vc,
                             PixelBuffer pb) {

  float rad = (float)VIEWPOINT_RADIUS * (float)vis_grid->nrows;
  printf("rad = %.1f\n", rad);

  // write values in pb
  float color[3];
  for (u16 y = 0; y < pb.height; y++) {
    for (u16 x = 0; x < pb.width; x++) {

      if (grid_is_nodata(vis_grid, y, x)) {
        for (int i = 0; i < 3; i++)
          color[i] = NODATA_COLOR[i];
      } else if (dist((u16)vr, (u16)vc, (u16)y, (u16)x) < rad) {
        for (int i = 0; i < 3; i++)
          color[i] = VIEWPOINT_COLOR[i];
      } else {
        float vis = grid_get(vis_grid, y, x); // value of this pixel
        if (vis == 1) {
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

  assert(alpha >= 0 && alpha <= 1);

  // shoudl also assert that they have same size
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
