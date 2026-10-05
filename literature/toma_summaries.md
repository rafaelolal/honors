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

*2023 · Dong & Zhang · A Multi-Level Distributed Computing Approach to XDraw Viewshed Analysis Using Apache Spark · Remote Sensing 15(3), 761*

ML-XDraw spreads the XDraw viewshed algorithm, which works outward from the observer in square rings, across a cluster of machines. It cuts the terrain into nested rings and splits each ring into equal-area pieces small enough for one worker's memory. Each ring's results are handed to the next ring through a shared store, and a trick near piece boundaries recovers cells that would otherwise be lost. Spark handles the scheduling.

* *Type:* single-observer (huge grids); approximate (XDraw, about 96.5 to 98% of cells match serial XDraw); rings-outward XDraw, distributed
* *Hardware:* A 7-node Apache Spark 2.4.7 cluster (96 CPU cores, 384 GB RAM, 64 executors with 1 core each), compared with a 48-core, 96 GB workstation running serial XDraw, in Scala (no GPU).
* *Input / output:* Resampled Australia DEMs from 4.5 to 28.2 billion cells (16.8 to 105 GB) in, a visibility bitmap out.
* *Runtime:* Wall-clock minutes, best speedup 13.15x on a 18.1-billion-cell DEM (195.8 min down to 14.89 min) versus serial XDraw on one workstation.
* *Code:* none found
* *Gap for us:* It scales one observer across machines and says GPU integration is future work, so it gives no GPU kernel and no all-observers computation.

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

*2019 · Wu et al. · HiViewshed: An Interactive Online Viewshed Analysis System for Multiple Observers · CSAE 2019 (ACM)*

HiViewshed is a web service that returns viewsheds for hundreds of observer points at once. A task scheduler gives each observer to a separate MPI process, and inside each process OpenMP threads split the area into equal angle slices. A pyramid of lower-resolution elevation tiles lets the system read coarser data when the map is zoomed out, which cuts the work. The per-observer viewshed uses the standard ray-and-maximum-slope method.

* *Type:* multi-observer (hundreds of chosen points, not every cell, though the abstract calls it "total viewshed"); line-of-sight max-slope (Bresenham rays), accuracy not stated; hybrid MPI + OpenMP task parallelism
* *Hardware:* One SMP server with Intel Xeon E5-4620 CPUs (16 cores in total, 192 GB RAM), using C++ with MPI and OpenMP (no GPU).
* *Input / output:* A 30 m SRTM raster of China (grid size not stated) and up to 512 observers in, a visible-area map for each observer out.
* *Runtime:* Average time per observer; the 300 km radius took only about 2x as long as the 100 km radius, and speed stopped improving past 16 processes, with no absolute best time or speedup given in the text.
* *Code:* none found
* *Gap for us:* It parallelizes over observers on CPUs with no GPU, uses a single-observer kernel, and reports no timings against any baseline, so all-cells total viewshed on a GPU is untouched.

*2015 · Feng et al. · A parallel algorithm for viewshed analysis in three-dimensional Digital Earth · Computers & Geosciences 75*

The authors compute a viewshed inside a 3D globe viewer using the graphics pipeline instead of line-of-sight tests. Each terrain or building triangle is stretched away from the viewpoint to form a closed "shadow volume". A stencil buffer then counts how many volumes cover each screen pixel, and pixels with a count of zero are visible. Because this is just rendering, it works on terrain, buildings and trees together and needs no elevation interpolation.

* *Type:* single-observer; approximate (limited by mesh resolution and screen pixels); GPU shadow-volume / stencil-buffer rendering
* *Hardware:* NVIDIA GTX280, GeForce 9800 and GeForce 8600 GPUs (Core2 Q6600/Quad/Duo CPUs), using DirectX 9.0c vertex and pixel shaders in C#.
* *Input / output:* Triangle meshes extracted from a 30 m/6 m global DEM at three levels of detail (6, 2.5 and 0.6 m) in, a red/green visible-invisible screen overlay out.
* *Runtime:* Average time per viewshed, 5.2 ms versus 8.4 ms for a shadow-map GPU method on the GTX280 (about 35% faster on average), and 0.42 to 0.55 ms versus 0.97 to 4.56 ms for a CPU line-of-sight method across the three detail levels.
* *Code:* none found
* *Gap for us:* It is a graphics-pipeline method for one viewpoint on a screen-sized image, so it does not give an exact raster viewshed for every cell and cannot be reused directly in CUDA.

*2014 · Tabik et al. · Efficient Data Structure and Highly Scalable Algorithm for Total-Viewshed Computation · venue not stated (IEEE-format preprint)*

