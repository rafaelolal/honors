// visdiff: git-style diff of two viewshed .asc grids.
//
// Draws the secondary grid colored by how each cell compares with the
// primary: green added, red removed, yellow nodata in only one, gray visible
// in both, black visible in neither, blue nodata in both.
// Exit code like git diff --exit-code: 0 identical, 1 different, 2 error.

#include "diff.h"
#include "grid.h"

#include <filesystem>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

static const char usage[] =
    "Usage: %s [-e <elev.asc>] [-o <prefix>] <primary.asc> <secondary.asc>\n"
    "    -e   Optional. DEM to also draw the diff over its hillshade\n"
    "    -o   Optional. Output name prefix in out/ (default: diff)\n";

int main(int argc, char** argv) {
  const char* elev_name = NULL;
  std::string prefix = "diff";

  int opt;
  while ((opt = getopt(argc, argv, "e:o:")) != -1) {
    switch (opt) {
    case 'e':
      elev_name = optarg;
      break;
    case 'o':
      prefix = optarg;
      break;
    default:
      printf(usage, argv[0]);
      return 2;
    }
  }
  if (argc - optind != 2) {
    printf(usage, argv[0]);
    return 2;
  }
  const char* primary_name = argv[optind];
  const char* secondary_name = argv[optind + 1];

  Grid ph, sh;
  Viewshed* primary = viewshed_read(primary_name, &ph);
  Viewshed* secondary = viewshed_read(secondary_name, &sh);

  if (ph.nrows != sh.nrows || ph.ncols != sh.ncols) {
    printf("grids differ in size: %u x %u vs %u x %u\n", ph.nrows, ph.ncols,
           sh.nrows, sh.ncols);
    return 2;
  }
  // tools print the header with different precision, so this only warns
  if (ph.cellsize != sh.cellsize || ph.xllcorner != sh.xllcorner ||
      ph.yllcorner != sh.yllcorner) {
    printf("warning: headers differ (cellsize %f vs %f, xll %f vs %f, "
           "yll %f vs %f)\n",
           ph.cellsize, sh.cellsize, ph.xllcorner, sh.xllcorner, ph.yllcorner,
           sh.yllcorner);
  }

  Grid* elev = NULL;
  if (elev_name) {
    elev = grid_read_from_file((char*)elev_name);
    if (elev->nrows != ph.nrows || elev->ncols != ph.ncols) {
      printf("DEM is %u x %u but the grids are %u x %u\n", elev->nrows,
             elev->ncols, ph.nrows, ph.ncols);
      return 2;
    }
  }

  // all outputs go in out/, relative to where visdiff is run
  std::filesystem::create_directories("out");
  std::string flat_path = "out/" + prefix + ".diff.bmp";
  std::string overlay_path = "out/" + prefix + ".diff-over-hillshade.bmp";
  DiffCounts d =
      write_diff_bitmaps(primary, secondary, elev, .5, flat_path.c_str(),
                         elev ? overlay_path.c_str() : NULL);

  long cells = (long)ph.nrows * ph.ncols;
  long changed =
      d.n[DIFF_ADDED] + d.n[DIFF_REMOVED] + d.n[DIFF_NODATA_MISMATCH];
  printf("primary:   %s (%u x %u)\n", primary_name, ph.nrows, ph.ncols);
  printf("secondary: %s\n", secondary_name);
  printf("+%ld added, -%ld removed, %ld nodata mismatch, %ld changed "
         "(%.4f%% of cells)\n",
         d.n[DIFF_ADDED], d.n[DIFF_REMOVED], d.n[DIFF_NODATA_MISMATCH],
         changed, 100.0 * changed / cells);
  printf("unchanged: %ld visible, %ld not visible, %ld nodata\n",
         d.n[DIFF_SAME_VIS], d.n[DIFF_SAME_INVIS], d.n[DIFF_SAME_NODATA]);
  printf("wrote %s%s%s\n", flat_path.c_str(), elev ? ", " : "",
         elev ? overlay_path.c_str() : "");

  viewshed_free(primary);
  viewshed_free(secondary);
  if (elev)
    grid_free(elev);
  return changed ? 1 : 0;
}
