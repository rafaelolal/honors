#!/usr/bin/env python3
"""Run named experiments from plans.py, appending one row per phase to OUT.

Save each version of the code first (from this folder):
    make -C ../src/r3 && cp ../src/r3/vis bin/NAME-COMMIT
Then run:
    python3 bench.py EXPERIMENT [EXPERIMENT ...]

Safe to stop and restart: finished runs are skipped. Failed runs are recorded
with their status and retried on restart. Every run also gets a time_cmd row
timing the whole command, even if it failed.

Logs go in logs/, named with the date: DATE-progress.log for what ran when, and
one file per run with the program's full output.
"""
import csv
import hashlib
import itertools
import os
import re
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path

import viewpoints
from plans import PLANS

TIMEOUT = 4 * 3600  # seconds, per run
OUT = Path("results.csv")
LOGS = Path("logs")

KEY = ["experiment", "code", "binary_sha", "dem", "vr", "vc", "vh", "threads",
       "schedule", "rep"]
# about the DEM and the run, the same on every phase row of a run
STATS = ["dem_name", "nrows", "ncols", "cells", "min_h", "max_h", "relief", "nodata",
         "nodata_pct", "viewshed_size", "viewshed_pct", "max_rss_mb"]
COLUMNS = ["timestamp", *KEY, "status", "output_sha", *STATS,
           "phase", "wall", "user", "sys"]

# older binaries print no @stat lines, only these
GRID_LINE = re.compile(r"rows=(\d+),cols=(\d+)\], range=\[(\S+), (\S+)\], "
                       r"NODATA values=(\d+)")
VIEWSHED_LINE = re.compile(r"viewshed size=(\d+)")


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()[:12]


def viewpoints_for(spec, dem):
    """A block's viewpoint list, picking random ones for a Random spec."""
    return viewpoints.sample(dem, spec) if isinstance(spec, viewpoints.Random) else spec


def configs(names):
    # reps are the outer loop, so slow drift overnight doesn't favor one config;
    # a block drops out once its own reps (default 1) are done
    max_reps = max(b.get("reps", 1) for name in names for b in PLANS[name])
    for rep in range(max_reps):
        for name in names:
            for b in PLANS[name]:
                if rep >= b.get("reps", 1):
                    continue
                for dem, code, (vr, vc), vh, threads, schedule in (
                    (dem, *rest) for dem in b["dem"] for rest in itertools.product(
                        b["code"], viewpoints_for(b["viewpoint"], dem), b["height"],
                        b["threads"], b.get("schedule", [""]))
                ):
                    # args isn't in KEY: keep one set of args per experiment
                    yield dict(experiment=name, code=code, binary_sha=sha(code),
                               dem=dem, vr=vr, vc=vc, vh=vh, threads=threads,
                               schedule=schedule, rep=rep,
                               args=[str(a) for a in b.get("args", [])])


def parse_stats(text, dem):
    """DEM and viewshed stats from a run's output; missing ones are left empty."""
    found = dict(l.split()[1:3] for l in text.splitlines() if l.startswith("@stat "))
    if "nrows" not in found and (m := GRID_LINE.search(text)):
        found.update(zip(["nrows", "ncols", "min_h", "max_h", "nodata"], m.groups()))
    if "viewshed_size" not in found and (m := VIEWSHED_LINE.search(text)):
        found["viewshed_size"] = m.group(1)

    stats = dict.fromkeys(STATS, "")
    stats.update({k: v for k, v in found.items() if k in stats})
    stats["dem_name"] = Path(dem).stem
    if found.get("nrows"):
        cells = int(found["nrows"]) * int(found["ncols"])
        data_cells = cells - int(found["nodata"])
        stats["cells"] = cells
        stats["relief"] = f"{float(found['max_h']) - float(found['min_h']):.2f}"
        stats["nodata_pct"] = f"{100 * int(found['nodata']) / cells:.4f}"
        if found.get("viewshed_size") and data_cells:
            stats["viewshed_pct"] = f"{100 * int(found['viewshed_size']) / data_cells:.4f}"
    return stats


def log_path(c):
    parts = [time.strftime("%Y-%m-%d_%H%M%S"), c["experiment"], Path(c["code"]).name,
             Path(c["dem"]).stem, f"r{c['vr']}", f"c{c['vc']}", f"h{c['vh']}",
             f"t{c['threads']}", c["schedule"], f"rep{c['rep']}"]
    return LOGS / ("_".join(p for p in parts if p) + ".log")


