Honors project on computing viewsheds over terrain grids, comparing serial, multicore and GPU implementations.

* `src/`: Code, with the viewshed implementations in `r3/` (serial) and `r3_parallel/` (GPU), alongside smaller CUDA exercises such as `collatz/`, `grayscale/`, `mmult/` and `heat/`.
* `experiments/`: The benchmark harness, experiment plans, saved program builds, timing results and analysis.
* `research/`: A symlink to `/home/research`, a shared folder of DEMs and other big files, which is not stored in this repo.
* `literature/`: Viewshed and parallel computing papers, with reading lists and summaries.
* `readings/`: GPU background material, covering foundations, architecture whitepapers, CUDA tuning guides and microbenchmark papers.

Every tool section below has the same layout: what the tool does, the commands, then the options and notes.

## Building `vis`

`src/r3/vis` is the serial viewshed program.

```bash
# fast build, for timing
make -C src/r3
# symbols and frame pointers, for perf
make -C src/r3 profiling
# debug build
make -C src/r3 debug
```

All builds put the grids on 2 MB huge pages (`-DHUGEPAGES`); `@stat huge_pages_mb` shows whether the kernel gave them.

| Flag | Meaning |
|---|---|
| `-e DEM.asc` | elevation grid (or give it as the last argument) |
| `-r ROW -c COL` | one viewpoint (default: the center) |
| `-p points.txt` | many viewpoints, one `row col` per line. Reads the DEM once and reuses the viewshed grid between points |
| `-h H` | viewpoint height above the ground (default 0) |
| `-v vis.asc` | also write the viewshed grid to `out/vis.asc` (with `-p`: `out/vis.rROW.cCOL.asc`) |
| `-b` | also write the hillshade, viewshed and overlay bitmaps (needs `-v`) |

By default `vis` only computes and prints the viewshed size and the time of each phase (`@time` and `@stat` lines).

## Experiments

Everything in this section runs from `experiments/`.

| File | What it's for |
|---|---|
| `plans.py` | The named experiments: which binaries, DEMs, viewpoints, heights, thread counts and reps to run |
| `bench.py` | Runs experiments from `plans.py` and appends one row per timed phase to `results.csv` |
| `viewpoints.py` | Picks random valid viewpoints and caches them in `serial_timing/viewpoints/` |
| `analysis/summarize.py` | Summary statistics per DEM and code, and comparisons against a baseline |
| `bin/` | Saved `vis` builds, one per code version being compared |
| `results.csv` | All timing results, one row per phase of each run |
| `env/` | One JSON per bench session describing the machine setup |
| `logs/` | `DATE-progress.log` lists what ran when; each run also gets a file with the full `vis` output |
| `log.md` | One entry per experiment: the question, the code, the conclusion, and any bad runs |

### Workflow

```bash
cd experiments
# 1. save the build under test
make -C ../src/r3 && cp ../src/r3/vis bin/NAME-COMMIT
# 2. add a plan to plans.py (see Plans below)
# 3. run it, in tmux if it is long
python3 bench.py EXPERIMENT [EXPERIMENT ...]
# 4. summarize it
python3 analysis/summarize.py EXPERIMENT
# 5. write the entry in log.md
```

### Plans

A plan is a named list of blocks in `plans.py`. Each block runs every combination of its lists.

```python
"my-test": [
    dict(code=["bin/old", "bin/new"], dem=[SIERRA], viewpoint=Random(k=30),
         height=[0], threads=[1], reps=3, batch=True),
],
```

| Key | Type | Meaning |
|---|---|---|
| `code` | list of paths | binaries to run |
| `dem` | list of paths | `.asc` grids |
| `viewpoint` | list of `(row, col)`, or `Random(...)` | viewpoints. `Random(k=30)` picks 30 valid points, `Random(pct=10)` picks 10% of the cells, `seed=` changes the draw (default 0) |
| `height` | list of numbers | viewpoint height, passed as `-h` |
| `threads` | list of ints | sets `OMP_NUM_THREADS` and pins to cores `0..n-1` |
| `schedule` | list of strings, optional | `OMP_SCHEDULE` values, default unset |
| `args` | flat list, optional | extra `vis` flags, e.g. `["-v", "vis.asc"]` or `["-b"]`. Not part of the run key, so keep one set per experiment |
| `reps` | int, optional | times each combination runs, default 1 |
| `batch` | bool, optional | run all viewpoints of a combination in one `vis -p` call, default `False` |

