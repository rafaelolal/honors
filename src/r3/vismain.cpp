//
// your name

#include "grid.h"
#include "map.h"
#include "vis.h"
#include <chrono>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <filesystem>
#include <string>
#include <sys/resource.h>
#include <vector>

// built by make with -DBUILD_FLAGS which a plain g++ build has none
#ifndef BUILD_FLAGS
#define BUILD_FLAGS "unknown"
#endif

// prints "@stat huge_pages_mb N" to this process's memory on transparent huge
// pages
static void print_huge_pages() {
  FILE* f = fopen("/proc/self/smaps_rollup", "r");
  if (!f)
    return;
  char line[256];
  long kb;
  while (fgets(line, sizeof line, f))
    if (sscanf(line, "AnonHugePages: %ld kB", &kb) == 1)
      printf("@stat huge_pages_mb %.1f\n", kb / 1024.0);
  fclose(f);
}

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
  size_t point_start = 0;
  void lap(const std::string& name) { // call at the END of a phase
    Snap s = now();
    rows.push_back(
        {name, {s.wall - last.wall, s.user - last.user, s.sys - last.sys}});
    last = s;
  }
  // -p: a point's phases are printed as soon as it's done, as "@ptime i ..."
  // lines, so a timeout keeps the points before it. The time spent printing
  // and checking between points isn't counted in any phase.
  void begin_point() {
    last = now();
    point_start = rows.size();
  }

  void end_point(size_t i) {
    for (size_t k = point_start; k < rows.size(); k++)
      printf("@ptime %zu %-24s %12.6f %12.6f %12.6f\n", i,
             rows[k].first.c_str(), rows[k].second.wall, rows[k].second.user,
             rows[k].second.sys);
    rows.resize(point_start);
    fflush(stdout);
    last = now();
  }

  void report() { // bench.py parses the @time lines
    // how the numbers were taken, so the log documents the method
    printf("#time one run, no warm-up; wall from steady_clock, user/sys from "
           "getrusage(RUSAGE_SELF)\n");
    printf("#time %-24s %12s %12s %12s\n", "phase", "wall(s)", "user(s)",
           "sys(s)");
    for (auto& [n, d] : rows)
      printf("@time %-24s %12.6f %12.6f %12.6f\n", n.c_str(), d.wall, d.user,
             d.sys);
    printf("@time %-24s %12.6f %12.6f %12.6f\n", "TOTAL", last.wall - t0.wall,
           last.user - t0.user, last.sys - t0.sys);
  }
};

// Command line options, anything not given keeps these defaults.
struct Options {
  char* elev_name = NULL;   // required
  char* vis_name = NULL;    // -v: write the viewshed .asc to out/; off if NULL
  bool bitmaps = false;     // -b: write the bitmaps to out/
  int vr = -1, vc = -1;     // -r/-c: viewpoint; -1 means the grid's center
  char* points_name = NULL; // -p: file of "row col" lines, instead of -r/-c
  float vh = 0;             // -h: viewpoint height above the terrain
};

static const char usage[] =
    "Usage: %s [options] <elev.asc>\n"
    "By default only computes the viewshed and prints its size and timings.\n"
    "    -e <elev.asc>   Elevation grid (or give it as the last argument)\n"
    "    -r <row>        Viewpoint row (default: center row)\n"
    "    -c <col>        Viewpoint col (default: center col)\n"
    "    -p <points.txt> Many viewpoints, one \"row col\" per line: reads the\n"
    "                    grid once and reuses the viewshed grid between them\n"
    "    -h <height>     Viewpoint height above ground (default: 0)\n"
    "    -v <vis.asc>    Also write the viewshed grid to out/<vis.asc>\n"
    "    -b              Also write out/<vis>.hillshade.bmp, .viewshed.bmp "
    "and\n"
    "                    .viewshed-over-hillshade.bmp (needs -v)\n"
    "With -p, outputs are named <vis>.r<row>.c<col>.asc (and .bmp).\n";

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
  while ((c = getopt(argc, argv, "e:v:br:c:h:p:")) != -1) {
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
    case 'p':
      o.points_name = optarg;
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
  if (o.vh < 0)
    fail("viewpoint height cannot be <0", argv[0]);
  // bitmaps are named after the -v grid
  if (o.bitmaps && !o.vis_name)
    fail("-b needs -v, which names the bitmaps", argv[0]);
  if (o.points_name && (o.vr >= 0 || o.vc >= 0))
    fail("give -p or -r/-c, not both", argv[0]);
  return o;
}

struct Point {
  int r, c;
};

static std::vector<Point> read_points(const char* name, const char* prog) {
  FILE* f = fopen(name, "r");
  if (!f)
    fail("cannot open the -p points file", prog);
  std::vector<Point> points;
  Point p;
  while (fscanf(f, "%d %d", &p.r, &p.c) == 2)
    points.push_back(p);
  bool clean_end = feof(f);
  fclose(f);
  if (!clean_end)
    fail("the -p file should only have \"row col\" lines", prog);
  if (points.empty())
    fail("the -p file has no points", prog);
  return points;
}

