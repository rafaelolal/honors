#ifndef __visgrid_h
#define __visgrid_h

#include "grid.h"
#include <stddef.h>
#include <stdint.h>

// Viewshed grid, 2 bits per cell, 4 cells per byte.
// low bit: visible or not; high bit: nodata
#define NOT_VISIBLE 0
#define VISIBLE 1
#define VIS_NODATA 2

typedef struct {
  unsigned int nrows;
  unsigned int ncols;
  uint8_t* data;
} VisGrid;

static inline size_t visgrid_index(const VisGrid* g, int r, int c) {
  // out of bounds would read or corrupt the wrong cell
  assert(r >= 0 && r < (int)g->nrows && c >= 0 && c < (int)g->ncols);
  return (size_t)r * g->ncols + c;
}

static inline uint8_t visgrid_get(const VisGrid* g, int r, int c) {
  size_t i = visgrid_index(g, r, c);
  return (g->data[i >> 2] >> ((i & 3) * 2)) & 3;
}

// Not thread safe: the 4 cells in a byte share it, so parallel writers need
// atomics or work split on byte boundaries.
static inline void visgrid_set(VisGrid* g, int r, int c, uint8_t v) {
  // a third bit would spill into the neighbouring cell
  assert(v <= 3);
  size_t i = visgrid_index(g, r, c);
  int shift = (i & 3) * 2;
  g->data[i >> 2] = (g->data[i >> 2] & ~(3u << shift)) | (v << shift);
}

// all cells start NOT_VISIBLE
VisGrid* visgrid_init(unsigned int nrows, unsigned int ncols);

// header and nodata value come from elev, so the .asc matches the DEM
void visgrid_write_to_file(const char* filename, const VisGrid* g,
                           const Grid* elev);

void visgrid_free(VisGrid* g);

#endif
