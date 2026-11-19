#include "diff.h"
#include "bmp.h"
#include "hillshade.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

typedef float Color[3];

// indexed by DiffClass
static const Color DIFF_COLORS[] = {
    {0, 1, 0},    // DIFF_FP: green
    {1, 0, 0},    // DIFF_FN: red
    {1, 1, 0},    // DIFF_NODATA_MISMATCH: yellow
    {.6, .6, .6}, // DIFF_TP: gray
    {0, 0, 0},    // DIFF_TN: black
    {0, 0, 1},    // DIFF_SAME_NODATA: blue
};

// hillshade nodata, same blue as vis uses
static const Color NODATA_COLOR = {0, 0, 1};

// color in [0,1] to a byte, truncating like vis does
static uint8_t to_byte(float v) { return (uint8_t)(uint16_t)(v * 255); }

////////////////////////////////////////////////////////////
static void viewshed_set(Viewshed* v, size_t i, uint8_t code) {
  int shift = (i & 3) * 2;
  v->data[i >> 2] = (v->data[i >> 2] & ~(3u << shift)) | (code << shift);
}

Viewshed* viewshed_read(const char* filename, Grid* header) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    printf("cannot open file %s\n", filename);
    exit(2);
  }
  grid_read_header(f, header);

  Viewshed* v = (Viewshed*)malloc(sizeof(Viewshed));
  v->nrows = header->nrows;
  v->ncols = header->ncols;
  size_t cells = (size_t)v->nrows * v->ncols;
  // zero is CELL_NOT_VISIBLE
  v->data = (uint8_t*)calloc((cells + 3) / 4, 1);
  // big grids can run out of memory, which would crash on first access
  assert(v->data);

  float x;
  for (size_t i = 0; i < cells; i++) {
    if (fscanf(f, "%f ", &x) != 1) {
      printf("%s: bad grid data at (%zu, %zu)\n", filename, i / v->ncols,
             i % v->ncols);
      exit(2);
    }
    // numeric compare, so GDAL's "1" and vis's "1.000000" both match
    if (x == header->nodata_value) {
      viewshed_set(v, i, CELL_NODATA);
    } else if (x == 1) {
      viewshed_set(v, i, CELL_VISIBLE);
    } else if (x != 0) {
      printf("%s: not a viewshed grid, value %f at (%zu, %zu)\n", filename, x,
             i / v->ncols, i % v->ncols);
      exit(2);
    }
  }
  fclose(f);
  return v;
}

void viewshed_free(Viewshed* v) {
  free(v->data);
  free(v);
}

////////////////////////////////////////////////////////////
DiffClass diff_class(uint8_t primary, uint8_t secondary) {
  bool pn = primary & CELL_NODATA, sn = secondary & CELL_NODATA;
  if (pn && sn)
    return DIFF_SAME_NODATA;
  if (pn || sn)
    return DIFF_NODATA_MISMATCH;
  if (primary == secondary)
    return primary == CELL_VISIBLE ? DIFF_TP : DIFF_TN;
  return secondary == CELL_VISIBLE ? DIFF_FP : DIFF_FN;
}

DiffCounts write_diff_bitmaps(const Viewshed* primary,
                              const Viewshed* secondary, const Grid* elev,
                              float alpha, const char* flat_path,
                              const char* overlay_path) {
  // outside this range, the blended colors overflow into wrong values
  assert(alpha >= 0 && alpha <= 1);
  // cells are compared by position, so the grids must line up
  assert(primary->nrows == secondary->nrows &&
         primary->ncols == secondary->ncols);
  // the overlay needs a hillshade to blend with
  assert(!overlay_path || elev);
  assert(!elev ||
         (elev->nrows == primary->nrows && elev->ncols == primary->ncols));

  const int w = primary->ncols, h = primary->nrows;
  DiffCounts counts = {};

  BmpWriter fb, ob;
  if (flat_path)
    bmp_open(&fb, flat_path, w, h);
  if (overlay_path)
    bmp_open(&ob, overlay_path, w, h);
  std::vector<uint8_t> frow(3 * (size_t)w), orow(3 * (size_t)w);

  // BMP stores rows bottom-up
  for (int r = h - 1; r >= 0; r--) {
    for (int c = 0; c < w; c++) {
      DiffClass d = diff_class(viewshed_get(primary, r, c),
                               viewshed_get(secondary, r, c));
      counts.n[d]++;

      uint8_t* fp = &frow[3 * (size_t)c];
      for (int i = 0; i < 3; i++)
        fp[i] = to_byte(DIFF_COLORS[d][i]);

      if (overlay_path) {
        float shade = hillshade_at(elev, r, c);
        uint8_t* op = &orow[3 * (size_t)c];
        for (int i = 0; i < 3; i++) {
          uint8_t hp = shade == elev->nodata_value ? to_byte(NODATA_COLOR[i])
                                                   : (uint8_t)(uint16_t)shade;
          op[i] = fp[i] + (int)(alpha * (float)(hp - fp[i]));
        }
      }
    }
    if (flat_path)
      bmp_write_row(&fb, frow.data());
    if (overlay_path)
      bmp_write_row(&ob, orow.data());
  }
  if (flat_path)
    bmp_close(&fb);
  if (overlay_path)
    bmp_close(&ob);
  return counts;
}
