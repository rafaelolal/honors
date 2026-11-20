#include "visgrid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VisGrid* visgrid_init(int nrows, int ncols) {
  VisGrid* g = (VisGrid*)malloc(sizeof(VisGrid));
  if (!g) {
    printf("cannot allocate the viewshed grid struct\n");
    exit(1);
  }
  g->nrows = nrows;
  g->ncols = ncols;
  size_t bytes = visgrid_bytes(nrows, ncols);
  g->data = (uint8_t*)big_alloc(bytes);
  // not an assert: those are compiled out in timing builds
  if (!g->data) {
    printf("cannot allocate %zu bytes for the viewshed grid\n", bytes);
    exit(1);
  }
  // all-zero bytes are NOT_VISIBLE; writing also touches the pages now, so
  // page faults aren't counted as viewshed time
  memset(g->data, 0, bytes);
  return g;
}

void visgrid_clear(VisGrid* g) {
  memset(g->data, 0, visgrid_bytes(g->nrows, g->ncols));
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
  const char* text[3] = {"0.000000 ", "1.000000 ", nodata};
  for (int r = 0; r < g->nrows; r++) {
    for (int c = 0; c < g->ncols; c++) {
      fputs(text[visgrid_cell(g, elev, r, c)], f);
    }
    fputs("\n", f);
  }
  fclose(f);
}

void visgrid_free(VisGrid* g) {
  free(g->data);
  free(g);
}
