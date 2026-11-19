// Laura Toma
//
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "grid.h"

/* ************************************************************ */
void grid_print_stats(const Grid* g, char* name) {

  printf("grid %s: \n\tn=%ld [rows=%d,cols=%d], range=[%.2f, %.2f], "
         "NODATA values=%d (%.2f percent)\n",
         name, (long int)g->nrows * (long int)g->ncols, g->nrows, g->ncols,
         g->min_value, g->max_value, g->nb_nodata_values,
         100.0 * ((double)g->nb_nodata_values) / ((double)g->nrows * g->ncols));
}

// Returns an empty grid object
Grid* grid_init() {
  Grid* grid;
  grid = (Grid*)malloc(sizeof(Grid));
  if (!grid) {
    printf("cannot allocate the grid struct\n");
    exit(1);
  }
  grid->data = NULL;
  grid->min_value = (float)INT_MAX;
  grid->max_value = (float)-INT_MAX;
  grid->nb_nodata_values = 0;
  return grid;
}

// grid must be allocated
void grid_read_header(FILE* in_file, Grid* grid) {

  if (fscanf(in_file, "ncols %d\n", &grid->ncols) != 1 ||
      fscanf(in_file, "nrows %d\n", &grid->nrows) != 1 ||
      fscanf(in_file, "xllcorner %f\n", &grid->xllcorner) != 1 ||
      fscanf(in_file, "yllcorner %f\n", &grid->yllcorner) != 1 ||
      fscanf(in_file, "cellsize %f\n", &grid->cellsize) != 1 ||
      fscanf(in_file, "NODATA_value %f\n", &grid->nodata_value) != 1 ||
      grid->nrows <= 0 || grid->ncols <= 0) {
    printf("bad grid header\n");
    exit(1);
  }
}

// allocate space for the grid data.
void grid_malloc_data(Grid* grid) {
  if (!grid->data) {
    grid->data = (float*)malloc(grid->nrows * grid->ncols * sizeof(float));
    // big grids can run out of memory, which would crash later on first access
    assert(grid->data);
  }
}

// min, max and nodata count, in one pass over the data
static void grid_compute_stats(Grid* grid) {
  size_t n = (size_t)grid->nrows * grid->ncols;
  for (size_t i = 0; i < n; i++) {
    float val = grid->data[i];
    if (val != grid->nodata_value) {
      grid->min_value = fmin(val, grid->min_value);
      grid->max_value = fmax(val, grid->max_value);
    } else {
      grid->nb_nodata_values++;
    }
  }
}

void grid_read_data(FILE* in_file, Grid* grid) {
  size_t n = (size_t)grid->nrows * grid->ncols;
  // raw writes; stats are computed once after the read
  for (size_t i = 0; i < n; i++) {
    if (fscanf(in_file, "%f ", &grid->data[i]) != 1) {
      printf("bad grid data at (%zu, %zu)\n", i / grid->ncols, i % grid->ncols);
      exit(1);
    }
  }
  grid_compute_stats(grid);
}

// Read grid from file and return it
Grid* grid_read(FILE* in_file) {
  Grid* grid = grid_init();
  grid_read_header(in_file, grid);
  grid_malloc_data(grid);
  grid_read_data(in_file, grid);
  return grid;
}

// read from file
Grid* grid_read_from_file(char* filename) {

  FILE* infile = fopen(filename, "r");
  if (!infile) {
    printf("cannot open file %s\n", filename);
    exit(1);
  }
  Grid* grid = grid_read(infile);
  fclose(infile);
  return grid;
}

// Free a grid and its associated malloced data;
void grid_free(Grid* grid) {
  free(grid->data);
  free(grid);
}
