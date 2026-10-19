#include <stdio.h>
#include <stdlib.h>
#include "grid.h"
#include "vis.h"

int main(int argc, char** argv) {
  if (argc != 6) {
    fprintf(stderr, "usage: %s <elev.asc> <vis.asc> <row> <col> <height>\n", argv[0]);
    return 1;
  }
  char* elev_name = argv[1];
  char* vis_name  = argv[2];
  int   vr        = atoi(argv[3]);
  int   vc        = atoi(argv[4]);
  float vh        = atof(argv[5]);

  printf("running with arguments:\n");
  printf("\telev_name: %s\n", elev_name);
  printf("\tvis_name:  %s\n", vis_name);
  printf("\tvp=(%d, %d, %.1f)\n", vr, vc, vh);

  FILE* in = fopen(elev_name, "r");
  if (!in) { perror(elev_name); return 1; }
  Grid* elev = grid_read(in);
  fclose(in);

  printf("grid: nrows=%d ncols=%d cellsize=%.4f elev in [%.2f, %.2f]\n",
         elev->nrows, elev->ncols, elev->cellsize,
         elev->min_value, elev->max_value);

  if (vr < 0 || vr >= elev->nrows || vc < 0 || vc >= elev->ncols) {
    fprintf(stderr, "viewpoint (%d,%d) is outside the grid\n", vr, vc);
    return 1;
  }

  Grid* vis = grid_init_from(elev);
  vis->nodata_value = -1;

  int size = compute_viewshed_grid(elev, vis, vr, vc, vh);
  printf("viewshed size = %d  (%.2f%% of %d cells)\n",
         size, 100.0 * size / (elev->nrows * elev->ncols),
         elev->nrows * elev->ncols);

  FILE* out = fopen(vis_name, "w");
  if (!out) { perror(vis_name); return 1; }
  grid_write(out, vis);
  fclose(out);
  printf("wrote %s\n", vis_name);

  grid_free(vis);
  grid_free(elev);
  return 0;
}