Instead of running one viewshed per observer, this paper computes the viewshed of every cell in the terrain together. For each of S directions it sweeps across the grid in the perpendicular direction, keeping a narrow "band of sight" of about sqrt(N) cells around each observer in a small doubly linked list. Each step of the sweep deletes one cell and inserts one, so the list is reused instead of rebuilt. The list is small enough to fit in the CPU's L1 cache, which makes the sweep very fast, and it supports observers raised above the ground.

* *Type:* Total viewshed; approximate (band-of-sight sampling, S directions); sweep with a linked-list profile.
* *Hardware:* Intel Core 2 Duo E8500 CPU, no GPU.
* *Input / output:* 2000x2000-cell DEMs at 10 m (4 million observers each) in, a visible area per observer out (the data structure is stored per band, not as a full N x N matrix).
* *Runtime:* 0.0032 s per viewpoint (total time divided by 4 million), versus 10 s for ArcMap Viewshed and 18 s for GRASS r.viewshed on one viewpoint, about 3,000x to 5,600x (derived).
* *Code:* www.ac.uma.es/~siham/tvs.tar.gz (link given in the paper, not checked).
* *Gap for us:* It is CPU-only and its linked list is updated sequentially, so a CUDA version must redesign the sweep so that many sectors or many sweep lines run in parallel.

*2014 · Ferreira et al. · A Parallel Algorithm for Viewshed Computation on Grid Terrains · Journal of Information and Data Management 5(2)*

The authors make Van Kreveld's sweep-line viewshed algorithm run on several CPU cores. That algorithm normally cannot be split up, because each cell's answer depends on all earlier cells as a ray rotates around the observer. They cut the area around the observer into pie-slice sectors, give each sector its own list of events and its own search tree, and process the sectors on separate threads. A few cells near sector edges get computed twice, which does not change the result.

* *Type:* single-observer; exact (same accuracy as Van Kreveld/R3); rotational sweep line (parallelized over sectors)
* *Hardware:* Dual Intel Xeon E5-2687 3.1 GHz (16 cores total) and an Intel Core i5-3330 (4 cores), using C++ and OpenMP.
* *Input / output:* SRTM grids from 5000x5000 to 30000x30000 cells (up to 3.35 GiB) in, one viewshed bitmap for a centered observer out.
* *Runtime:* Wall-clock viewshed time only, best speedup 12.0x on 16 threads (24 s to 2 s on 5000x5000) versus the serial Van Kreveld implementation.
* *Code:* none found
* *Gap for us:* It computes one observer on CPU threads and says a GPU version is hard because the balanced tree needs pointers, so it gives no GPU method and no way to compute all observers.

*2014 · Osterman, Benedičič & Ritoša · An IO-efficient parallel implementation of an R2 viewshed algorithm for large terrain maps on a CUDA GPU · IJGIS 28(11)*

Instead of testing every cell, each GPU thread follows one ray from the observer to the map edge. It keeps the highest elevation angle seen so far (a horizon) and marks cells below it as hidden. This is the R2 method, and it accepts small errors in exchange for speed. The map is cut into horizontal strips, and double buffering overlaps disk reads, GPU copies and computation. This lets maps far larger than memory be processed.

* *Type:* Single observer; approximate (99.52-99.89% agreement with R3); R2 ray sweep with horizon vectors.
* *Hardware:* One NVIDIA GTX 680 with an Intel Core i7-3930K, 32 GB RAM and an SSD, CUDA SDK 5.0, Ubuntu 11.04.
* *Input / output:* Ten Slovenia DEMs from 14,081x8,961 (0.48 GB) up to 281,610x179,210 (186 GB, 50.5 billion cells, up-sampled) go in, and a same-size visibility map for one observer comes out.
* *Runtime:* Wall-clock 7 s for 1.89 GB and 1,922 s for 186 GB including disk I/O, 10x over its own sequential version and about 204x to 3,000x over GRASS r.los at 1.89 GB (derived).
* *Code:* Source stated at http://viewshed.s51mo.net (r.cuda.visibility for GRASS GIS), availability not verified.
* *Gap for us:* It uses only about 2(R+C) threads per observer and is approximate, so it neither scales to every cell as an observer nor avoids error accumulating over millions of observers.

*2013 · Li, Franklin & Benedetti · Parallel Multiple Observer Siting on Terrain · short 2-page version, venue not stated in the PDF (an earlier, shorter version of the 2014 ACM SIGSPATIAL paper of the same title)*

The paper speeds up a program that picks places for many observers (such as lookout towers) so that together they see as much terrain as possible. The program has four stages: estimate how visible each point is from random targets, pick candidate observers, compute each candidate's viewshed, and greedily choose the ones adding the most new area. Each stage is parallelised with OpenMP on the CPU and with CUDA on the GPU (one thread per point, or one thread block per observer).

