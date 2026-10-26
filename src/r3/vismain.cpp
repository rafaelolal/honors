//
// your name

#include "grid.h"
#include "map.h"
#include "pixel_buffer.h"
#include "vis.h"

#include <assert.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// debug output level, set with -d; used by vis.cpp
int DEBUG_LEVEL = 0;

/* ************************************************************ */
/*  Reads command line arguments
 */
void parseArguments(int argc, char* const* argv, char** elev_name,
                    char** vis_name, int* vr, int* vc, float* vh,
                    int* debug_level) {

  const char usage[] =
      "Usage: %s -e <elev.asc> -v <vis.asc> -r <vr> -c <vc> [-h <vh>] [-d "
      "<level>]\n"
      "    -e, --elev        Required string. Elevation grid name.\n"
      "    -v, --vis         Required string. Vis grid name.\n"
      "    -r, --row         Required int. Viewpoint row\n"
      "    -c, --col         Required int. Viewpoint col\n"
      "    -h, --height      Optional int. Viewpoint height above ground\n"
      "    -d, --debug       Optional int. Debug level (0=off, 1=verbose)\n";

  /* if (argc != ) { */
  /*   printf("wrong nb arguments argc=%d (expect 6)\n", argc);  */
  /*   printf("%s", usage); */
  /*   exit(1);  */
  /* } */

  // default initial values
  *elev_name = *vis_name = NULL;
  *vr = *vc = -1;
  *vh = 0;
  *debug_level = 0;

  char c;
  while ((c = getopt(argc, argv, "e:v:r:c:h:d:")) != -1) {
    switch (c) {
    case 'e':
      // elevation grid
      assert(optarg);
      *elev_name = optarg;
      break;
    case 'v':
      assert(optarg);
      // vis grid name
      *vis_name = optarg;
      break;
    case 'r':
      // viewpoint row
      assert(optarg);
      *vr = atoi(optarg);
      break;
    case 'c':
      // viewpoint col
      assert(optarg);
      *vc = atoi(optarg);
      break;
    case 'h':
      // viewpoint  height above ground
      assert(optarg);
      *vh = atof(optarg);
      printf("h\n");
      fflush(stdout);
      break;
    case 'd':
      // debug level
      assert(optarg);
      *debug_level = atoi(optarg);
      break;
    default:
      printf("Illegal option given\n");
      exit(2);
    } // switch c
  } // while

  if (*elev_name == NULL) {
    printf("oops..Elev grid name cannot be NULL\n");
    printf(usage, argv[0]);
    exit(1);
  }
  if (*vis_name == NULL) {
    printf("oops..Vis grid name  cannot be NULL\n");
    printf(usage, argv[0]);
    exit(1);
  }
  if (*vr < 0) {
    printf("oops..viewpoint row  cannot be <0 \n");
    printf(usage, argv[0]);
    exit(1);
  }
  if (*vc < 0) {
    printf("oops..viewpoint col  cannot be <0 \n");
    printf(usage, argv[0]);
    exit(1);
  }
  if (*debug_level < 0) {
    printf("oops..debug level cannot be <0 \n");
    printf(usage, argv[0]);
    exit(1);
  }
  if (*vh < 0) {
    printf("oops..viewpoint height  cannot be <0 \n");
    printf(usage, argv[0]);
    exit(1);
  }
  fflush(stdout);
}

////////////////////////////////////////////////////////////
int main(int argc, char** argv) {

  char *elev_name, *vis_name;
  int vr, vc;
  float vh;
  parseArguments(argc, argv, &elev_name, &vis_name, &vr, &vc, &vh,
                 &DEBUG_LEVEL);
  printf("running with arguments:\n\telev_name: %s \n\tvis_name: %s\n\tvp=(%d, "
         "%d, %.1f)\n\tdebug_level: %d\n",
         elev_name, vis_name, vr, vc, vh, DEBUG_LEVEL);
  fflush(stdout);

  // read elevation grid
  printf("reading %s\n", elev_name);
  Grid* elev_grid = grid_read_from_file(elev_name);
  grid_print_stats(elev_grid, elev_name);

  // create the viewshed grid
  Grid* vis_grid = grid_init_from(elev_grid);

  // compute viewshed grid
  compute_viewshed_grid(elev_grid, vis_grid, vr, vc, vh);

  grid_print_stats(vis_grid, vis_name);

  // write viewshed grid
  printf("writing %s\n", vis_name);
  grid_write_to_file(vis_name, vis_grid);

  ////////////////////////////////////////////////////////////////////
  // CREATE BITMAPS
  //////////////////////////////////////////////////////////////////

  const PixelBuffer pb = init_pixel_buffer(elev_grid->ncols, elev_grid->nrows);
  const PixelBuffer pb2 = init_pixel_buffer(elev_grid->ncols, elev_grid->nrows);
  float alpha;

  // create hillshade grid
  Grid* hillshade_grid = grid_init_from(elev_grid);
  generate_hillshade(elev_grid, hillshade_grid);
  // grid_print_stats(vis_grid, "hillshade grid: ");
  // create hillshade bitmap
  grid_to_pixelbuffer(hillshade_grid, pb);
  printf("writing map.hillshade.bmp\n");
  save_pixel_buffer_to_file(&pb, "map.hillshade.bmp");
  grid_free(hillshade_grid);

  // create viewshed bitmap in pb2
  grid_vis_to_pixelbuffer(vis_grid, vr, vc, pb2);

  printf("writing map.viewshed.bmp\n");
save_pixel_buffer_to_file(&pb2, "map.viewshed.bmp");
printf("done\n"); fflush(stdout);

  // viewshed overlayed on hillshade
  // pb has hillshade, pb2 has viewshed
  alpha = .5;
  pixelbuffer_overlay(pb, pb2, alpha);
  printf("writing map.viewshed-over-hillshade.bmp\n");
  save_pixel_buffer_to_file(&pb, "map.viewshed-over-hillshade.bmp");
  printf("done\n");
  fflush(stdout);

  // cleanup: free elevation grid, we don't need it anymore
  grid_free(elev_grid);

  // cleanup: free vis grid, we don't need it anymore
  grid_free(vis_grid);

  // free the pixel buffers
  deinit_pixel_buffer(&pb);
  deinit_pixel_buffer(&pb2);
}
