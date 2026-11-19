# Optimizations, 2026-11-18

These changes are to `is_point_visible` and its helpers in `vis.cpp`, guided by the profile `perf_data/washu20.data` (Washington DEM, event `cpu_core/cycles/Pu`). The full line-by-line breakdown is in `perf_data/washu20.data_analysis.md`.

## Evidence from the profile

Top functions, by own time (`perf report -i perf_data/washu20.data --no-children -g none --stdio`):

```
38.06%  vis  vis        [.] is_point_visible(_grid const*, int, int, int, int, float)
29.73%  vis  libc.so.6  [.] __vfscanf_internal
 8.64%  vis  libc.so.6  [.] __GI_____strtof_l_internal
 8.27%  vis  libc.so.6  [.] __strlen_avx2
 6.31%  vis  libc.so.6  [.] round_and_return
```

Inside `is_point_visible`, nearly all the time is the loop over grid-line crossings. The hottest instructions, from `perf annotate` (percent of the function's own time):

```
 7.56 │ vcvttss2si   %xmm0,%ebp                 below = (int)other
 0.02 │ mov          %ebp,%eax
      │ imul         %esi,%eax                  below * ncols
 0.02 │ lea          (%rdx,%rax,1),%r12d        + line
 0.03 │ add          %esi,%eax                  + ncols, next row
11.08 │ add          %edx,%eax
 0.02 │ mov          %eax,%eax                  widen 32 → 64 bits
 0.18 │ vmovss       (%rcx,%rax,4),%xmm14       load h2
 0.08 │ vmovss       (%rcx,%r12,4),%xmm13       load h1
 8.99 │ vucomiss     %xmm14,%xmm12              h2 == nodata
 8.83 │ vucomiss     %xmm13,%xmm12              h1 == nodata
11.31 │ vcvtsi2ss    %ebp,%xmm5,%xmm1           (float)below, for t
 9.98 │ vsubss       %xmm0,%xmm3,%xmm1          1 - t
10.31 │ vmulss       %xmm7,%xmm0,%xmm0          (height - vz) * steps_f
10.65 │ jbe          b0                         visibility test
```

Mapped back to source lines:

| % | Line |
|---|---|
| 21.2 | `if ((height - vz) * steps_f > target_rise * k_f)` |
| 18.1 | the nodata check on `h1` and `h2` |
| 11.4 | `float t = value - below;` |
| 11.1 | `grid->data[r * grid->ncols + c]`, the index math |
| 10.0 | `h1 * (1.0f - t) + h2 * t` |

## Changes

### 1. Visibility test compares against a precomputed slope

Before: `(height - vz) * steps_f > target_rise * k_f`, two multiplies per step.
After: `slope = target_rise / steps` once per target, then `height > vz + k_f * slope`, one FMA per step.

Why: this was the hottest line (21.2%). It trades one division per target for one multiply removed from every step.

### 2. Pointer indexing with 64-bit offsets

Before: each step called `grid_get`, computing `r * ncols + c` in 32 bits and then widening it to 64 bits for the address (the `imul`/`add`/`mov %eax,%eax` above).
After: `check_crossings` keeps a pointer `line_start` to the current grid line and moves it by one line each step. `get_fraction` reads `line_start[lower * stride]` and the next cell at `+ stride`, with all offsets as 64-bit `long`, which also covers the per-step move being negative when the ray runs left or up.

Why: the index math was 11.1%. In the horizontal-line loop (`AlongRows = false`), `stride` is 1, so the multiply is gone completely. In the vertical-line loop, `lower * ncols` stays, because `lower` does not change by a fixed amount, but the `+ line` add and the 32-to-64-bit widening are gone. `grid_get`'s bounds check is kept as asserts on `lower` against `count`.

### 3. Second cell skipped when the ray lands exactly on a cell (REVERTED)

When `fraction == 0`, `h2` was set to `h1` instead of being read. The ablation (below) showed this branch costs about 15%, more than changes 1 and 2 gain together, so it was removed. Both cells are always read again. To keep the edge case safe, `grid_malloc_data` now allocates one padding row past the end, set to a value that isn't nodata. The second cell has zero weight there.

### 4. Nodata check moved out of the interpolation

Before: `interpolate` read both cells, set a `bool* nodata`, and blended.
After: `get_fraction` reads the two cells and returns `t`. `check_crossings` tests them for nodata, and `interpolate(h1, h2, t)` only blends.

Why: readability. Every helper is inlined, so the machine code is the same. The nodata check is still 18% of the time. The planned fix is to replace nodata cells with a huge sentinel, so the check only runs after a step fails the visibility test (see the analysis file).

## Verification

The output viewshed (`-v`) is byte-identical to the version before the pointer indexing change on five runs: `set1.asc` at the center, `(10, 380)` and `(460, 5)` with height 20, and `maine-brunsdem-26m.asc` at the center and `(3, 3)`.

## Ablation (experiment `ablation-2026-11-18`)

One change at a time, `compute_viewshed_grid` time relative to HEAD `bin/grid-1bit`, best of 3:

| Binary | Change | portland | bremen | sierra (3 pts) |
|---|---|---|---|---|
| `abl-head` | HEAD `vis.cpp`, `int` instead of `unsigned` in `Grid` | 1.058 | 1.041 | 1.03–1.04 |
| `abl-slope` | + 1 | 0.997 | 0.984 | 0.99 |
| `abl-ptr` | + 2 | 0.981 | 0.990 | 0.98–1.05 |
| `abl-ptrslope` | + 1 + 2 | **0.923** | **0.937** | **0.92–0.98** |
| `abl-opt` | + 1 + 2 + 3 (the full 2026-11-18 version) | 1.076 | 1.065 | 1.07–1.08 |

The `int` change slows `grid_get`'s index math by about 5%, but the hot loop no longer calls `grid_get` once 2 is in. Kept: 1 and 2. Dropped: 3. Change 1 flips a few cells (+0 to +6 per viewshed). Change 2 alone is bit-identical to HEAD.

### Second round (experiment `ablation2-2026-11-18`, on top of 1 + 2)

| Binary | Change | portland | bremen | sierra (3 pts) |
|---|---|---|---|---|
| `k-nan` | nodata stored as NaN; the nodata check folds into the visibility test | **0.716** | **0.685** | **0.68–0.77** |
| `k-fma` | `interpolate` as `h1 + t * (h2 - h1)` | 0.973 | 0.948 | 0.95–0.96 |
| `k-floor` | `t` from `floorf` instead of int-to-float | 1.153 | 1.111 | 1.09–1.13 |

Kept: NaN and FMA. Dropped: `floorf`. `grid_is_nodata` now uses `isnan`, and `nodata_value` is only used for the header and the output. Do not build with `-ffast-math`, which assumes NaN never happens.

Final (`r3-keep2`, experiment `final-2026-11-18`): 0.63–0.78 of HEAD's `compute_viewshed_grid` time.

The profile below predates these changes. `washu20.data` predates them, so the next step is to rebuild with `make perf`, record a new profile and compare.