* *Type:* Multi-observer siting (with an approximate sampled visibility index per cell); approximate; viewshed plus visibility index plus greedy cover, with OpenMP and CUDA.
* *Hardware:* Dual Intel Xeon E5-2687 (16 cores, 32 threads, 128 GB RAM) and one NVIDIA Tesla K20x (2688 cores, 6 GB), with OpenMP and CUDA.
* *Input / output:* Puget Sound terrains from 1025x1025 to 16385x16385 go in, and a set of selected observer positions comes out.
* *Runtime:* Wall-clock total 5033 s sequential versus 312.5 s CUDA and 317.1 s OpenMP at 16385x16385, about 16x for both (CUDA 16.1x, OpenMP 15.9x).
* *Code:* none found.
* *Gap for us:* The visibility index is a random-sampled estimate used only to choose sites, so exact all-cells visibility on a GPU is left open.

*2012 · Osterman · Implementation of the r.cuda.los module in the open source GRASS GIS by using parallel computation on the NVIDIA CUDA graphic cards · Elektrotehniski vestnik 79(1-2)*

The paper rewrites GRASS GIS's r.los module in CUDA to compute a viewshed from one observer. Each GPU thread handles one straight line from the observer to the map edge, and 2*(rows+cols) threads cover the whole map. A thread walks along its line, compares the viewing angle of each cell with the largest angle seen so far, and marks the cell visible if its angle is higher. It also corrects for Earth curvature and can limit the azimuth and tilt for radio planning.

* *Type:* Single observer; approximate (grid walk along lines to the map edge); one thread per line.
* *Hardware:* NVIDIA GeForce GTX 560 Ti (336 CUDA cores, 1 GB) with an Intel Core 2 Duo E8200 CPU, using CUDA.
* *Input / output:* A DEM up to 28161x17921 cells at 12.5 m (all of Slovenia, about 505 million cells, derived) in, a map of the viewing angle at the visible cells out.
* *Runtime:* 18 s for the whole 28161x17921 map, and 1.2 s versus 516 s for r.los (about 430x, derived) at 12.5 m with a 10 km maximum distance.
* *Code:* none found (the kernel is listed in the paper).
* *Gap for us:* It computes only one observer and spends over 90% of its time on disk and PCIe transfers, so a total viewshed needs far more parallelism than one thread per line and a way to keep data on the GPU.

*2012 · Tabik, Zapata & Romero · Simultaneous computation of total viewshed on large high resolution grids · IJGIS 27(4), 2013*

The paper computes the visible area from every point of a terrain map in one combined pass, instead of repeating a single-viewpoint calculation for each point. For each of 360 one-degree sectors it sorts the points along the sector direction. It then sweeps through them while a hull tree (a tree of convex hulls) gives each point's horizon profile quickly. A cheaper second stage derives the other half of the data from the opposite sector, which halves the storage and work.

* *Type:* Total; approximate (1-degree sectors, precision loss with distance); horizon / hull-tree sector sweep (Stewart-style).
* *Hardware:* One PC with an Intel Core 2 Duo 3.16 GHz and 3.25 GB RAM, C++ -O3, no GPU or threads.
* *Input / output:* A 2000x2000, 10 m Málaga DEM goes in, and a total-viewshed map comes out, plus about 42 GB of intermediate per-sector files.
* *Runtime:* Wall-clock about 6.5 h for the main stage, an amortised 0.0063 s per viewpoint versus about 10 s for ArcGIS (about 1,600x, derived) and about 155 s for GRASS r.los (about 25,000x, derived), though both baselines are single-viewpoint tools.
* *Code:* none found.
* *Gap for us:* It is single-threaded and disk-heavy (42 GB for 4 M cells), so a GPU version must replace the hull tree and file storage and be validated against exact viewsheds.

*2012 · Zhao, Padmanabhan & Wang · A parallel computing approach to viewshed analysis of large terrain data using graphics processing units · IJGIS 27(2), 2013*

For one observer, the paper gives every terrain cell its own GPU thread, which checks whether that cell is visible using the exact R3 line-of-sight method. To handle maps larger than memory, it cuts the terrain into coarse tiles that are moved from disk to GPU memory. Smaller sub-tiles are then staged in fast on-chip shared memory so that accesses are regular. A tile-reuse scheme avoids reloading data.

