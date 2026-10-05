*2024 · Xiao, Deng, Wen & Gu · Parallel algorithm for multi-viewpoint viewshed analysis on the GPU grounded in target cluster segmentation · International Journal of Digital Earth 17(1)*

The paper speeds up shadow-map viewsheds for many viewpoints over terrain and buildings. The CPU divides the area into a uniform grid and groups nearby viewpoints with K-means clustering. Each pixel is then tested only against the viewpoints in its own cluster and neighbouring cells, instead of against all of them. An adaptive depth bias fitted by regression removes most shadow artefacts.

* *Type:* Multi-observer (15-600 viewpoints, not total); approximate (shadow-map depth comparison, not checked against R3); shadow map with grid and K-means clustering.
* *Hardware:* An AMD 5700X with a GeForce GTX 1080 Ti (and an AMD 6850HS laptop with Radeon 680M), using WebGL/GLSL in Cesium, not CUDA.
* *Input / output:* Central Changsha (about 43.6 km2, about 17,000 buildings, 12.5 m ALOS DEM) with up to 600 viewpoints goes in, and a visible/invisible colour per fragment drawn in Cesium comes out (no stored raster).
* *Runtime:* Wall-clock for 600 viewpoints is 150.3 ms versus 162.2 ms for the plain shadow-map method on the GTX 1080 Ti (about 8% less), and it is 0.04-4.3% slower at 15-75 viewpoints.
* *Code:* none found.
* *Gap for us:* It handles at most 600 viewpoints with a roughly 10% gain and no exactness check, leaving millions of exact per-observer CUDA results untouched.

*2024 · Romero et al. · SkewEngine: enhancing performance of intensive calculations on regular meshes · The Journal of Supercomputing 80*

SkewEngine turns the skewed-DEM trick into a reusable C++ template. For each direction it resamples the grid so that points along that direction share a memory row. A user-supplied kernel runs on each independent row, and the result is un-skewed and combined. Directions are spread over CPU threads and GPUs, and four pre-built input variants (normal/transposed, normal/mirrored) avoid the per-angle branching that was costly in the original sDEM.

* *Type:* Total (viewshed case study; also Radon and cepstrum kernels); approximate (180 sectors, bilinear skew); rotational sweep on a skewed mesh.
* *Hardware:* Four machines, from a laptop with an RTX 4050 to an HPC node with 8x V100, plus a server with 4x GTX 980, using OpenMP, CUDA and OpenCL.
* *Input / output:* A 25 M-point Sierra de las Nieves DEM (about 5000x5000 inferred) goes in, and a per-cell visibility map on the same grid comes out.
* *Runtime:* Wall-clock 3.5 s versus 10.2 s for sDEM (about 2.9x, machine not stated), with 59 s in the CPU-only worst case.
* *Code:* https://github.com/luisfromero/skewEngine
* *Gap for us:* It reports no accuracy analysis, no multi-GPU viewshed scaling and only a 25 M-point DEM, so larger grids, exactness against the 180-sector approximation, and memory limits remain open.

*2021 · Chen & Chen · A parallel multipoint viewshed analysis method for urban 3D building scenes · Transactions in GIS 25(4)*

The paper uses the graphics card's 3D rendering pipeline, the same trick games use for shadows, to show what parts of city buildings can be seen from several viewpoints at once. Each viewpoint renders a depth image (shadow map), and a depth test marks which building surface points are visible. The per-viewpoint results are then combined into a cumulative visibility index that grades each area as fully, partly or not visible.

* *Type:* Multi-observer (1-5 viewpoints, 3D building scene, not a DEM); approximate (shadow map, checked only visually); rendering-based.
* *Hardware:* A laptop with an Intel Core i7-9750H and NVIDIA GTX 1650, using OpenGL/GLSL in osgEarth (not CUDA).
* *Input / output:* 200 3D building models over 43.46 km2 in China go in (no DEM grid), and per-viewpoint visibility textures plus a colour-graded cumulative rendering come out.
* *Runtime:* Wall-clock 0.2375 s for 1 viewpoint to 1.4856 s for 5 viewpoints on 20 buildings, and 3.4x to 6.6x faster than a point-to-point line-of-sight method (derived).
* *Code:* none found.
* *Gap for us:* It handles only a handful of viewpoints on a 3D building scene, so it says nothing about viewshed sizes for every cell of a large DEM in CUDA.

*2020 · Qarah · Efficient Viewshed Computation Algorithms On GPUs and CPUs · PhD dissertation, University of South Florida*

