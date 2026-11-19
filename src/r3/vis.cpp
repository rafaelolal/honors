#include "vis.h"
#include "grid.h"
#include <cassert>
#include <cmath>

// line_start points at the first cell of the grid line the ray crosses, and
// stride is the distance between neighboring cells along that line. count is
// the number of cells on the line, only used to check bounds.
static inline float get_fraction(float position, const float* line_start,
                                 long stride, int count, float* h1,
                                 float* h2) {
  assert(position >= 0.0f);

  // 64-bit so it can index memory without widening
  const long lower = static_cast<long>(position);
  const float fraction = position - static_cast<float>(lower);
  assert(lower < count);

  const float* cell = line_start + lower * stride;
  // Always read both cells: a branch on fraction == 0 cost ~15% (ablation
  // 2026-11-18). On the last row or column the second cell has zero weight
  // and lands in the padding grid_malloc_data adds.
  assert(lower + 1 < count || fraction == 0.0f);
  *h1 = cell[0];
  *h2 = cell[stride];

  return fraction;
}

// Height at fraction t of the way from h1 to h2.
static inline float interpolate(float h1, float h2, float t) {
  return h1 + t * (h2 - h1);
}

// Checks where the ray crosses one family of grid lines. "line" is the axis we
// step along and "other" is the axis we solve for. Returns VISIBLE if the ray
// gets through, otherwise NOT_VISIBLE or VIS_NODATA.
// AlongRows: the ray crosses vertical grid lines, so interpolate between rows.
// Otherwise it crosses horizontal grid lines, so interpolate between columns.
template <bool AlongRows>
static uint8_t check_crossings(const Grid* elev_grid, int start, int end,
                               float other_start, float other_step, float vz,
                               float target_rise) {
  int dir = end > start ? 1 : -1;
  int steps = dir * (end - start);
  float k_f = 1.0f;
  float slope = target_rise / steps;
  other_step *= dir;

  const long ncols = elev_grid->ncols;
  // cells along a grid line are a row apart for vertical lines, adjacent for
  // horizontal ones; moving to the next line is the other way around
  const long stride = AlongRows ? ncols : 1;
  const long line_step = AlongRows ? 1 : ncols;
  const int count = AlongRows ? elev_grid->nrows : elev_grid->ncols;

  // negative when the ray runs left or up
  const long line_move = dir * line_step;

  const float* line_start = elev_grid->data + (start + dir) * line_step;
  for (int k = 1; k < steps; k++, k_f++, line_start += line_move) {
    float other = other_start + k_f * other_step;
    float h1, h2;
    float t = get_fraction(other, line_start, stride, count, &h1, &h2);
    float height = interpolate(h1, h2, t);

    // nodata cells are NaN, which fails every comparison
    if (!(height <= vz + k_f * slope)) {
      return std::isnan(height) ? VIS_NODATA : NOT_VISIBLE;
    }
  }

  return VISIBLE;
}

uint8_t is_point_visible(const Grid* elev_grid, int r, int c, int vr, int vc,
                         float vh) {
  if (grid_is_nodata(elev_grid, r, c)) {
    return VIS_NODATA;
  }

  if (vr == r && vc == c) {
    return VISIBLE;
  }

  float vz = grid_get(elev_grid, vr, vc) + vh;
  float pz = grid_get(elev_grid, r, c);

  float target_rise = pz - vz;

  float dy = r - vr;
  float dx = c - vc;

  // Vertical grid lines: interpolate between rows.
  uint8_t result =
      check_crossings<true>(elev_grid, vc, c, vr, dy / dx, vz, target_rise);
  if (result == VISIBLE) {
    // Horizontal grid lines: interpolate between columns.
    result =
        check_crossings<false>(elev_grid, vr, r, vc, dx / dy, vz, target_rise);
  }
  return result;
}

long compute_viewshed_grid(const Grid* elev_grid, VisGrid* vis_grid, int vr,
                           int vc, float vh) {
  // grid_get also checks it's inside the grid
  assert(!grid_is_nodata(elev_grid, vr, vc));

  long count = 0;
  for (int r = 0; r < (int)elev_grid->nrows; r++) {
    for (int c = 0; c < (int)elev_grid->ncols; c++) {
      uint8_t v = is_point_visible(elev_grid, r, c, vr, vc, vh);
      // the grid starts all NOT_VISIBLE, so only other values need writing
      if (v != NOT_VISIBLE) {
        visgrid_set(vis_grid, r, c, v);
      }
      count += v == VISIBLE;
    }
  }

  return count;
}