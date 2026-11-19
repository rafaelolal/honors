# Experiment log

One entry per experiment. Note bad runs here instead of deleting them from `results.csv`.

Template:

## YYYY-MM-DD: experiment-name
- **Question:**
- **Code:**
- **Conclusion:**

## 2026-10-31: serial-timing
- Test run of serial algorithm on 10 points with smaller DEMs
- Serial code as of this date
- 
- 

## 2026-11-18: versions-2026-11-18
- **Question:** How much faster is the serial viewshed after the 2026-11-18 optimizations (`src/r3/notes/optimizations_2026-11-18.md`) than older versions?
- **Code:** `bin/two_bit` (Nov 8, the build profiled in `washu20.data`), `bin/grid-1bit` (byte-identical to a fresh build of HEAD a469ee2), `bin/r3-a469ee2-opt` (a469ee2 plus the uncommitted 2026-11-18 changes to `vis.cpp`). One thread, height 0, DEMs and viewpoints in `plans.py`. Planned so that `two_bit`, the slowest, takes about 1 hour.
- **Note:** The `-opt` binary gives slightly different viewsheds (set1 at (250, 250): 11980 cells vs 11970 for HEAD), from the precomputed slope's rounding and from not flagging a nodata neighbor that has zero weight. Compare `viewshed_size` along with the timings.
- **Conclusion:** `-opt` was 4–10% *slower* than HEAD on every point; `two_bit` ~1.7x slower than HEAD. The ablation below found why.

## 2026-11-18: ablation-2026-11-18, keep-2026-11-18
- **Question:** Which of the 2026-11-18 changes gained time and which lost it?
- **Code:** `bin/abl-*`, one change each off HEAD (see `src/r3/notes/optimizations_2026-11-18.md`). `bin/r3-keep` = slope + pointer indexing, no `fraction == 0` branch.
- **Conclusion:** slope ~3–5% faster, pointer indexing ~2–5% faster, the `fraction == 0` branch ~15% slower (removed). `r3-keep` is ~6–8% faster than HEAD, knox included. `keep-2026-11-18` was stopped early (too long); the rows it wrote are fine.

## 2026-11-18: ablation2-2026-11-18, final-2026-11-18
- **Question:** Do the remaining items in `washu20.data_analysis.md` help, on top of `r3-keep`?
- **Code:** `bin/k-fma` (`h1 + t*(h2-h1)`), `bin/k-floor` (`floorf` for `t`), `bin/k-nan` (nodata stored as NaN, one test per step). `bin/r3-keep2` = `r3-keep` + NaN + FMA, the current working tree.
- **Conclusion:** NaN ~25–32% faster, FMA ~4% faster, `floorf` ~9–15% slower (dropped). `r3-keep2` is 23–37% faster than HEAD (portland 19.0 → 12.8 s). NaN and floor are bit-identical to `r3-keep`; FMA and slope flip a few cells (sierra (18, 313): 23902 → 23926, the largest).

