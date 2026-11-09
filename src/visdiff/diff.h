#ifndef __diff_h
#define __diff_h

#include "grid.h"
#include <stddef.h>
#include <stdint.h>

// A viewshed read from an .asc, 2 bits per cell, 4 cells per byte.
// low bit: visible or not; high bit: nodata
#define CELL_NOT_VISIBLE 0
#define CELL_VISIBLE 1
#define CELL_NODATA 2

typedef struct {
  unsigned int nrows;
  unsigned int ncols;
  uint8_t* data;
} Viewshed;

static inline uint8_t viewshed_get(const Viewshed* v, int r, int c) {
  // out of bounds would read the wrong cell
  assert(r >= 0 && r < (int)v->nrows && c >= 0 && c < (int)v->ncols);
  size_t i = (size_t)r * v->ncols + c;
  return (v->data[i >> 2] >> ((i & 3) * 2)) & 3;
}

// reads a viewshed .asc (values 0, 1 or nodata) and fills header with its
// header; exits with status 2 on any other value
Viewshed* viewshed_read(const char* filename, Grid* header);

void viewshed_free(Viewshed* v);

// How a cell compares between a primary and a secondary viewshed
typedef enum {
  DIFF_ADDED,           // 0 -> 1
  DIFF_REMOVED,         // 1 -> 0
  DIFF_NODATA_MISMATCH, // nodata in exactly one
  DIFF_SAME_VIS,        // 1 -> 1
  DIFF_SAME_INVIS,      // 0 -> 0
  DIFF_SAME_NODATA,     // nodata in both
  DIFF_NCLASSES
} DiffClass;

typedef struct {
  long n[DIFF_NCLASSES]; // cells per DiffClass
} DiffCounts;

DiffClass diff_class(uint8_t primary, uint8_t secondary);

// Classifies every cell and streams the diff bitmaps at full resolution.
// Either path may be NULL to skip that bitmap; overlay_path needs elev.
// alpha=1 shows the hillshade, alpha=0 the diff colors.
DiffCounts write_diff_bitmaps(const Viewshed* primary,
                              const Viewshed* secondary, const Grid* elev,
                              float alpha, const char* flat_path,
                              const char* overlay_path);

#endif
