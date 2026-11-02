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

*Side note:*

```
Convert row col to coordinate
Xllcorner + col * size
Yllcorner + (nrows - row) * size
```