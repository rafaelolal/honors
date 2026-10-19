// your name

#include "viewshed.h"
#include "grid.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//
void compute_viewshed_grid(const Grid* elevGrid, Grid* visGrid, int vr, int vc,
                           float vh) {

  assert(elevGrid);
  // visGrid is assumed created/allocated
  assert(visGrid && visGrid->data);

  // initialize visGrid to NOT_VISIBLE
  for (int i = 0; i < elevGrid->nrows; i++) {
    for (int j = 0; j < elevGrid->ncols; j++) {
      if (grid_is_nodata(elevGrid, i, j)) {
        grid_set_nodata(visGrid, i, j);
      } else {
        grid_set(visGrid, i, j, NOT_VISIBLE);
      }
    } // for j
  } // for i
}
