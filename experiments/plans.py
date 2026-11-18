"""What to run. Each named experiment is a list of blocks, and each block runs
every combination of its lists. Paths are relative to this folder.

schedule is optional and needs schedule(runtime) in the code.

args is optional: extra arguments passed to vis after -e/-r/-c/-h. vis only
computes by default; ["-v", "vis.asc"] also writes the grid (and fills
output_sha), and ["-b"] writes the bitmaps. Keep one args per experiment,
since args isn't part of the key that marks a run as done.
"""
from pathlib import Path

SET1 = "../research/DEM/set1.asc"
SIERRA = "../research/DEM/sierra-30.asc"

# smallest first; the last four got 3 viewpoints instead of 10
TIMING_DEMS = [
    "flat", "wall", "set1", "maine-brunsdem-26m", "_hemisphere",
    "maine-islanddem-2m", "kaweah-30m", "sierra-30", "maine-southport-2m",
    "oahu-5m", "maine-bremen-2m", "hawaii-10m", "maine", "maine-portland-10m",
    "maine-newMeadows-1m", "iowa-1m", "washington-10m", "maine-lincoln-2m",
    "maine-knox-2m", "mtrainier-0.6m",
]

# bin/serial predates the compute-only default and requires -v
OLD_BINARY_ARGS = ["-v", "vis.asc"]


def picked_viewpoints(name):
    """The valid points run_serial_timing.sh picked; none until it reaches this DEM."""
    path = Path(__file__).parent / "serial_timing" / "viewpoints" / f"{name}.txt"
    if not path.exists():
        return []
    return [tuple(map(int, line.split())) for line in path.read_text().splitlines()]


# Block keys. Lists are crossed, so each combination is one run.
#   code      list of binary paths (sha goes in the key)
#   dem       list of .asc paths
#   viewpoint list of (row, col) tuples
#   height    list of heights above terrain, passed as -h
#   threads   list of ints: sets OMP_NUM_THREADS, pins to cores 0..n-1
#   schedule  optional list of OMP_SCHEDULE strings, default [""] (unset)
#   args      optional flat list of extra vis args, default [] (not crossed;
#             not in the key)
#   reps      optional int, times each combination runs, default 1

PLANS = {
    "worst-case-test": [
        dict(code=["bin/two_bit"], dem=[f"../research/DEM/{name}.asc"], viewpoint=points, height=[0], threads=[1])
        for name, points in [("maine-knox-2m", [(0, 22996)]), ("washington-10m", [(30895, 5833)]), ("mtrainier-0.6m", [(22385, 4904)]), ("washington-10m", [(0, 10514)])]
    ],
    "serial-timing-2": [
        dict(code=["bin/serial"], dem=[f"../research/DEM/{name}.asc"],
             viewpoint=picked_viewpoints(name), height=[0], threads=[1],
             args=OLD_BINARY_ARGS)
        for name in TIMING_DEMS
    ],
    "serial-baseline": [
        dict(code=["bin/serial"], dem=[SET1], viewpoint=[(250, 250)],
             height=[10], threads=[1], args=OLD_BINARY_ARGS),
        dict(code=["bin/serial"], dem=[SIERRA], viewpoint=[(1500, 1500)],
             height=[10], threads=[1], args=OLD_BINARY_ARGS),
    ],
}


# args example: save the grid and bitmaps for one run, to look at or diff
    # "set1-outputs": [
    #     dict(code=["bin/serial-COMMIT"], dem=[SET1], viewpoint=[(250, 250)],
    #          height=[0, 10], threads=[1], args=["-v", "vis.asc", "-b"]),
    # ],

# Open MP example:
    # "omp-scaling": [
    #     dict(code=["bin/omp-COMMIT"], dem=[SIERRA], viewpoint=[(1500, 1500)],
    #          height=[10], threads=[1, 2, 4, 8, 16, 24],
    #          schedule=["static", "dynamic,64"]),
    # ],