#!/usr/bin/env python3
"""Spread of one phase's time over viewpoints, per DEM and code, from results.csv.

    python3 analysis/summarize.py EXPERIMENT [...] [--phase P] [--step S]

Reps of a viewpoint are averaged first (arithmetic mean, since times are
costs), so each viewpoint counts once. Prints n, the median with its 95%
confidence interval, mean, std, min, quartiles, max and the percentiles every
--step% (default 10), and writes them all to analysis/summary-EXPERIMENT.csv.
--step 1 gives enough points to draw the curve of where the times land.

The CI is nonparametric (from ranks, Le Boudec), so it doesn't assume the times
are normal. It needs n >= 6; below that it is left empty.

    python3 analysis/summarize.py EXPERIMENT --baseline CODE

compares every code against CODE (a binary name like r3-keep) on the viewpoints
both ran. It prints the absolute medians next to the speedup. Since both codes
ran the same viewpoints, the test is paired: the 95% CI of the median per-viewpoint
time saved (baseline - code), significant when it excludes 0.
"""
import argparse
import csv
import math
import statistics
from collections import defaultdict
from pathlib import Path

HERE = Path(__file__).resolve().parent
RESULTS = HERE.parent / "results.csv"
GROUP = ["experiment", "code", "dem_name", "vh", "threads", "schedule"]


def median_ci(times, level=0.95):
    """(low, high) of the median's CI from order statistics, or ("", "") if n < 6.

    Picks the widest j with P(Binomial(n, 1/2) < j) <= (1 - level) / 2, so
    [x_j, x_(n-j+1)] covers the median with at least the given level.
    """
    xs, n = sorted(times), len(times)
    tail, j = 0.0, 0
    while True:
        tail += math.comb(n, j) / 2**n
        if tail > (1 - level) / 2:
            break
        j += 1
    if j == 0:
        return "", ""
    return xs[j - 1], xs[n - j]


def summarize(times, step):
    n = len(times)
    lo, hi = median_ci(times)
    row = dict(n=n, median=statistics.median(times), ci_lo=lo, ci_hi=hi,
               mean=statistics.fmean(times),
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
    ap.add_argument("--baseline", help="code name to compare the others against")
    a = ap.parse_args()

    # group -> viewpoint -> times of its reps
    runs = defaultdict(lambda: defaultdict(list))
    with RESULTS.open(newline="") as f:
        for r in csv.DictReader(f):
            if r["experiment"] in a.experiments and r["status"] == "ok" and r["phase"] == a.phase:
                runs[tuple(r[k] for k in GROUP)][(r["vr"], r["vc"])].append(float(r["wall"]))
    if not runs:
        raise SystemExit(f"no ok rows for phase {a.phase} in {', '.join(a.experiments)}")

    if a.baseline:
        return compare(runs, a.baseline, a.phase)

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
        shown = ["n", "median", "ci_lo", "ci_hi", "mean", "std", "min", "p25", "p75", "max"]
    print(f"phase {a.phase}, seconds")
    print(f"{'dem':<22}{'code':<18}" + "".join(f"{c:>9}" for c in shown))
    for r in rows:
        print(f"{r['dem_name']:<22}{Path(r['code']).name:<18}" + "".join(
            f"{r[c]:>9.4f}" if isinstance(r[c], float) else f"{r[c] or '-':>9}"
            for c in shown))
    print("ci_lo/ci_hi: 95% CI of the median; '-' when n < 6")
    print(f"wrote {out.relative_to(HERE.parent)}")


def compare(runs, baseline, phase):
    """Each code vs the baseline, on the viewpoints both ran."""
    code_at = GROUP.index("code")
    # the same setting without the code -> code name -> viewpoint -> mean time
    by_setting = defaultdict(dict)
    for group, points in runs.items():
        setting = group[:code_at] + group[code_at + 1:]
        by_setting[setting][Path(group[code_at]).name] = {
            vp: statistics.fmean(t) for vp, t in points.items()}

    def fmt(times):
        lo, hi = median_ci(times)
        ci = f"[{lo:.4f}, {hi:.4f}]" if lo != "" else "[n < 6]"
        return f"{statistics.median(times):9.4f} {ci:>20}"

    print(f"phase {phase}, seconds. median over viewpoints [95% CI]; "
          f"speedup = baseline median / code median;\n"
          f"saved = median of (baseline - code) per viewpoint [95% CI], "
          f"significant if the CI excludes 0")
    found = False
    for setting, codes in sorted(by_setting.items()):
        if baseline not in codes:
            continue
        found = True
        base = codes[baseline]
        exp, dem, vh, threads, schedule = setting
        print(f"\n{exp}: {dem}, h={vh}, threads={threads}" + (f", {schedule}" if schedule else ""))
        print(f"  {'code':<18}{'n':>4} {'median':>9} {'95% CI':>20} {'speedup':>8} "
              f"{'saved':>9} {'95% CI':>20}  significant")
        for name, pts in sorted(codes.items()):
            common = sorted(base.keys() & pts.keys())
            if not common:
                continue
            b = [base[vp] for vp in common]
            t = [pts[vp] for vp in common]
            if name == baseline:
                print(f"  {name:<18}{len(t):>4} {fmt(t)} {'1.000':>8}  (baseline)")
                continue
            saved = [x - y for x, y in zip(b, t)]
            lo, hi = median_ci(saved)
            if lo == "":
                sig, ci = "n < 6", "[n < 6]"
            else:
                sig = "yes" if lo > 0 or hi < 0 else "no"
                ci = f"[{lo:.4f}, {hi:.4f}]"
            speedup = statistics.median(b) / statistics.median(t)
            print(f"  {name:<18}{len(t):>4} {fmt(t)} {speedup:>8.3f} "
                  f"{statistics.median(saved):9.4f} {ci:>20}  {sig}")
    if not found:
        raise SystemExit(f"no rows for baseline code {baseline}")


if __name__ == "__main__":
    main()
