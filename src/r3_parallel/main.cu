// Rafael Almeida
//
// R3 viewshed, single file. Grid code, hillshade and bitmap code are from
// Laura Toma's starter code; pixel buffer is from Caspian Ahlberg (MIT).
//
// build: g++ -m64 -Winline -g -O3 -Wall -o vis viewshed.cpp -lm
// run:   ./vis -e <elev.asc> -v <vis.asc> -r <vr> -c <vc> [-h <vh>] [-d <level>]

#include <assert.h>
#include <float.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

using namespace std;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

constexpr float PI = 3.141592653589793;

#define VISIBLE 1
#define NOT_VISIBLE 0

/* ************************************************************ */
/*  GRID */
/* ************************************************************ */

typedef struct _grid {
  unsigned int ncols;
  unsigned int nrows;
  float* data;

  float xllcorner;
  float yllcorner;
  float cellsize;
  float nodata_value;

  float min_value;
  float max_value;

  int nb_nodata_values;
} Grid;

static inline int inside_grid(const Grid* grid, int r, int c) {
  return r >= 0 && r < grid->nrows && c >= 0 && c < grid->ncols;
}

static inline float grid_get(const Grid* grid, int r, int c) {
  assert(inside_grid(grid, r, c));
  return grid->data[r * grid->ncols + c];
}

static inline bool grid_is_nodata(const Grid* grid, int r, int c) {
  assert(inside_grid(grid, r, c));
  return grid->nodata_value == grid_get(grid, r, c);
}

static inline void grid_set(Grid* grid, int r, int c, const float val) {
  assert(inside_grid(grid, r, c));
  grid->data[r * grid->ncols + c] = val;
  if (val != grid->nodata_value) {
    grid->min_value = fmin(val, grid->min_value);
    grid->max_value = fmax(val, grid->max_value);
  } else {
    grid->nb_nodata_values++;
  }
}

void grid_print_stats(const Grid* g, const char* name) {
  printf("grid %s: \n\tn=%ld [rows=%d,cols=%d], range=[%.2f, %.2f], "
         "NODATA values=%d (%.2f percent)\n",
         name, (long int)g->nrows * (long int)g->ncols, g->nrows, g->ncols,
         g->min_value, g->max_value, g->nb_nodata_values,
         100.0 * ((double)g->nb_nodata_values) / ((double)g->nrows * g->ncols));
}

// Returns an empty grid object
Grid* grid_init() {
  Grid* grid = (Grid*)malloc(sizeof(Grid));
  assert(grid);
  grid->data = NULL;
  grid->min_value = (float)INT_MAX;
  grid->max_value = (float)-INT_MAX;
  grid->nb_nodata_values = 0;
  return grid;
}

// allocate space for the grid data.
void grid_malloc_data(Grid* grid) {
  if (!grid->data) {
    grid->data = (float*)malloc(grid->nrows * grid->ncols * sizeof(float));
    assert(grid->data);
  }
}

// read from file
Grid* grid_read_from_file(const char* filename) {
  FILE* in_file = fopen(filename, "r");
  if (!in_file) {
    printf("cannot open file %s\n", filename);
    exit(1);
  }

  Grid* grid = grid_init();

  // header. Checked with if instead of assert so it still runs with -DNDEBUG
  if (fscanf(in_file, "ncols %d\n", &grid->ncols) != 1 ||
      fscanf(in_file, "nrows %d\n", &grid->nrows) != 1 ||
      fscanf(in_file, "xllcorner %f\n", &grid->xllcorner) != 1 ||
      fscanf(in_file, "yllcorner %f\n", &grid->yllcorner) != 1 ||
      fscanf(in_file, "cellsize %f\n", &grid->cellsize) != 1 ||
      fscanf(in_file, "NODATA_value %f\n", &grid->nodata_value) != 1) {
    printf("bad header in %s\n", filename);
    exit(1);
  }

  grid_malloc_data(grid);

  float val;
  for (int r = 0; r < grid->nrows; r++) {
    for (int c = 0; c < grid->ncols; c++) {
      if (fscanf(in_file, "%f ", &val) != 1) {
        printf("bad data in %s at (%d, %d)\n", filename, r, c);
        exit(1);
      }
      grid_set(grid, r, c, val);
    }
  }

  fclose(in_file);
  return grid;
}

