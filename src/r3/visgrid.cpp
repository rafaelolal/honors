#include "visgrid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VisGrid* visgrid_init(unsigned int nrows, unsigned int ncols) {
  VisGrid* g = (VisGrid*)malloc(sizeof(VisGrid));
  g->nrows = nrows;
  g->ncols = ncols;
  size_t bytes = ((size_t)nrows * ncols + 3) / 4;
  g->data = (uint8_t*)malloc(bytes);
  // big grids can run out of memory, which would crash on first access
  assert(g->data);
  // zero is NOT_VISIBLE; writing also touches the pages now, so page faults
  // aren't counted as viewshed time
  memset(g->data, 0, bytes);
  return g;
}

void visgrid_write_to_file(const char* filename, const VisGrid* g,
                           const Grid* elev) {
  FILE* f = fopen(filename, "w+");
  if (!f) {
    printf("cannot open file %s\n", filename);
    exit(1);
  }
  fprintf(f, "ncols %d\n", g->ncols);
  fprintf(f, "nrows %d\n", g->nrows);
  fprintf(f, "xllcorner %f\n", elev->xllcorner);
  fprintf(f, "yllcorner %f\n", elev->yllcorner);
  fprintf(f, "cellsize %f\n", elev->cellsize);
  fprintf(f, "NODATA_value %f\n", elev->nodata_value);

  // same text as the old float grid, so outputs stay byte-identical
  char nodata[64];
  snprintf(nodata, sizeof nodata, "%f ", elev->nodata_value);
  const char* text[4] = {"0.000000 ", "1.000000 ", nodata, nodata};
  for (unsigned int r = 0; r < g->nrows; r++) {
    for (unsigned int c = 0; c < g->ncols; c++) {
      fputs(text[visgrid_get(g, r, c)], f);
    }
    fputs("\n", f);
  }
  fclose(f);
}

void visgrid_free(VisGrid* g) {
  free(g->data);
  free(g);
}