// out/vis.asc, or with -p out/vis.r<R>.c<C>.asc, so each point gets its own
static std::string out_base(const Options& o, bool batch, Point p) {
  std::string base = "out/" + std::filesystem::path(o.vis_name).stem().string();
  if (batch)
    base += ".r" + std::to_string(p.r) + ".c" + std::to_string(p.c);
  return base;
}

////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
  Timer T;

  Options o = parseArguments(argc, argv);
  // the setup a result depends on, kept in every log
  printf("@build compiler g++ %s\n@build flags %s\n", __VERSION__, BUILD_FLAGS);

  // read elevation grid
  printf("reading %s\n", o.elev_name);
  Grid* elev_grid = grid_read_from_file(o.elev_name);
  grid_print_stats(elev_grid, o.elev_name);
  // "@stat name value" lines are parsed by experiments/bench.py
  printf("@stat nrows %d\n@stat ncols %d\n@stat min_h %f\n@stat max_h %f\n"
         "@stat nodata %d\n",
         elev_grid->nrows, elev_grid->ncols, elev_grid->min_value,
         elev_grid->max_value, elev_grid->nb_nodata_values);
  T.lap("grid_read_from_file");

  bool batch = o.points_name != NULL;
  std::vector<Point> points;
  if (batch) {
    points = read_points(o.points_name, argv[0]);
  } else {
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
    points.push_back({o.vr, o.vc});
  }

  printf("running with arguments:\n\telev_name: %s\n", o.elev_name);
  if (batch)
    printf("\tpoints: %s (%zu), h=%.1f\n", o.points_name, points.size(), o.vh);
  else
    printf("\tvp=(%d, %d, %.1f)\n", o.vr, o.vc, o.vh);
  printf("\tvis_name: %s\n\tbitmaps: %d\n", o.vis_name ? o.vis_name : "(none)",
         o.bitmaps);
  // all outputs go in out/, relative to where vis is run
  if (o.vis_name || o.bitmaps)
    std::filesystem::create_directories("out");
  fflush(stdout);

  // create the viewshed grid, once for all points
  VisGrid* vis_grid = visgrid_init(elev_grid->nrows, elev_grid->ncols);
  T.lap("init_vis_grid");

  for (size_t i = 0; i < points.size(); i++) {
    Point p = points[i];
    if (batch) {
      // a bad point is reported and skipped, the rest still run
      if (!inside_grid(elev_grid, p.r, p.c) ||
          grid_is_nodata(elev_grid, p.r, p.c)) {
        printf("@pskip %zu %d %d %s\n", i, p.r, p.c,
               inside_grid(elev_grid, p.r, p.c) ? "nodata" : "outside");
        continue;
      }
      printf("@point %zu %d %d\n", i, p.r, p.c);
      T.begin_point();
      // every point starts from an all NOT_VISIBLE grid, reusing the memory
      visgrid_clear(vis_grid);
      T.lap("clear_vis_grid");
    }

    long count = compute_viewshed_grid(elev_grid, vis_grid, p.r, p.c, o.vh);
    T.lap("compute_viewshed_grid");

    if (o.vis_name) {
      std::string vis_path =
          out_base(o, batch, p) +
          std::filesystem::path(o.vis_name).extension().string();
      visgrid_write_to_file(vis_path.c_str(), vis_grid, elev_grid);
      T.lap("grid_write_to_file");
    }

    if (o.bitmaps) {
      // out/vis.asc -> out/vis.hillshade.bmp, ...
      std::string base = out_base(o, batch, p);
      std::string hs = base + ".hillshade.bmp", vs = base + ".viewshed.bmp",
                  ov = base + ".viewshed-over-hillshade.bmp";
      // full resolution, streamed row by row, so no image is held in memory
      write_bitmaps(elev_grid, vis_grid, p.r, p.c, .5, hs.c_str(), vs.c_str(),
                    ov.c_str());
      T.lap("write_bitmaps");
    }

    if (batch) {
      printf("@pstat %zu viewshed_size %ld\n", i, count);
      T.end_point(i);
    } else {
      printf("viewshed size=%ld\n@stat viewshed_size %ld\n", count, count);
    }
  }

  // memory on 2 MB pages, to confirm huge pages worked; its own phase
  // so reading /proc isn't timed as anything else
  print_huge_pages();
  T.lap("check_huge_pages");

  // cleanup: free elevation grid, we don't need it anymore
  grid_free(elev_grid);
  T.lap("free_elev_grid");

  // cleanup: free vis grid, we don't need it anymore
  visgrid_free(vis_grid);
  T.lap("free_vis_grid");

  T.report();
}