* *Type:* Single observer; exact (R3, differences under 0.00002% from float rounding); R3 line of sight with GPU tiling.
* *Hardware:* One GPU of an NVIDIA Tesla S1070 (GT200, compute capability 1.3) with one 2.4 GHz AMD Opteron core, CUDA SDK 4.0.
* *Input / output:* Seven Utah DEMs from 20,000x20,000 (1.49 GB) to 74,256x74,256 (20.54 GB, about 5.5 billion cells, resampled to 1 m) go in, and a boolean visibility map for one viewpoint comes out.
* *Runtime:* Wall-clock, the best case is 179 s versus 16,917 s for sequential R3 at 1.49 GB (about 94x, derived), with the paper headlining more than 50x for datasets under 4 GB.
* *Code:* none found.
* *Gap for us:* It is a single-viewpoint O(n^3) method taking hours to days per observer, so it does not address producing a viewshed size for every cell.

*2004 · Salomon et al. · Accelerating Line of Sight Computation Using Graphics Processing Units · venue not stated (OneSAF/RDECOM-funded conference paper)*

The paper answers many point-to-point line-of-sight questions between simulated entities on a terrain. The GPU first draws the terrain from above, then draws each query as a line with the depth test reversed, and an occlusion query reports whether any part of the line goes below the terrain. If none does, the two points can definitely see each other, and these queries are discarded. Only the remaining queries go to an exact ray-casting test on the CPU, and the GPU and CPU work on alternating batches at the same time.

* *Type:* Point-to-point line of sight (not a viewshed); exact result with conservative GPU culling; rasterization plus CPU ray casting.
* *Hardware:* NVIDIA GeForce 6800 Ultra with a Pentium 4 3.4 GHz, using OpenGL occlusion queries (graphics API, not CUDA).
* *Input / output:* A terrain of about 1 million triangles covering about 400 km per side with query pairs up to 4 km apart in, a yes/no line-of-sight answer per pair out.
* *Runtime:* About 4 microseconds per query on average (speedup versus a CPU-only baseline not stated).
* *Code:* none found.
* *Gap for us:* It answers pairwise queries through the graphics pipeline and never produces a viewshed, so it gives only the idea of GPU culling and not a way to compute all observers at once.

*2004 · Llobera et al. · Calculating the Inherent Visual Structure of a Landscape ('Total Viewshed') Using High-Throughput Computing · CAA 2004 (proceedings 2010)*

The authors want to know how much of a landscape can be seen from every point on a map. They use a simple brute-force line-of-sight routine that counts the cells visible from each point. They split the points into 160 independent jobs of about 1000 viewpoints each. The jobs run on idle lab PCs under the Condor system, so the speedup comes only from distribution and not from a better algorithm.

* *Type:* Total; approximate (Bresenham-style line of sight, no exactness claim); brute-force R3-like line of sight farmed out as independent jobs.
* *Hardware:* A Condor pool of 43 PCs (13 Pentium 4 1.5 GHz and 30 Pentium 4 2 GHz, Windows NT), with no GPU.
* *Input / output:* A 400x400-cell, 50 m DEM (extended to 1200x1200 to avoid edge effects, 20 km radius) goes in, and a 400x400 raster of visible-cell counts comes out.
* *Runtime:* Wall-clock 25 h 13 min on the pool versus 34 days 1 h 24 min estimated on one similar PC, about 32x (derived).
* *Code:* none found.
* *Gap for us:* It only farms out an unoptimised per-cell viewshed on CPUs, so GPU mapping, memory-aware algorithms and comparison with modern methods are all open.

*1998 · Ware, Kidner and Rallings · Parallel Distributed Viewshed Analysis · ACM GIS '98 (6th ACM International Symposium on Advances in GIS)*

The paper speeds up the "reverse viewshed" (a visibility index for each cell in a region, counting how many line-of-sight profiles succeed) by splitting the work over a cluster of ordinary networked PCs. It divides the DEM and the set of observer cells among the machines, either fixed in advance (static) or handed out in chunks by a master node as workers finish (dynamic "farming"). It compares 6 static and 5 dynamic schemes on 1 to 24 processors. Dynamic farming balances the load best, and the best chunk size is about 0.1 to 0.3% of the domain.

* *Type:* Multi-observer (visibility index for a block of observers); exact line-of-sight profiles; domain decomposition, master/worker.
* *Hardware:* 24 Pentium 100 MHz PCs on 10 Mbit/s Ethernet running Windows NT 4.0 with PVM, no GPU.
* *Input / output:* A 441x441 DEM (22 km x 22 km at 50 m) with 41x41 = 1,681 observer cells in, a visibility index per observer out (about 327 million profiles).
* *Runtime:* Speedup of about 22x on 24 processors versus one processor (absolute times not stated).
* *Code:* none found.
* *Gap for us:* It scales across machines but covers only 1,681 observers and shows that load balancing and chunk size matter, whereas a CUDA total viewshed needs the same balancing across thousands of GPU threads.