The thesis puts the exact "rotating line" viewshed method on a GPU. It does this by turning every cell into events (insert, query, remove), sorting them by angle and distance, and giving each thread a small angular slice of the circle to sweep. A pre-allocated static tree replaces the usual dynamic tree. The grid is then split into sectors that are processed one after another, so that larger maps fit in memory without losing exactness.

* *Type:* Single observer; exact; radial (rotational) sweep with status tree, parallelised on GPU.
* *Hardware:* One NVIDIA Titan V (11 GB) with an AMD Ryzen Threadripper 1950X, CUDA (nvcc 10), Ubuntu 18.04.
* *Input / output:* SRTM grids from 5,000x5,000 to 30,000x30,000 (including Germany at 10,800x10,800) go in, and a per-cell visibility array for one observer comes out.
* *Runtime:* Wall-clock about 1.7 s at 10,800x10,800 (read from a chart), 3.5x faster than GPU-R3 and about 6,200x faster than sequential R3 (about 10,000 s read off a chart).
* *Code:* none found.
* *Gap for us:* At 1.7 s per observer, repeating it for every cell would take about 6 years (derived), and nothing here shares work between observers or uses multiple GPUs.

*2020 · Sánchez-Fernández, Romero, Bandera & Tabik · A data relocation approach for terrain surface analysis on multi-GPU systems: a case study on the total viewshed problem · IJGIS (published 2021)*

The paper rearranges ("skews") the elevation grid once for each viewing direction. After that, each line of sight lies along a single row of memory. Each GPU thread then does a simple forward or backward running-maximum sweep along its row to find the visible area. Directions are independent, so they are spread over several GPUs and the results are summed.

* *Type:* Total; approximate (1.18% absolute difference versus the earlier CPU TVS on the 2k DEM); sector sweep on a skewed DEM (sDEM).
* *Hardware:* Intel Xeon E5-2698 v3 (16 cores) with 4x NVIDIA GTX 980, CUDA 10.0 and OpenMP.
* *Input / output:* Montes de Málaga DEMs of 2000x2000, 4000x4000 and 8000x8000 at 10 m go in, and a same-size total-viewshed matrix (visible area per cell) comes out.
* *Runtime:* Speedup and throughput measured on sectors from 0 to 45 degrees only, with a best of 827.3x (4 GPUs, 4k DEM) over the single-thread CPU TVS algorithm and no full 360-degree wall-clock reported.
* *Code:* yes, figshare 10.6084/m9.figshare.11370549 (preprint arXiv 2003.02200).
* *Gap for us:* It reports no end-to-end timing and holds a skewed DEM (about 2x the DEM) in each GPU, so grids larger than GPU memory, exactness versus R2/R3, and tiling are left open.

*2015 · Feng et al. · A parallel algorithm for viewshed analysis in three-dimensional Digital Earth · Computers & Geosciences 75*

The authors compute a viewshed inside a 3D globe viewer using the graphics pipeline instead of line-of-sight tests. Each terrain or building triangle is stretched away from the viewpoint to form a closed "shadow volume". A stencil buffer then counts how many volumes cover each screen pixel, and pixels with a count of zero are visible. Because this is just rendering, it works on terrain, buildings and trees together and needs no elevation interpolation.

* *Type:* single-observer; approximate (limited by mesh resolution and screen pixels); GPU shadow-volume / stencil-buffer rendering
* *Hardware:* NVIDIA GTX280, GeForce 9800 and GeForce 8600 GPUs (Core2 Q6600/Quad/Duo CPUs), using DirectX 9.0c vertex and pixel shaders in C#.
* *Input / output:* Triangle meshes extracted from a 30 m/6 m global DEM at three levels of detail (6, 2.5 and 0.6 m) in, a red/green visible-invisible screen overlay out.
* *Runtime:* Average time per viewshed, 5.2 ms versus 8.4 ms for a shadow-map GPU method on the GTX280 (about 35% faster on average), and 0.42 to 0.55 ms versus 0.97 to 4.56 ms for a CPU line-of-sight method across the three detail levels.
* *Code:* none found
* *Gap for us:* It is a graphics-pipeline method for one viewpoint on a screen-sized image, so it does not give an exact raster viewshed for every cell and cannot be reused directly in CUDA.

*2014 · Osterman, Benedičič & Ritoša · An IO-efficient parallel implementation of an R2 viewshed algorithm for large terrain maps on a CUDA GPU · IJGIS 28(11)*

Instead of testing every cell, each GPU thread follows one ray from the observer to the map edge. It keeps the highest elevation angle seen so far (a horizon) and marks cells below it as hidden. This is the R2 method, and it accepts small errors in exchange for speed. The map is cut into horizontal strips, and double buffering overlaps disk reads, GPU copies and computation. This lets maps far larger than memory be processed.