// Write the complete asc file for a grid.
void grid_write_to_file(const char* filename, const Grid* grid) {
  FILE* out_file = fopen(filename, "w+");
  if (!out_file) {
    printf("cannot open file %s\n", filename);
    exit(1);
  }

  fprintf(out_file, "ncols %d\n", grid->ncols);
  fprintf(out_file, "nrows %d\n", grid->nrows);
  fprintf(out_file, "xllcorner %f\n", grid->xllcorner);
  fprintf(out_file, "yllcorner %f\n", grid->yllcorner);
  fprintf(out_file, "cellsize %f\n", grid->cellsize);
  fprintf(out_file, "NODATA_value %f\n", grid->nodata_value);

  for (int r = 0; r < grid->nrows; r++) {
    for (int c = 0; c < grid->ncols; c++) {
      fprintf(out_file, "%f ", grid_get(grid, r, c));
    }
    fprintf(out_file, "\n");
  }
  fclose(out_file);
}

// Initialize a grid based on an existing grid, copying its header.
Grid* grid_init_from(const Grid* grid) {
  Grid* new_grid = grid_init();
  new_grid->ncols = grid->ncols;
  new_grid->nrows = grid->nrows;
  new_grid->xllcorner = grid->xllcorner;
  new_grid->yllcorner = grid->yllcorner;
  new_grid->cellsize = grid->cellsize;
  new_grid->nodata_value = grid->nodata_value;
  grid_malloc_data(new_grid);
  return new_grid;
}

// Free a grid and its associated malloced data;
void grid_free(Grid* grid) {
  free(grid->data);
  free(grid);
}

/* ************************************************************ */
/*  VIEWSHED (R3) */
/* ************************************************************ */

