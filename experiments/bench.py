#!/usr/bin/env python3
"""Run named experiments from plans.py, appending one row per phase to OUT.

Save each version of the code first (from this folder):
    make -C ../src/r3 && cp ../src/r3/vis bin/NAME-COMMIT
Then run:
    python3 bench.py EXPERIMENT [EXPERIMENT ...]

Safe to stop and restart: finished runs are skipped. Failed runs are recorded
with their status and retried on restart.
"""
import csv
import hashlib
import itertools
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path

from plans import PLANS

REPS = 5
TIMEOUT = 4 * 3600  # seconds, per run
OUT = Path("results.csv")

KEY = ["experiment", "code", "binary_sha", "dem", "vr", "vc", "vh", "threads",
       "schedule", "rep"]
COLUMNS = ["timestamp", *KEY, "status", "output_sha", "phase", "wall", "user", "sys"]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()[:12]


def configs(names):
    # reps are the outer loop, so slow drift overnight doesn't favor one config
    for rep in range(REPS):
        for name in names:
            for b in PLANS[name]:
                for code, dem, (vr, vc), vh, threads, schedule in itertools.product(
                    b["code"], b["dem"], b["viewpoint"], b["height"], b["threads"],
                    b.get("schedule", [""]),
                ):
                    yield dict(experiment=name, code=code, binary_sha=sha(code),
                               dem=dem, vr=vr, vc=vc, vh=vh, threads=threads,
                               schedule=schedule, rep=rep)


def run(c):
    env = dict(os.environ, OMP_NUM_THREADS=str(c["threads"]),
               OMP_PROC_BIND="close", OMP_PLACES="cores")
    if c["schedule"]:
        env["OMP_SCHEDULE"] = c["schedule"]
    # CPUs 0-7 are the performance cores; going past 8 threads adds efficiency cores
    cmd = ["taskset", "-c", f"0-{c['threads'] - 1}",
           Path(c["code"]).resolve(), "-e", Path(c["dem"]).resolve(), "-v", "vis.asc",
           "-r", str(c["vr"]), "-c", str(c["vc"]), "-h", str(c["vh"])]
    # outputs are big, so each run gets a directory that is deleted afterwards;
    # it's on the same disk as the project so write times stay realistic
    with tempfile.TemporaryDirectory(dir=".", prefix="bench-") as tmp:
        try:
            p = subprocess.run(cmd, cwd=tmp, env=env, capture_output=True, text=True,
                               timeout=TIMEOUT)
        except subprocess.TimeoutExpired:
            return "timeout", "", []
        if p.returncode:
            return f"exit {p.returncode}", "", []
        output = Path(tmp) / "out" / "vis.asc"
        if not output.exists():
            return "no output", "", []
        phases = [l.split()[1:] for l in p.stdout.splitlines() if l.startswith("@time")]
        return ("ok" if phases else "no timing"), sha(output), phases


def main(names):
    unknown = [n for n in names if n not in PLANS]
    if not names or unknown:
        sys.exit(f"usage: bench.py EXPERIMENT ...\navailable: {', '.join(PLANS)}")
    os.chdir(Path(__file__).resolve().parent)

    done = set()
    if OUT.exists():
        with OUT.open(newline="") as f:
            done = {tuple(r[k] for k in KEY) for r in csv.DictReader(f)
                    if r["status"] == "ok"}

    todo = [c for c in configs(names) if tuple(str(c[k]) for k in KEY) not in done]
    for i, c in enumerate(todo, 1):
        print(f"[{i}/{len(todo)}] {time.strftime('%H:%M:%S')} {c}", flush=True)
        status, output_sha, phases = run(c)
        print(f"    {status}", flush=True)

        new_file = not OUT.exists()
        with OUT.open("a", newline="") as f:
            w = csv.writer(f)
            if new_file:
                w.writerow(COLUMNS)
            stamp = time.strftime("%Y-%m-%d %H:%M:%S")
            for phase in phases or [["-", "", "", ""]]:
                w.writerow([stamp, *(c[k] for k in KEY), status, output_sha, *phase])


if __name__ == "__main__":
    main(sys.argv[1:])
