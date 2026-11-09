//
// your name

#include "grid.h"
#include "map.h"
#include "vis.h"
#include <chrono>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#include <filesystem>
#include <string>
#include <sys/resource.h>
#include <vector>

struct Snap {
  double wall, user, sys;
};

static Snap now() {
  rusage r;
  getrusage(RUSAGE_SELF, &r);
  double wall = std::chrono::duration<double>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();
  return {wall, r.ru_utime.tv_sec + r.ru_utime.tv_usec / 1e6,
          r.ru_stime.tv_sec + r.ru_stime.tv_usec / 1e6};
}

struct Timer {
  std::vector<std::pair<std::string, Snap>> rows;
  Snap t0 = now(), last = t0;
  void lap(const std::string& name) { // call at the END of a phase
    Snap s = now();
    rows.push_back(
        {name, {s.wall - last.wall, s.user - last.user, s.sys - last.sys}});
    last = s;
  }
  void report() { // bench.py parses the @time lines
    printf("#time %-24s %12s %12s %12s\n", "phase", "wall(s)", "user(s)",
           "sys(s)");
    for (auto& [n, d] : rows)
      printf("@time %-24s %12.6f %12.6f %12.6f\n", n.c_str(), d.wall, d.user,
             d.sys);
    printf("@time %-24s %12.6f %12.6f %12.6f\n", "TOTAL", last.wall - t0.wall,
           last.user - t0.user, last.sys - t0.sys);
  }
};

// debug output level, set with -d, used by vis.cpp
int DEBUG_LEVEL = 0;

// Command line options, anything not given keeps these defaults.
struct Options {
  char* elev_name = NULL; // required
  char* vis_name = NULL;  // -v: write the viewshed .asc to out/; off if NULL
  bool bitmaps = false;   // -b: write the bitmaps to out/
  int vr = -1, vc = -1;   // -r/-c: viewpoint; -1 means the grid's center
  float vh = 0;           // -h: viewpoint height above the terrain
};

static const char usage[] =
    "Usage: %s [options] <elev.asc>\n"
    "By default only computes the viewshed and prints its size and timings.\n"
    "    -e <elev.asc>   Elevation grid (or give it as the last argument)\n"
    "    -r <row>        Viewpoint row (default: center row)\n"
    "    -c <col>        Viewpoint col (default: center col)\n"
    "    -h <height>     Viewpoint height above ground (default: 0)\n"
    "    -v <vis.asc>    Also write the viewshed grid to out/<vis.asc>\n"
    "    -b              Also write the hillshade, viewshed and overlay "
    "bitmaps to out/\n"
    "    -d <level>      Debug level (0=off, 1=verbose; default: 0)\n";

static void fail(const char* message, const char* prog) {
  printf("oops..%s\n", message);
  printf(usage, prog);
  exit(1);
}

/* ************************************************************ */
/*  Reads command line arguments
 */
static Options parseArguments(int argc, char* const* argv) {
  Options o;

  // int, since a char can't hold getopt's -1 where char is unsigned
  int c;
  while ((c = getopt(argc, argv, "e:v:br:c:h:d:")) != -1) {
    switch (c) {
    case 'e':
      o.elev_name = optarg;
      break;
    case 'v':
      o.vis_name = optarg;
      break;
    case 'b':
      o.bitmaps = true;
      break;
    case 'r':
      o.vr = atoi(optarg);
      break;
    case 'c':
      o.vc = atoi(optarg);
      break;
    case 'h':
      o.vh = atof(optarg);
      break;
    case 'd':
      DEBUG_LEVEL = atoi(optarg);
      break;
    default:
      printf(usage, argv[0]);
      exit(2);
    }
  }

  // the elevation grid can also be the one positional argument
  if (!o.elev_name && optind == argc - 1)
    o.elev_name = argv[optind++];
  if (optind < argc)
    fail("unexpected extra arguments", argv[0]);
  if (!o.elev_name)
    fail("an elevation grid is required", argv[0]);
  if (DEBUG_LEVEL < 0)
    fail("debug level cannot be <0", argv[0]);
  if (o.vh < 0)
    fail("viewpoint height cannot be <0", argv[0]);
  return o;
}

////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  Timer T;

  Options o = parseArguments(argc, argv);

  // read elevation grid
  printf("reading %s\n", o.elev_name);
  Grid* elev_grid = grid_read_from_file(o.elev_name);
  grid_print_stats(elev_grid, o.elev_name);
  T.lap("grid_read_from_file");

  // default viewpoint is the center, which needs the grid's size
  if (o.vr < 0)
    o.vr = elev_grid->nrows / 2;
  if (o.vc < 0)
    o.vc = elev_grid->ncols / 2;
  // checked here, not with an assert, since asserts are off in timing builds
  if (!inside_grid(elev_grid, o.vr, o.vc))
    fail("viewpoint is outside the grid", argv[0]);
  if (grid_is_nodata(elev_grid, o.vr, o.vc))
    fail("viewpoint is on a nodata cell, pick another with -r/-c", argv[0]);

  printf("running with arguments:\n\telev_name: %s\n\tvp=(%d, %d, %.1f)\n"
         "\tvis_name: %s\n\tbitmaps: %d\n\tdebug_level: %d\n",
         o.elev_name, o.vr, o.vc, o.vh, o.vis_name ? o.vis_name : "(none)",
         o.bitmaps, DEBUG_LEVEL);
  fflush(stdout);

  // create the viewshed grid
  VisGrid* vis_grid = visgrid_init(elev_grid->nrows, elev_grid->ncols);
  T.lap("init_vis_grid");

  // compute viewshed grid
  long count = compute_viewshed_grid(elev_grid, vis_grid, o.vr, o.vc, o.vh);
  T.lap("compute_viewshed_grid");

  printf("viewshed size=%ld\n", count);

  // all outputs go in out/, relative to where vis is run
  if (o.vis_name || o.bitmaps)
    std::filesystem::create_directories("out");

  if (o.vis_name) {
    std::string vis_path =
        "out/" + std::filesystem::path(o.vis_name).filename().string();
    printf("writing %s\n", vis_path.c_str());
    visgrid_write_to_file(vis_path.c_str(), vis_grid, elev_grid);
    T.lap("grid_write_to_file");
  }

  if (o.bitmaps) {
    // full resolution, streamed row by row, so no image is held in memory
    printf("writing out/map.hillshade.bmp, out/map.viewshed.bmp, "
           "out/map.viewshed-over-hillshade.bmp\n");
    fflush(stdout);
    write_bitmaps(elev_grid, vis_grid, o.vr, o.vc, .5, "out/map.hillshade.bmp",
                  "out/map.viewshed.bmp",
                  "out/map.viewshed-over-hillshade.bmp");
    T.lap("write_bitmaps");
  }

  // cleanup: free elevation grid, we don't need it anymore
  grid_free(elev_grid);
  T.lap("free_elev_grid");

  // cleanup: free vis grid, we don't need it anymore
  visgrid_free(vis_grid);
  T.lap("free_vis_grid");

  T.report();
}