### `bench.py`

```bash
# run one or more experiments
python3 bench.py EXPERIMENT [EXPERIMENT ...]
# list the available experiments
python3 bench.py
# long run, in the background
tmux new -d -s NAME 'python3 bench.py EXPERIMENT; exec bash'
```

Features:
- **Resumable:** finished runs are skipped on restart; failed and timed-out runs are retried. Points skipped by `vis` (nodata or outside the grid) are not retried.
- **Reps as the outer loop:** all combinations run once before any runs twice, so drift over a long run doesn't favor one configuration.
- **Batch mode (`batch=True`):** one `vis -p` call per combination, so the DEM is read once. Each point still gets its own rows; shared phases (reading, `TOTAL`, `time_cmd`) go on a row with empty `vr` and `vc`. Points that finished before a crash or timeout are kept.
- **Random viewpoints:** drawn uniformly over data cells with a fixed seed and cached per DEM, `k` and seed, so reruns use the same points. The first use on a DEM indexes its rows once (cached too).
- **Pinning:** each run is pinned with `taskset` to cores `0..threads-1`. Cores 0–7 are the performance cores.
- **Timeout:** 4 hours per viewpoint (`TIMEOUT`); a batch gets 4 hours × its number of points.
- **Binary identity:** each binary's sha is in the run key, so two builds saved under the same name get separate rows.
- **Output checks:** with `-v`, the output grid's sha goes in `output_sha`. Outputs are written to a temporary folder in `experiments/` and deleted afterwards.
- **Machine setup:** each session saves CPU, kernel, governor, turbo, compiler and commit (with a dirty flag) to `env/ID.json`; each row's `env` column names it.
- **Load:** the 1-minute load average before each run goes in `load1`.
- **Logs:** progress in `logs/DATE-progress.log`; full `vis` output per run in `logs/`.

Columns of `results.csv`, one row per phase:

| Group | Columns |
|---|---|
| Run key | `experiment`, `code`, `binary_sha`, `dem`, `vr`, `vc`, `vh`, `threads`, `schedule`, `rep` |
| Result | `timestamp`, `status` (`ok`, `exit N`, `timeout`, `no output`, `no timing`, `skip REASON`), `output_sha` |
| DEM | `dem_name`, `nrows`, `ncols`, `cells`, `min_h`, `max_h`, `relief`, `nodata`, `nodata_pct` |
| Viewshed | `viewshed_size`, `viewshed_pct` |
| Timing | `phase`, `wall`, `user`, `sys` |
| Setup | `max_rss_mb`, `build_flags`, `load1`, `env` |

Phases include `grid_read_from_file`, `init_vis_grid`, `clear_vis_grid`, `compute_viewshed_grid`, `free_vis_grid`, `TOTAL` (timed inside `vis`) and `time_cmd` (the whole command, timed from outside).

Before a long run:
- **Power profile:** `powerprofilesctl get` should say `performance`. Set it with `powerprofilesctl set performance` (survives reboots).
- **Quiet machine:** the first number from `uptime` should be near 0. Check `who` and `top` for other jobs.
- **Turbo:** leave it on for single-thread runs. For thread scaling, turn it off with `echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo` and back on with `echo 0` (resets to on at reboot).
- **Same settings throughout:** results from different settings aren't comparable. If a setting changes, rerun the baseline.

### `analysis/summarize.py`

```bash
# spread over viewpoints, per DEM and code
python3 analysis/summarize.py EXPERIMENT [EXPERIMENT ...]
# another phase
python3 analysis/summarize.py EXPERIMENT --phase time_cmd
# percentiles every 1%, for plotting
python3 analysis/summarize.py EXPERIMENT --step 1
# each code against a baseline code
python3 analysis/summarize.py EXPERIMENT --baseline r3-keep2
```

| Option | Meaning |
|---|---|
| `--phase P` | phase to summarize, default `compute_viewshed_grid` |
| `--step S` | percentile spacing in %, default 10 |
| `--baseline CODE` | compare every code against `CODE` (binary name without `bin/`) |

