"""Random valid viewpoints for plans: viewpoint=Random(k=30) or Random(pct=10).

Points are uniform over the DEM's data cells (rejection sampling, nodata redrawn),
with a fixed seed, and saved to serial_timing/viewpoints/ so reruns of a plan get
the same points. Assumes one grid row per line after the header, like vis.
"""
import random
from array import array
from dataclasses import dataclass
from pathlib import Path

CACHE = Path(__file__).parent / "serial_timing" / "viewpoints"
CHUNK = 64 << 20


@dataclass(frozen=True)
class Random:
    k: int | None = None    # number of points
    pct: float | None = None  # or a percentage of all cells (nrows * ncols)
    seed: int = 0

    def __post_init__(self):
        if (self.k is None) == (self.pct is None):
            raise ValueError("Random needs exactly one of k or pct")


def read_header(f):
    """Header fields, lowercased, and the number of header lines."""
    header = {}
    f.seek(0)
    while True:
        pos = f.tell()
        parts = f.readline().split()
        if not parts or not parts[0][:1].isalpha():
            f.seek(pos)
            return header, len(header)
        header[parts[0].decode().lower()] = parts[1].decode()


def row_starts(dem, nrows, nheader):
    """Byte offset of each grid row, from one pass over the file, cached."""
    size = dem.stat().st_size
    cache = CACHE / f"{dem.stem}.rowidx"
    starts = array("q")
    if cache.exists():
        starts.frombytes(cache.read_bytes())
        if starts and starts[0] == size:  # first entry is the DEM's size, to spot changes
            return starts[1:]
        starts = array("q")

    print(f"indexing rows of {dem.name} (once per DEM)...", flush=True)
    lines = array("q", [0])
    with dem.open("rb") as f:
        base = 0
        while chunk := f.read(CHUNK):
            i = chunk.find(b"\n")
            while i != -1:
                lines.append(base + i + 1)
                i = chunk.find(b"\n", i + 1)
            base += len(chunk)
    if lines[-1] == size:  # trailing newline, not the start of a line
        lines.pop()
    starts = lines[nheader:]
    if len(starts) != nrows:
        raise SystemExit(f"{dem}: {len(starts)} data lines but nrows={nrows}; "
                         "random viewpoints need one row per line")
    CACHE.mkdir(parents=True, exist_ok=True)
    cache.write_bytes((array("q", [size]) + starts).tobytes())
    return starts


def sample(dem, spec):
    """List of (row, col) valid viewpoints for a Random spec, cached per DEM/k/seed."""
    dem = Path(dem)
    with dem.open("rb") as f:
        header, nheader = read_header(f)
        nrows, ncols = int(header["nrows"]), int(header["ncols"])
        k = spec.k if spec.k is not None else max(1, round(spec.pct / 100 * nrows * ncols))
        out = CACHE / f"{dem.stem}-k{k}-s{spec.seed}.txt"
        if out.exists():
            return [tuple(map(int, l.split())) for l in out.read_text().splitlines()]

        starts = row_starts(dem, nrows, nheader)
        # tolerance since vis stores floats, and maine.asc's nodata is -3.4e38
        nodata = float(header["nodata_value"])
        eps = abs(nodata) * 1e-6
        rng = random.Random(spec.seed)
        picked, seen = [], set()
        for _ in range(100 * k):
            if len(picked) == k:
                break
            r, c = rng.randrange(nrows), rng.randrange(ncols)
            if (r, c) in seen:
                continue
            f.seek(starts[r])
            fields = f.readline().split()
            if len(fields) != ncols:
                raise SystemExit(f"{dem}: row {r} has {len(fields)} fields, not {ncols}")
            seen.add((r, c))
            if abs(float(fields[c]) - nodata) > eps:
                picked.append((r, c))
        else:
            if len(picked) < k:
                raise SystemExit(f"{dem}: only {len(picked)} of {k} valid points "
                                 f"after {100 * k} draws")

    print(f"picked {k} random viewpoints on {dem.name} -> {out.name}", flush=True)
    out.write_text("".join(f"{r} {c}\n" for r, c in picked))
    return picked
