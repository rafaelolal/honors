Honors project on computing viewsheds over terrain grids, comparing serial, multicore and GPU implementations.

* `src/`: Code, with the viewshed implementations in `r3/` (serial) and `r3_parallel/` (GPU), alongside smaller CUDA exercises such as `collatz/`, `grayscale/`, `mmult/` and `heat/`.
* `experiments/`: The benchmark harness, experiment plans, saved program builds, timing results and analysis.
* `research/`: A symlink to `/home/research`, a shared folder of DEMs and other big files, which is not stored in this repo.
* `literature/`: Viewshed and parallel computing papers, with reading lists and summaries.
* `readings/`: GPU background material, covering foundations, architecture whitepapers, CUDA tuning guides and microbenchmark papers.

## Running experiments in tmux

```bash
# start a session and enter it
tmux new -s NAME             
# list sessions
tmux ls                      
# rejoin a session
tmux attach -t NAME          
# end a session from outside it, stopping whatever runs in it
tmux kill-session -t NAME    
```

Inside a session, press `Ctrl-b` then `d` to detach and leave it running. Typing `exit` ends the session. To scroll back through the output, press `Ctrl-b` then `[`, and `q` to stop scrolling.

To start a command without entering the session, use `tmux new -d -s NAME 'COMMAND; exec bash'`. The `exec bash` keeps the session open when the command finishes, so you can still read its output.

## Profiling with perf

### Recording a profile

```bash
# build with symbols and frame pointers, so perf can name functions and lines
cd src/r3 && make perf
# record with call stacks, writing to a named file
perf record -g -o perf_data/NAME.data ./vis ARGS
# record at a higher sample rate, for short runs
perf record -F 4999 -g -o perf_data/NAME.data ./vis ARGS
```

Rebuild with `make` (no `perf`) afterwards to go back to the fast binary for timing.

### Reading a profile

```bash
# interactive menu, functions sorted by time
perf report -i perf_data/NAME.data
# same, but each function's own time only, without callees added in
perf report -i perf_data/NAME.data --no-children
# plain text, top 5 functions
perf report -i perf_data/NAME.data --no-children -g none --stdio | grep '%' | head -5
# rank source lines instead of functions
perf report -i perf_data/NAME.data --no-children -g none --sort srcline --stdio | grep '%' | head -5
# source and assembly of one function, with time per line
perf annotate -i perf_data/NAME.data --stdio -l FUNCTION
```

The honors computer has two core types, so perf first asks which event to show. Pick `cpu_core/cycles/Pu`.

The `Self` column is time spent in the function's own code, and `Children` adds the time of everything it calls. To find the hot spots, read `Self`.

### Navigating the menu

| Key | Action |
|---|---|
| `↑` `↓` / `j` `k` | move between rows |
| `PgUp` `PgDn` / `Space` | scroll a page |
| `Home` `End` | jump to top or bottom |
| `Enter` | open the menu for the selected row |
| `+` / `e` | expand or collapse its call stack |
| `a` | annotate: show its source and assembly with time per line |
| `/` | filter rows by name |
| `s` | toggle source lines in the annotate view |
| `t` | in annotate, cycle percent, samples and time |
| `Tab` | in annotate, jump to the next hottest line |
| `h` / `?` | help with every key |
| `Esc` / `←` | go back one view |
| `q` | quit |

When the screen gets garbled in tmux, press `Ctrl-b` then `:` and type `refresh-client` to redraw it. If it still looks wrong, quit and run the command again with `--stdio`.

*Side note:*

```
Convert row col to coordinate
Xllcorner + col * size
Yllcorner + (nrows - row) * size
```