Features:
- **Only `ok` rows** of the given phase are used.
- **Reps averaged first:** each viewpoint's reps are averaged, so every viewpoint counts once.
- **Summary per DEM and code:** n, median with 95% CI, mean, std, min, quartiles, max and percentiles every `--step`%. Written to `analysis/summary-EXPERIMENT.csv`.
- **Median CI from ranks:** doesn't assume normal times. Needs at least 6 viewpoints; shows `-` below that.
- **Baseline comparison:** on the viewpoints both codes ran, prints each median with its CI, the speedup (baseline median / code median) and the median time saved per viewpoint with its 95% CI. A difference is significant when that CI excludes 0.

## Checking accuracy with `visdiff`

`src/visdiff/visdiff` compares a viewshed against a reference, such as one made with GDAL.

```bash
# compare two viewsheds
src/visdiff/visdiff reference.asc ours.asc
# draw the diff over the DEM's hillshade, with a chosen output name
src/visdiff/visdiff -e DEM.asc -o NAME reference.asc ours.asc
```

| Option | Meaning |
|---|---|
| `-e DEM.asc` | also draw the diff over the DEM's hillshade |
| `-o NAME` | output name prefix in `out/`, default `diff` |

Prints TP, TN, FP and FN, the error rate `(FP + FN) / N` and the relative error `(FP + FN) / (TP + FN)`, each with its counts. Writes a diff bitmap to `out/` and prints its color key.

## tmux

tmux keeps long runs going after you disconnect.

```bash
# start a session and enter it
tmux new -s NAME
# start a command in a new session without entering it
tmux new -d -s NAME 'COMMAND; exec bash'
# list sessions
tmux ls
# rejoin a session
tmux attach -t NAME
# end a session and whatever runs in it
tmux kill-session -t NAME
```

| Keys (inside a session) | Action |
|---|---|
| `Ctrl-b` `d` | detach, leaving it running |
| `Ctrl-b` `[` | scroll back; `q` to stop |
| `Ctrl-b` `:` `refresh-client` | redraw a garbled screen |
| `exit` | end the session |

`exec bash` keeps the session open after the command finishes, so its output can still be read.

## perf

perf shows where `vis` spends its time.

```bash
# build with symbols and frame pointers
make -C src/r3 profiling
# record with call stacks
perf record -g -o perf_data/NAME.data ./vis ARGS
# record at a higher sample rate, for short runs
perf record -F 4999 -g -o perf_data/NAME.data ./vis ARGS
# interactive report, sorted by time
perf report -i perf_data/NAME.data
# own time only, without callees
perf report -i perf_data/NAME.data --no-children
# plain text, top 5 functions
perf report -i perf_data/NAME.data --no-children -g none --stdio | grep '%' | head -5
# top 5 source lines
perf report -i perf_data/NAME.data --no-children -g none --sort srcline --stdio | grep '%' | head -5
# source and assembly of one function, with time per line
perf annotate -i perf_data/NAME.data --stdio -l FUNCTION
```

Notes:
- Rebuild with `make` afterwards; the perf build is not for timing.
- The honors computer has two core types, so perf first asks which event to show. Pick `cpu_core/cycles/Pu`.
- `Self` is time in the function's own code; `Children` adds everything it calls. Read `Self` to find hot spots.
- If the menu is garbled in tmux, redraw with `Ctrl-b` `:` `refresh-client`, or rerun with `--stdio`.

| Key (in `perf report`) | Action |
|---|---|
| `↑` `↓` / `j` `k` | move between rows |
| `PgUp` `PgDn` / `Space` | scroll a page |
| `Home` `End` | jump to top or bottom |
| `Enter` | open the menu for the selected row |
| `+` / `e` | expand or collapse its call stack |
| `a` | annotate: source and assembly with time per line |
| `/` | filter rows by name |
| `s` | in annotate, toggle source lines |
| `t` | in annotate, cycle percent, samples and time |
| `Tab` | in annotate, jump to the next hottest line |
| `h` / `?` | help |
| `Esc` / `←` | back one view |
| `q` | quit |

## Row and column to map coordinates

```
X = xllcorner + col * cellsize
Y = yllcorner + (nrows - row) * cellsize
```
