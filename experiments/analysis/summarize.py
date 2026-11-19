#!/usr/bin/env python3
"""Spread of one phase's time over viewpoints, per DEM and code, from results.csv.

    python3 analysis/summarize.py EXPERIMENT [...] [--phase P] [--step S]

Reps of a viewpoint are averaged first, so each viewpoint counts once. Prints
n, mean, std, min, quartiles, max and the percentiles every --step% (default 10),
and writes them all to analysis/summary-EXPERIMENT.csv. --step 1 gives enough
points to draw the curve of where the times land.
"""
import argparse
import csv
import statistics
from collections import defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
RESULTS = HERE.parent / "results.csv"
GROUP = ["experiment", "code", "dem_name", "vh", "threads", "schedule"]


def summarize(times, step):
    n = len(times)
    row = dict(n=n, mean=statistics.fmean(times),
               std=statistics.stdev(times) if n > 1 else 0.0, min=min(times))
    # inclusive: p0 is the min and p100 the max, so it reads as an ECDF
    q = statistics.quantiles(times, n=100, method="inclusive") if n > 1 else [times[0]] * 99
    row.update({f"p{p}": q[p - 1] for p in sorted({*range(step, 100, step), 25, 50, 75})})
    row["max"] = max(times)
    return row


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("experiments", nargs="+")
    ap.add_argument("--phase", default="compute_viewshed_grid",
                    help="e.g. TOTAL or time_cmd for the whole run")
    ap.add_argument("--step", type=int, default=10, help="percentile spacing, in %%")
    a = ap.parse_args()

    # group -> viewpoint -> times of its reps
    runs = defaultdict(lambda: defaultdict(list))
    with RESULTS.open(newline="") as f:
        for r in csv.DictReader(f):
            if r["experiment"] in a.experiments and r["status"] == "ok" and r["phase"] == a.phase:
                runs[tuple(r[k] for k in GROUP)][(r["vr"], r["vc"])].append(float(r["wall"]))
    if not runs:
        raise SystemExit(f"no ok rows for phase {a.phase} in {', '.join(a.experiments)}")

    rows = []
    for group, points in sorted(runs.items()):
        times = sorted(statistics.fmean(t) for t in points.values())
        rows.append(dict(zip(GROUP, group)) | summarize(times, a.step))

    cols = list(rows[0])
    out = HERE / f"summary-{'+'.join(a.experiments)}.csv"
    with out.open("w", newline="") as f:
        w = csv.DictWriter(f, cols)
        w.writeheader()
        w.writerows(rows)

    # every column fits at a 10% step; finer steps only show the quartiles
    shown = [c for c in cols if c not in GROUP]
    if a.step < 10:
        shown = ["n", "mean", "std", "min", "p25", "p50", "p75", "max"]
    print(f"phase {a.phase}, seconds")
    print(f"{'dem':<22}{'code':<18}" + "".join(f"{c:>9}" for c in shown))
    for r in rows:
        print(f"{r['dem_name']:<22}{Path(r['code']).name:<18}" + "".join(
            f"{r[c]:>9.4f}" if isinstance(r[c], float) else f"{r[c]:>9}" for c in shown))
    print(f"wrote {out.relative_to(HERE.parent)}")


if __name__ == "__main__":
    main()
