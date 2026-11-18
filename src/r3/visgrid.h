#ifndef __visgrid_h
#define __visgrid_h

#include "grid.h"
#include <stddef.h>
#include <stdint.h>

// Viewshed grid, 1 bit per cell, 8 per byte: visible or not. Nodata isn't
// stored; writers take it from the elevation grid.

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

// bytes needed for nrows * ncols cells
static inline size_t visgrid_bytes(unsigned int nrows, unsigned int ncols) {
  return ((size_t)nrows * ncols + 7) / 8;
}

// NOT_VISIBLE or VISIBLE
static inline uint8_t visgrid_get(const VisGrid* g, int r, int c) {
  size_t i = visgrid_index(g, r, c);
  return (g->data[i >> 3] >> (i & 7)) & 1;
}

// Each cell must be set at most once, and only from NOT_VISIBLE.
// Not thread safe: cells share a byte, so parallel writers need atomics or
// work split on byte boundaries.
static inline void visgrid_set(VisGrid* g, int r, int c, uint8_t v) {
  assert(v <= VIS_NODATA);
  size_t i = visgrid_index(g, r, c);
  // VIS_NODATA has no bit, so it stays NOT_VISIBLE
  g->data[i >> 3] |= (uint8_t)((v == VISIBLE) << (i & 7));
}

// What a writer should show for a cell: VIS_NODATA wherever the DEM has no
// data, which is the only way 1 bit can report it.
static inline uint8_t visgrid_cell(const VisGrid* g, const Grid* elev, int r,
                                   int c) {
  return grid_is_nodata(elev, r, c) ? VIS_NODATA : visgrid_get(g, r, c);
}

// all cells start NOT_VISIBLE
VisGrid* visgrid_init(unsigned int nrows, unsigned int ncols);

// header and nodata value come from elev, so the .asc matches the DEM
void visgrid_write_to_file(const char* filename, const VisGrid* g,
                           const Grid* elev);

void visgrid_free(VisGrid* g);

#endif
