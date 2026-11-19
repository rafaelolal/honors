// visdiff: compares a viewshed .asc against a reference .asc.
//
// With the reference as truth, each cell is TP (visible in both), TN (in
// neither), FP (only in ours) or FN (only in the reference). Prints the
// counts, the error rates and a color key for the diff bitmap.
// Exit code like git diff --exit-code: 0 identical, 1 different, 2 error.

#include "diff.h"
#include "grid.h"

#include <filesystem>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

static const char usage[] =
    "Usage: %s [-e <elev.asc>] [-o <prefix>] <reference.asc> <ours.asc>\n"
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
    printf("grids differ in size: %d x %d vs %d x %d\n", ph.nrows, ph.ncols,
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
      printf("DEM is %d x %d but the grids are %d x %d\n", elev->nrows,
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

  long tp = d.n[DIFF_TP], tn = d.n[DIFF_TN], fp = d.n[DIFF_FP],
       fn = d.n[DIFF_FN];
  long n = tp + tn + fp + fn;
  long changed = fp + fn + d.n[DIFF_NODATA_MISMATCH];

  printf("reference: %s (%d x %d)\n", primary_name, ph.nrows, ph.ncols);
  printf("ours:      %s\n", secondary_name);
  printf("\n%-6s %-7s %-26s %15s\n", "class", "color", "meaning", "cells");
  printf("%-6s %-7s %-26s %15ld\n", "TP", "gray", "visible in both", tp);
  printf("%-6s %-7s %-26s %15ld\n", "TN", "black", "visible in neither", tn);
  printf("%-6s %-7s %-26s %15ld\n", "FP", "green", "visible only in ours", fp);
  printf("%-6s %-7s %-26s %15ld\n", "FN", "red", "visible only in reference",
         fn);
  printf("%-6s %-7s %-26s %15ld\n", "-", "yellow", "nodata in only one",
         d.n[DIFF_NODATA_MISMATCH]);
  printf("%-6s %-7s %-26s %15ld\n", "-", "blue", "nodata in both",
         d.n[DIFF_SAME_NODATA]);

  printf("\nN = TP + TN + FP + FN = %ld (nodata excluded)\n", n);
  // ratio, or n/a when the denominator is 0
  auto ratio = [](const char* name, long num, long den) {
    if (den)
      printf("%-37s = %.6f (%.4f%%)\n", name, (double)num / den,
             100.0 * num / den);
    else
      printf("%-37s = n/a\n", name);
  };
  ratio("error rate      (FP + FN) / N", fp + fn, n);
  ratio("relative error  (FP + FN) / (TP + FN)", fp + fn, tp + fn);

  printf("\nwrote %s%s%s\n", flat_path.c_str(), elev ? ", " : "",
         elev ? overlay_path.c_str() : "");

  viewshed_free(primary);
  viewshed_free(secondary);
  if (elev)
    grid_free(elev);
  return changed ? 1 : 0;
}