// Parameters aren't of type Point to keep functions generalizable
float distance(float first_y, float first_x, float second_y, float second_x) {
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
  assert(coordinate >= 0);
  int horizontal_size = interpolate_y == 0 ? dem->nrows : dem->ncols;
  assert(coordinate < horizontal_size);

  assert(value >= 0);
  int v_below = static_cast<int>(value);
  int v_above = v_below + 1;
  assert(v_below >= 0);
  assert(v_above >= 0);
  int vertical_size = interpolate_y == 1 ? dem->nrows : dem->ncols;
  assert(v_below < vertical_size);
  assert(v_above < vertical_size);

  float t = value - v_below;
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

  if (grid_is_nodata(dem, y_below, x_below) ||
      grid_is_nodata(dem, y_above, x_above)) {
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

/* elev_grid: input elevation grid
   r, c:      the point being tested
   vr, vc:    viewpoint row and column
   vh:        viewpoint height ABOVE the terrain at (vr,vc)
   returns VISIBLE if (r,c) is visible from the viewpoint, NOT_VISIBLE otherwise
 */
int is_point_visible(const Grid* elev_grid, int r, int c, int vr, int vc,
                     float vh) {

  if (grid_is_nodata(elev_grid, r, c)) {
    return elev_grid->nodata_value;
  }

  if (vr == r && vc == c) {
    return 1;
  }

  float vz = grid_get(elev_grid, vr, vc) + vh;
  float pz = grid_get(elev_grid, r, c);

  // altitude from viewpoint to target
  // phi in math convention
  float los_altitude = altitude(vr, vc, vz, r, c, pz);
  // 90 degrees
  assert(los_altitude >= -PI / 2 && los_altitude <= PI / 2);

  // horizontal angle from viewpoint to target
  float dy = r - vr;
  float dx = c - vc;
  float slope = dy / dx;

  if (DEBUG_LEVEL == 1) {
    cout << "viewpoint: (" << vr << ", " << vc << ") target: (" << r << ", "
         << c << ") slope: " << slope << " los_altitude: " << los_altitude
         << endl;
  }

  // Vertical intersections
  int dir_x = dx > 0 ? 1 : -1;
  // Start one away in the direction of target, and move towards it
  if (DEBUG_LEVEL == 1) {
    cout << "y_calcs" << endl;
  }
  for (int x = vc; x != c; x += dir_x) {
    // Skip first iteration, could literally just do
    // x = vc + dir_x, but that leads to bugs when x overshoots c since I am
    // checking x != c instead of x < c or x > c
    if (x == vc) {
      continue;
    }
    // y is ray vertical intersection point y-value
    float y_calc = vr + static_cast<float>(x - vc) * slope;
    float height = interpolate(1, y_calc, x, elev_grid);
    if (height == elev_grid->nodata_value) {
      return elev_grid->nodata_value;
    }

    float z = altitude(vr, vc, vz, y_calc, x, height);
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

  int dir_y = r - vr > 0 ? 1 : -1;
  // Horizontal intersections
  if (DEBUG_LEVEL == 1) {
    cout << "x_calcs" << endl;
  }
  for (int y = vr; y != r; y += dir_y) {
    if (y == vr) {
      continue;
    }
    // x is ray horizontal intersection point x-value
    float x_calc = vc + static_cast<float>(y - vr) / slope;
    float height = interpolate(0, x_calc, y, elev_grid);
    if (height == elev_grid->nodata_value) {
      return elev_grid->nodata_value;
    }

    float z = altitude(vr, vc, vz, y, x_calc, height);
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

  return VISIBLE;
}

// compute viewshed of (r,c,h) and populate vis_grid with values
int compute_viewshed_grid(const Grid* elev_grid, Grid* vis_grid, int vr, int vc,
                          float vh) {

  assert(vr >= 0 && vr < elev_grid->nrows);
  assert(vc >= 0 && vc < elev_grid->ncols);
  // The viewpoint must be a valid point in the grid, not a nodata value
  assert(!grid_is_nodata(elev_grid, vr, vc));
  assert(vh >= 0);

  int r, c, count = 0;
  for (r = 0; r < elev_grid->nrows; r++) {
    for (c = 0; c < elev_grid->ncols; c++) {
      if (grid_is_nodata(elev_grid, r, c)) {
        grid_set(vis_grid, r, c, elev_grid->nodata_value);
        continue;
      }

      int is_visible = is_point_visible(elev_grid, r, c, vr, vc, vh);
      count += is_visible;
      grid_set(vis_grid, r, c, is_visible);
    }
  }
  return count;
}

/* ************************************************************ */
/*  PIXEL BUFFER (Caspian Ahlberg, MIT License) */
/* ************************************************************ */

typedef struct {
  u32* const pixels;
  const u16 width, height;
} PixelBuffer;

PixelBuffer init_pixel_buffer(const u16 width, const u16 height) {
  return PixelBuffer{(u32*)malloc(width * height * sizeof(u32)), width,
                     height};
}

void deinit_pixel_buffer(const PixelBuffer* const pb) { free(pb->pixels); }

void write_pixel_to_buffer(const PixelBuffer* const pb, const u16 x,
                           const u16 y, const u8 r, const u8 g, const u8 b) {
  static const u32 alpha_mask = 0xffu << 24u;
  const u32 pixel = alpha_mask | (u32)(b << 16u) | (u32)(g << 8u) | r;
  pb->pixels[y * pb->width + x] = pixel;
}

// Writes the pixel buffer as a 24-bit .bmp (replaces stb_image_write, since
// bmp is the only format we use)
void save_pixel_buffer_to_file(const PixelBuffer* const pb,
                               const char* const filename) {
  FILE* f = fopen(filename, "wb");
  if (!f) {
    fprintf(stderr, "cannot open file %s\n", filename);
    exit(1);
  }

  const u32 row_size = (3 * pb->width + 3) & ~3u; // rows padded to 4 bytes
  const u32 image_size = row_size * pb->height;
  const u32 file_size = 54 + image_size;

  u8 header[54] = {'B', 'M'};
  auto put32 = [&](int at, u32 v) {
    for (int i = 0; i < 4; i++)
      header[at + i] = (v >> (8 * i)) & 0xff;
  };
  put32(2, file_size);
  put32(10, 54);         // pixel data offset
  put32(14, 40);         // info header size
  put32(18, pb->width);
  put32(22, pb->height); // positive height = rows stored bottom-up
  header[26] = 1;        // color planes
  header[28] = 24;       // bits per pixel
  put32(34, image_size);
  fwrite(header, 1, sizeof(header), f);

  u8* row = (u8*)calloc(row_size, 1);
  for (int y = pb->height - 1; y >= 0; y--) {
    for (int x = 0; x < pb->width; x++) {
      const u32 pixel = pb->pixels[y * pb->width + x];
      row[3 * x + 0] = (pixel >> 16u) & 0xff; // b
      row[3 * x + 1] = (pixel >> 8u) & 0xff;  // g
      row[3 * x + 2] = pixel & 0xff;          // r
    }
    fwrite(row, 1, row_size, f);
  }
  free(row);
  fclose(f);
}

/* ************************************************************ */
/*  MAP: hillshade, viewshed bitmap, overlay (Laura Toma) */
/* ************************************************************ */

#define FLAT_ASPECT 0
#define Z_FACTOR 5

typedef float Color[3];

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

/*
a b c
d e f
g h i

if e is nodata :           return nodata
if any neighbor is nodata: return nodata
if (r,c) on the boundary:  return nodata

else return (c + 2f + i - a - 2d -g)/(8*cell)
 */
float dzdx(const Grid* grid, int r, int c) {

  assert(inside_grid(grid, r, c));
  if (grid_is_nodata(grid, r, c))
    return grid->nodata_value;

  if (r == 0 || r == grid->nrows - 1 || c == 0 || c == grid->ncols - 1)
    return grid->nodata_value;

  float res = 0;
  for (int l = -1; l <= 1; l++) {
    for (int j = -1; j <= 1; j++) {
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

/*
a b c
d e f
g h i

if e is nodata :           return nodata
if any neighbor is nodata: return nodata
if (r,c) on the boundary:  return nodata

else return  (g + 2h + i)- (a + 2b + c) )/(8*cell)

NOTE WE GO DY = SOUTH - NORTH TO GO CLOCKWISE, BECAUSE SUN AZIMUTH IS MEASURED
CLOCKWISE WITH 315 BEING NW if we do dy = north - south then we need to set sun
azimuth as 135 instead of 315
 */
float dzdy(const Grid* grid, int r, int c) {

  assert(inside_grid(grid, r, c));
  if (grid_is_nodata(grid, r, c))
    return grid->nodata_value;

  if (r == 0 || r == grid->nrows - 1 || c == 0 || c == grid->ncols - 1)
    return grid->nodata_value;

  float res = 0;
  for (int l = -1; l <= 1; l++) {
    for (int j = -1; j <= 1; j++) {
      if (l == 0 && j == 0)
        continue;

      if (grid_is_nodata(grid, r + l, c + j))
        return grid->nodata_value;

      if (l == 1 && j == -1)
        res += grid_get(grid, r + l, c + j); // g
      if (l == 1 && j == 0)
        res += 2 * grid_get(grid, r + l, c + j); // 2h
      if (l == 1 && j == 1)
        res += grid_get(grid, r + l, c + j); // i

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

float slope_rad(const Grid* grid, float dzdx, float dzdy) {
  if ((dzdx == grid->nodata_value) || (dzdy == grid->nodata_value))
    return grid->nodata_value;

  return atan(Z_FACTOR * sqrt(dzdx * dzdx + dzdy * dzdy));
}

// The direction the steepest downslope direction is facing is the
// aspect. Aspect in radians is defined in the range of 0 to 2pi, with
// 0 toward east.
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

// populate hillshade_grid with values 0..255
void generate_hillshade(const Grid* grid, Grid* hillshade_grid) {

  assert(grid && hillshade_grid);
  // sun zenith in radians relative to vertical
  float sun_zenith_rad = (90 - SUN_zenith_deg) * PI / 180;

  // sun azimuth in radians; change the azimuth angle from its
  // geographic unit (compass direction) to a mathematic unit (right
  // angle), then convert to radians.
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
        hillshade = cos(sun_zenith_rad) * cos(terrain_slope_rad) +
                    sin(sun_zenith_rad) * sin(terrain_slope_rad) *
                        cos(sun_azimuth_rad - terrain_aspect_rad);
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

// viewshed bitmap: visible green, invisible gray, viewpoint red
void grid_vis_to_pixelbuffer(const Grid* vis_grid, int vr, int vc,
                             PixelBuffer pb) {

  float rad = (float)VIEWPOINT_RADIUS * (float)vis_grid->nrows;
  printf("rad = %.1f\n", rad);

  float color[3];
  for (u16 y = 0; y < pb.height; y++) {
    for (u16 x = 0; x < pb.width; x++) {

      if (grid_is_nodata(vis_grid, y, x)) {
        for (int i = 0; i < 3; i++)
          color[i] = NODATA_COLOR[i];
      } else if (dist((u16)vr, (u16)vc, (u16)y, (u16)x) < rad) {
        for (int i = 0; i < 3; i++)
          color[i] = VIEWPOINT_COLOR[i];
      } else if (grid_get(vis_grid, y, x) == 1) {
        for (int i = 0; i < 3; i++)
          color[i] = VIS_COLOR[i];
      } else {
        for (int i = 0; i < 3; i++)
          color[i] = INVIS_COLOR[i];
      }

      // bring it from [0,1] to [0, 255]
      write_pixel_to_buffer(&pb, x, y, (u16)(color[0] * max_rgb_value),
                            (u16)(color[1] * max_rgb_value),
                            (u16)(color[2] * max_rgb_value));
    }
  }
}

// this grid contains values 0..255, put those in a pixelbuffer
void grid_to_pixelbuffer(const Grid* grid, PixelBuffer pb) {
  u16 rgb_color[3];
  for (u16 y = 0; y < pb.height; y++) {
    for (u16 x = 0; x < pb.width; x++) {

      float shade = grid_get(grid, y, x);
      if (grid_is_nodata(grid, y, x)) {
        for (int i = 0; i < 3; i++)
          rgb_color[i] = (u16)255 * NODATA_COLOR[i];
      } else {
        rgb_color[0] = (u16)shade;
        rgb_color[1] = (u16)shade;
        rgb_color[2] = (u16)shade;
      }
      write_pixel_to_buffer(&pb, x, y, rgb_color[0], rgb_color[1],
                            rgb_color[2]);
    }
  }
}

// overlay pb2 on top of pb1, with given transparency alpha. when
// alpha=1, show pb1; when alpha=0, show pb2. write the output in pb1
void pixelbuffer_overlay(PixelBuffer pb1, PixelBuffer pb2, float alpha) {

  assert(alpha >= 0 && alpha <= 1);
  assert(pb1.height == pb2.height && pb1.width == pb2.width);

  for (u16 y = 0; y < pb1.height; y++) {
    for (u16 x = 0; x < pb1.width; x++) {

      const u32 pixel1 = pb1.pixels[y * pb1.width + x];
      const u32 pixel2 = pb2.pixels[y * pb2.width + x];

      u16 color1[3], color2[3];
      u32 red_mask = 0xff;
      u32 green_mask = 0xff << 8u;
      u32 blue_mask = 0xff << 16u;
      color1[0] = pixel1 & red_mask;
      color1[1] = (pixel1 & green_mask) >> 8u;
      color1[2] = (pixel1 & blue_mask) >> 16u;

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

/* ************************************************************ */
/*  MAIN */
/* ************************************************************ */

//  Reads command line arguments
void parseArguments(int argc, char* const* argv, char** elev_name,
                    char** vis_name, int* vr, int* vc, float* vh,
                    int* debug_level) {

  const char usage[] =
      "Usage: %s -e <elev.asc> -v <vis.asc> -r <vr> -c <vc> [-h <vh>] [-d "
      "<level>]\n"
      "    -e, --elev        Required string. Elevation grid name.\n"
      "    -v, --vis         Required string. Vis grid name.\n"
      "    -r, --row         Required int. Viewpoint row\n"
      "    -c, --col         Required int. Viewpoint col\n"
      "    -h, --height      Optional int. Viewpoint height above ground\n"
      "    -d, --debug       Optional int. Debug level (0=off, 1=verbose)\n";

  // default initial values
  *elev_name = *vis_name = NULL;
  *vr = *vc = -1;
  *vh = 0;
  *debug_level = 0;

  int c;
  while ((c = getopt(argc, argv, "e:v:r:c:h:d:")) != -1) {
    switch (c) {
    case 'e':
      *elev_name = optarg; // elevation grid
      break;
    case 'v':
      *vis_name = optarg; // vis grid name
      break;
    case 'r':
      *vr = atoi(optarg); // viewpoint row
      break;
    case 'c':
      *vc = atoi(optarg); // viewpoint col
      break;
    case 'h':
      *vh = atof(optarg); // viewpoint height above ground
      break;
    case 'd':
      *debug_level = atoi(optarg); // debug level
      break;
    default:
      printf("Illegal option given\n");
      exit(2);
    }
  }

  const char* error = NULL;
  if (*elev_name == NULL)
    error = "Elev grid name cannot be NULL";
  else if (*vis_name == NULL)
    error = "Vis grid name cannot be NULL";
  else if (*vr < 0)
    error = "viewpoint row cannot be <0";
  else if (*vc < 0)
    error = "viewpoint col cannot be <0";
  else if (*debug_level < 0)
    error = "debug level cannot be <0";
  else if (*vh < 0)
    error = "viewpoint height cannot be <0";

  if (error) {
    printf("oops..%s\n", error);
    printf(usage, argv[0]);
    exit(1);
  }
  fflush(stdout);
}

int main(int argc, char** argv) {

  char *elev_name, *vis_name;
  int vr, vc;
  float vh;
  parseArguments(argc, argv, &elev_name, &vis_name, &vr, &vc, &vh,
                 &DEBUG_LEVEL);
  printf("running with arguments:\n\telev_name: %s \n\tvis_name: %s\n\tvp=(%d, "
         "%d, %.1f)\n\tdebug_level: %d\n",
         elev_name, vis_name, vr, vc, vh, DEBUG_LEVEL);
  fflush(stdout);

  // read elevation grid
  printf("reading %s\n", elev_name);
  Grid* elev_grid = grid_read_from_file(elev_name);
  grid_print_stats(elev_grid, elev_name);

  // compute viewshed grid
  Grid* vis_grid = grid_init_from(elev_grid);
  compute_viewshed_grid(elev_grid, vis_grid, vr, vc, vh);
  grid_print_stats(vis_grid, vis_name);

  // write viewshed grid
  printf("writing %s\n", vis_name);
  grid_write_to_file(vis_name, vis_grid);

  // CREATE BITMAPS
  const PixelBuffer pb = init_pixel_buffer(elev_grid->ncols, elev_grid->nrows);
  const PixelBuffer pb2 = init_pixel_buffer(elev_grid->ncols, elev_grid->nrows);

  // hillshade bitmap in pb
  Grid* hillshade_grid = grid_init_from(elev_grid);
  generate_hillshade(elev_grid, hillshade_grid);
  grid_to_pixelbuffer(hillshade_grid, pb);
  printf("writing map.hillshade.bmp\n");
  save_pixel_buffer_to_file(&pb, "map.hillshade.bmp");
  grid_free(hillshade_grid);

  // viewshed bitmap in pb2
  grid_vis_to_pixelbuffer(vis_grid, vr, vc, pb2);

  // viewshed overlayed on hillshade
  pixelbuffer_overlay(pb, pb2, .5);
  printf("writing map.viewshed-over-hillshade.bmp\n");
  save_pixel_buffer_to_file(&pb, "map.viewshed-over-hillshade.bmp");
  printf("done\n");
  fflush(stdout);

  grid_free(elev_grid);
  grid_free(vis_grid);
  deinit_pixel_buffer(&pb);
  deinit_pixel_buffer(&pb2);
}