* *Type:* Single observer; approximate (99.52-99.89% agreement with R3); R2 ray sweep with horizon vectors.
* *Hardware:* One NVIDIA GTX 680 with an Intel Core i7-3930K, 32 GB RAM and an SSD, CUDA SDK 5.0, Ubuntu 11.04.
* *Input / output:* Ten Slovenia DEMs from 14,081x8,961 (0.48 GB) up to 281,610x179,210 (186 GB, 50.5 billion cells, up-sampled) go in, and a same-size visibility map for one observer comes out.
* *Runtime:* Wall-clock 7 s for 1.89 GB and 1,922 s for 186 GB including disk I/O, 10x over its own sequential version and about 204x to 3,000x over GRASS r.los at 1.89 GB (derived).
* *Code:* Source stated at http://viewshed.s51mo.net (r.cuda.visibility for GRASS GIS), availability not verified.
* *Gap for us:* It uses only about 2(R+C) threads per observer and is approximate, so it neither scales to every cell as an observer nor avoids error accumulating over millions of observers.

*2012 · Osterman · Implementation of the r.cuda.los module in the open source GRASS GIS by using parallel computation on the NVIDIA CUDA graphic cards · Elektrotehniski vestnik 79(1-2)*

The paper rewrites GRASS GIS's r.los module in CUDA to compute a viewshed from one observer. Each GPU thread handles one straight line from the observer to the map edge, and 2*(rows+cols) threads cover the whole map. A thread walks along its line, compares the viewing angle of each cell with the largest angle seen so far, and marks the cell visible if its angle is higher. It also corrects for Earth curvature and can limit the azimuth and tilt for radio planning.

* *Type:* Single observer; approximate (grid walk along lines to the map edge); one thread per line.
* *Hardware:* NVIDIA GeForce GTX 560 Ti (336 CUDA cores, 1 GB) with an Intel Core 2 Duo E8200 CPU, using CUDA.
* *Input / output:* A DEM up to 28161x17921 cells at 12.5 m (all of Slovenia, about 505 million cells, derived) in, a map of the viewing angle at the visible cells out.
* *Runtime:* 18 s for the whole 28161x17921 map, and 1.2 s versus 516 s for r.los (about 430x, derived) at 12.5 m with a 10 km maximum distance.
* *Code:* none found (the kernel is listed in the paper).
* *Gap for us:* It computes only one observer and spends over 90% of its time on disk and PCIe transfers, so a total viewshed needs far more parallelism than one thread per line and a way to keep data on the GPU.

*2012 · Zhao, Padmanabhan & Wang · A parallel computing approach to viewshed analysis of large terrain data using graphics processing units · IJGIS 27(2), 2013*

For one observer, the paper gives every terrain cell its own GPU thread, which checks whether that cell is visible using the exact R3 line-of-sight method. To handle maps larger than memory, it cuts the terrain into coarse tiles that are moved from disk to GPU memory. Smaller sub-tiles are then staged in fast on-chip shared memory so that accesses are regular. A tile-reuse scheme avoids reloading data.

* *Type:* Single observer; exact (R3, differences under 0.00002% from float rounding); R3 line of sight with GPU tiling.
* *Hardware:* One GPU of an NVIDIA Tesla S1070 (GT200, compute capability 1.3) with one 2.4 GHz AMD Opteron core, CUDA SDK 4.0.
* *Input / output:* Seven Utah DEMs from 20,000x20,000 (1.49 GB) to 74,256x74,256 (20.54 GB, about 5.5 billion cells, resampled to 1 m) go in, and a boolean visibility map for one viewpoint comes out.
* *Runtime:* Wall-clock, the best case is 179 s versus 16,917 s for sequential R3 at 1.49 GB (about 94x, derived), with the paper headlining more than 50x for datasets under 4 GB.
* *Code:* none found.
* *Gap for us:* It is a single-viewpoint O(n^3) method taking hours to days per observer, so it does not address producing a viewshed size for every cell.

---

*2022 · Sánchez Fernández · Novel Parallel Approaches to Efficiently Solve Spatial Problems on Heterogeneous CPU-GPU Systems · PhD thesis, Universidad de Málaga*