def run(c, log):
    env = dict(os.environ, OMP_NUM_THREADS=str(c["threads"]),
               OMP_PROC_BIND="close", OMP_PLACES="cores")
    if c["schedule"]:
        env["OMP_SCHEDULE"] = c["schedule"]
    # CPUs 0-7 are the performance cores; going past 8 threads adds efficiency cores
    cmd = ["taskset", "-c", f"0-{c['threads'] - 1}",
           Path(c["code"]).resolve(), "-e", Path(c["dem"]).resolve(),
           "-r", str(c["vr"]), "-c", str(c["vc"]), "-h", str(c["vh"]), *c["args"]]
    # vis only writes the viewshed grid when given -v NAME
    args = c["args"]
    out_name = args[args.index("-v") + 1] if "-v" in args else None
    # outputs are big, so each run gets a directory that is deleted afterwards;
    # it's on the same disk as the project so write times stay realistic
    with tempfile.TemporaryDirectory(dir=".", prefix="bench-") as tmp, log.open("w") as f:
        start = time.monotonic()
        p = subprocess.Popen(cmd, cwd=tmp, env=env, stdout=f, stderr=subprocess.STDOUT)
        timed_out = threading.Event()

        def kill():
            timed_out.set()
            p.kill()

        timer = threading.Timer(TIMEOUT, kill)
        timer.start()
        # wait4 gives this run's own usage, including its peak memory;
        # taskset execs vis, so the pid is vis itself
        _, wait_status, usage = os.wait4(p.pid, 0)
        timer.cancel()
        wall = time.monotonic() - start
        p.returncode = os.waitstatus_to_exitcode(wait_status)
        if timed_out.is_set():
            status = "timeout"
        else:
            status = f"exit {p.returncode}" if p.returncode else "ok"
        time_cmd = ["time_cmd", f"{wall:.6f}", f"{usage.ru_utime:.6f}",
                    f"{usage.ru_stime:.6f}"]

        output_sha = ""
        if status == "ok" and out_name:
            output = Path(tmp) / "out" / out_name
            if output.exists():
                output_sha = sha(output)
            else:
                status = "no output"

    text = log.read_text(errors="replace")
    phases = [l.split()[1:] for l in text.splitlines() if l.startswith("@time")]
    if status == "ok" and not phases:
        status = "no timing"
    stats = parse_stats(text, c["dem"])
    stats["max_rss_mb"] = f"{usage.ru_maxrss / 1024:.1f}"  # Linux reports KB
    return status, output_sha, stats, [*phases, time_cmd]


def main(names):
    unknown = [n for n in names if n not in PLANS]
    if not names or unknown:
        sys.exit(f"usage: bench.py EXPERIMENT ...\navailable: {', '.join(PLANS)}")
    os.chdir(Path(__file__).resolve().parent)
    LOGS.mkdir(exist_ok=True)
    progress = LOGS / f"{time.strftime('%Y-%m-%d')}-progress.log"

    def say(message):
        line = f"{time.strftime('%Y-%m-%d %H:%M:%S')} {message}"
        print(line, flush=True)
        with progress.open("a") as f:
            f.write(line + "\n")

    done = set()
    if OUT.exists():
        with OUT.open(newline="") as f:
            reader = csv.DictReader(f)
            # appending rows under a different header would shift every column
            if reader.fieldnames != COLUMNS:
                sys.exit(f"{OUT} has different columns than bench.py writes; "
                         "migrate it or move it aside")
            done = {tuple(r[k] for k in KEY) for r in reader if r["status"] == "ok"}

    todo = [c for c in configs(names) if tuple(str(c[k]) for k in KEY) not in done]
    for i, c in enumerate(todo, 1):
        say(f"[{i}/{len(todo)}] {c}")
        status, output_sha, stats, phases = run(c, log_path(c))
        say(f"    {status}, {phases[-1][1]}s")

        new_file = not OUT.exists()
        with OUT.open("a", newline="") as f:
            w = csv.writer(f)
            if new_file:
                w.writerow(COLUMNS)
            stamp = time.strftime("%Y-%m-%d %H:%M:%S")
            for phase in phases:
                w.writerow([stamp, *(c[k] for k in KEY), status, output_sha,
                            *(stats[k] for k in STATS), *phase])


if __name__ == "__main__":
    main(sys.argv[1:])
