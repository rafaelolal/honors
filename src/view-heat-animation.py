#!/usr/bin/env python3
import argparse
import glob
import os
import re

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation


def frame_index(path):
    match = re.search(r"heat_(\d+)\.bin$", path)
    return int(match.group(1)) if match else -1


def load_frame(path):
    with open(path, "rb") as f:
        height, width = np.fromfile(f, dtype=np.int32, count=2)
        data = np.fromfile(f, dtype=np.float32, count=height * width)
        return data.reshape((height, width))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--mp4", action="store_true", help="save as .mp4 instead of .gif (needs ffmpeg)")
    parser.add_argument("--show", action="store_true", help="open an interactive window instead of saving")
    parser.add_argument("--pattern", default="frames/heat_*.bin", help="glob pattern for frame files")
    parser.add_argument("--out", default=None, help="output file path")
    args = parser.parse_args()

    files = sorted(glob.glob(args.pattern), key=frame_index)
    empty = [f for f in files if os.path.getsize(f) < 8]
    if empty:
        print(f"Skipping {len(empty)} empty frame files")
        files = [f for f in files if f not in empty]
    if not files:
        raise SystemExit(f"No frame files found matching {args.pattern}")

    print(f"Found {len(files)} frames")

    fig = plt.figure(figsize=(8, 8))
    fig.tight_layout()

    img = plt.imshow(load_frame(files[0]), cmap="hot", interpolation="none", vmin=10)
    plt.colorbar(img, label="Temperature")

    def drawframe(i):
        img.set_data(load_frame(files[i]))
        return (img,)

    ani = animation.FuncAnimation(fig, drawframe, frames=len(files), interval=50, blit=True)

    if args.show:
        plt.show()
        return

    out = args.out or ("heat-animation.mp4" if args.mp4 else "heat-animation.gif")
    if args.mp4 or out.endswith(".mp4"):
        ani.save(out, writer=animation.FFMpegWriter(fps=20))
    else:
        ani.save(out, writer=animation.PillowWriter(fps=20))
    plt.close(fig)
    print(f"Saved animation to {os.path.abspath(out)}")


if __name__ == "__main__":
    main()