This PhD thesis covers three spatial problems, and only the total-viewshed part (Chapter 4) and its use for UAV path planning (Chapter 5) matter here. Its method, called sDEM, rotates the terrain one sector at a time and rewrites it as a "skewed" grid so that every cell along a line of sight sits in the same memory row. One GPU thread per observer cell then sweeps along its row, and the results are un-skewed and added up. The sectors are split across several GPUs, which gives large speedups over the earlier single-threaded total-viewshed (TVS) code.

* *Type:* Total; approximate (discrete sectors, 1.18% absolute / 4.21% relative difference vs TVS); rotational sweep on a skewed DEM (sDEM).
* *Hardware:* Intel Xeon E5-2698 v3 (16 cores) with 4x GeForce GTX 980 (Maxwell, 4 GB each), CUDA 10.0, plus OpenMP on the CPU.
* *Input / output:* Input is a 10 m DEM of Montes de Málaga at 2000x2000, 4000x4000 or 8000x8000 cells, and output is one visibility ("heat") value per DEM cell.
* *Runtime:* Wall-clock; best case 827.3x with 4 GPUs on the 4000x4000 DEM (15.86 s vs 13121.14 s) against single-thread TVS, extrapolated from sectors 0-45 degrees.
* *Code:* sDEM on Figshare, https://doi.org/10.6084/m9.figshare.11370549 (single-core, multicore, single-GPU, multi-GPU).
* *Gap for us:* It was tested only up to 64 M cells on old Maxwell GPUs and is approximate, so exactness, bigger DEMs and modern GPUs remain open.

*2019 · Qarah & Tu · A Fast Exact Viewshed Algorithm on GPU · IEEE International Conference on Big Data (Big Data) 2019*

The paper makes the exact "rotating line" viewshed method (radial sweep, as accurate as R3 but far cheaper) run on a GPU for one observer. Every cell creates three events (add, check, remove) that are sorted by angle, so each GPU thread can handle its own slice of the circle. It replaces the usual pointer-based search tree with a pre-allocated static one, and packs three sort keys into one integer, which cuts memory and speeds up sorting. This is the conference version of the Qarah thesis, which extends it to larger maps.

* *Type:* Single observer; exact (same results as sequential radial sweep and R3); GPU radial sweep with merge-path sort, angle histogram, prefix sum and a static binary search tree.
* *Hardware:* NVIDIA Titan V for the GPU code and an AMD Ryzen Threadripper 1950X (16 cores, 2.15 GHz) for the sequential baselines, compiled with NVCC 10.
* *Input / output:* A 10,801 x 10,801 elevation grid of Germany (about 116 million cells, 0.43 GB) plus 5,000² and 10,000² SRTM grids in, and a visible/hidden value for every cell out.
* *Runtime:* 1.7 s on the 10,801² grid, about 3.5x faster than GPU R3 and about 6,200x faster than sequential R3 (just under 3 hours); the abstract says 1,627x over sequential radial sweep, but the body says 1,980x for that comparison and 1,627x for GPU R3 over CPU R3, so the abstract figure looks mislabeled.
* *Code:* none found.
* *Gap for us:* It computes one observer at a time and keeps about 5.9 GB of intermediate data for a 116-million-cell grid on one GPU, so repeating it for every cell is impractical and a total viewshed needs a different way of sharing work.

*2015 · Cauchi-Saunders & Lewis · GPU enabled XDraw viewshed analysis · Journal of Parallel and Distributed Computing 84*

The paper ports XDraw, the fast approximate method that grows outward ring by ring from the observer, to the GPU. Each point on a ring depends only on the next ring inward, so each ring point becomes one GPU thread. The authors reduce branching, stage ring data in per-multiprocessor buffers and merge the eight octant kernels into two. They compare it with GPU and CPU versions of R3, R2 and DDA on small Tasmanian grids.

* *Type:* Single-observer; approximate (XDraw, tile-edge artifacts, accuracy not quantified); expanding square rings.
* *Hardware:* AMD "6870" GPU (presumably Radeon HD 6870, derived) with an Intel i5-3570 and 12 GB RAM, using C++ AMP (DirectX 11), not CUDA.
* *Input / output:* Five Tasmania DEMs from 0.8 M to 9.1 M points in, and a visibility array for one observer out.
* *Runtime:* Wall-clock average 0.0375 s for the optimized XDraw, about 3.5x faster than plain GPU XDraw and about 17.7x faster than CPU XDraw (both derived), excluding host-device transfers.
* *Code:* none found.
* *Gap for us:* It covers only tiny single-observer grids with unquantified accuracy, so it gives no evidence that XDraw can be accurate and scalable enough for a total viewshed.

*2015 · Axell & Fridén · Comparison between GPU and parallel CPU optimizations in viewshed analysis · MSc thesis, Chalmers University of Technology*

