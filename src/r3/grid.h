// Laura Toma
//
#ifndef __grid_h
#define __grid_h

#include <assert.h>
#include <math.h>
#include <stdio.h>

typedef struct _grid {
  int ncols;
  int nrows;
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
  // out of bounds would read the wrong cell or crash
  assert(inside_grid(grid, r, c));
  return grid->data[r * grid->ncols + c];
}

static inline bool grid_is_nodata(const Grid* grid, int r, int c) {
  // nodata cells are stored as NaN
  return isnan(grid_get(grid, r, c));
}

// print basic info
void grid_print_stats(const Grid* g, char* name);

// read  from file
Grid* grid_read(FILE* in_file);

// read from file
Grid* grid_read_from_file(char* filename);

// free space
void grid_free(Grid* grid);

// malloc for the big grids. Built with -DHUGEPAGES (all make targets), it asks
// for 2 MB pages instead of 4 KB ones
void* big_alloc(size_t bytes);

#endif