This thesis asks whether a CPU or a GPU is better at computing a viewshed (the area visible from one observer point on a terrain grid). The authors implement the R2 method, which sends a line of sight from the observer to every cell on the border of a square and tracks the highest slope seen along each line. They write an OpenCL GPU version and an 8-thread, SIMD-vectorized CPU version (using the ISPC compiler), then tune both, for example by reusing GPU memory and overlapping data transfers with computation using two command queues.

* *Type:* Single-observer; approximate (R2 sightlines, error measured against an exact R3 implementation); line-of-sight/sightline family (R2, with R3 as reference).
* *Hardware:* Intel Core i7-3610QM at 2.3 GHz (AVX, 8 GB RAM) and NVIDIA GTX 660M, using OpenCL.
* *Input / output:* A 10 m resolution DEM of the Lake Tahoe area (up to a 16001x16001 raster) with one observer in, a visibility raster out.
* *Runtime:* Time per viewshed (absolute times not stated in the text); at 80 km range (16001x16001) the GPU is about 3.1x faster than the vectorized multithreaded CPU, 29.3x faster than sequential CPU, and the parallel CPU is 9.4x faster than sequential CPU.
* *Code:* none found.
* *Gap for us:* It computes only one observer per run and uses OpenCL on a 2012-era laptop GPU, so it gives no total-viewshed (all observers) method and no CUDA or modern-GPU numbers.

*2011 · Chao et al. · Parallel Algorithm for Viewshed Analysis on a Modern GPU · International Journal of Digital Earth*

This paper treats the observer as a light source and uses shadow mapping, a standard 3D graphics trick, to find what can be seen. The scene is projected onto a sphere around the viewpoint, flattened into a 2D depth texture with a stereographic projection, and each rendered terrain pixel is compared with the stored depth. Pixels farther than the stored depth are in shadow, which means not visible. All of this runs in vertex and pixel shaders as part of normal 3D rendering, so the viewshed is drawn directly on the terrain in real time.

* *Type:* Single viewpoint; approximate (limited by shadow-map resolution and projection precision); shadow mapping on the graphics pipeline (not CUDA).
* *Hardware:* Consumer PCs with Intel Core2 Q6600 + NVIDIA GTX280, Core2 Quad 2.5 GHz + GF9800, and Core2 Duo 2.4 GHz + GF8600, using the DirectX 9.0c API with vertex and pixel shaders.
* *Input / output:* A 3D terrain (DEM-based rendering data) and a viewpoint go in, and a rendered 3D scene with invisible areas shaded comes out (an image, not a stored visibility raster).
* *Runtime:* Frame time ("overhead") of 8.4 ms on the GTX280, 16 ms on the GF9800 and 36.3 ms on the GF8600, and about 60 frames per second at scope 5000 (the paper gives no CPU baseline or speedup figure).
* *Code:* none found.
* *Gap for us:* The output is a rendered picture for one viewpoint rather than a per-cell visibility result, and accuracy drops with distance, so it does not give an exact, storable total viewshed.

*2011 · Gao et al. · Optimization for Viewshed Analysis on GPU · IEEE conference proceedings, 2011 (conference name not stated in the PDF text)*

The authors port the standard exact viewshed method to CUDA. For each target cell it checks the line of sight to the observer, interpolating elevations where the line crosses grid lines. Each cell's visibility is independent, so one thread handles one cell. To make this run well, they put the DEM in texture memory, the observer data in constant memory and the results in global memory. Their "Comb" traversal splits the work into x-direction and y-direction passes so threads in a block run the same instructions with little branching. The result matches the original method exactly.

* *Type:* Single viewpoint; exact (basic bilinear-interpolation line-of-sight, O(N^3) for an N by N grid); CUDA, one thread per target cell.
* *Hardware:* Intel Core2 Quad Q9550 at 2.83 GHz CPU and NVIDIA GeForce GT 240 (128 stream processors) with CUDA 2.3.
* *Input / output:* A grid DEM (2847 x 3717 cells, or 366 x 372 cells) and one viewpoint go in, and a visibility matrix comes out.
* *Runtime:* Wall-clock time; on the large DEM 656 ms on the GPU vs 46,390 ms on the CPU (about 70x, derived), and 47 ms vs 797 ms on the small DEM (about 17x, derived), both vs the same exact method on CPU.
* *Code:* none found.
* *Gap for us:* It is a one-viewpoint exact kernel whose DEM must fit in GPU memory (the authors name partitioning as future work), so it gives a per-observer building block but no way to scale to every cell as an observer.
