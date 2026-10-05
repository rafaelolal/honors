*2026 · Aleksiejūnas & Stankevičius · Application of XDraw Algorithm to Diffraction Modeling Using 3D Vector Building Data · Transactions in GIS 30(2): e70253*

Radio signals bend around building edges, and this paper estimates that loss around 3D building models. It adapts the XDraw viewshed method, which grows outward from the antenna in rings and reuses the strongest blocking edges found by neighbouring points on the inner ring, to building roofs and walls instead of raster heights. The area is split into pie-slice sectors and run on 8 CPU processes.

* *Type:* Single (radio diffraction loss per antenna); approximate; XDraw ring-based with vector variants (Max-Obst, Max-Roof, ITU-R P.526 diffraction).
* *Hardware:* CPU only (Intel Xeon E5-2670 2x8 cores, 128 GB, and Core i9-10900, 64 GB; Ubuntu 24.10, Python 3.12, Numba), with no GPU.
* *Input / output:* Input is 3D BAG LOD 2.2 Amsterdam buildings (up to 357,792 buildings, 595 km²) on a 1 m grid with analysis radii of 150 to 1500 m, and output is a diffraction-loss raster in dB per antenna (best RMSE 6.8 dB versus the exact vector method).
* *Runtime:* Wall-clock, with one tile at 1 km radius under 10 min on both CPUs and ten tiles taking slightly over 1 h at 1.5 km on the i9-10900, and no single speedup figure given.
* *Code:* None found (on reasonable request).
* *Gap for us:* It is a single-source, CPU-only vector XDraw whose authors name GPU acceleration as future work, leaving a CUDA ring/sector formulation and an extension from one antenna to many or all positions open.

*2026 · Jiao, Yang, Jia, Ma, Zhang, Wang & Li · VRPF: A Fine-Grained 3D Radar Power-Density Computation Framework Based on Photogrammetric City Models for Urban Observation · Remote Sensing 18(12), 1936*

This paper computes how strong a radar signal is at many points in a city, which first requires knowing whether a straight line from the radar to each point is blocked by buildings. It stores the city's triangle mesh in an R-tree (a box-based search index) so each line only has to be tested against nearby triangles, then does exact ray-triangle tests and applies a power formula. The loop over target points runs on many CPU threads with OpenMP.

* *Type:* Multi-observer (a few radars, many random targets; not total); exact (99.82% agreement with an Open3D BVH baseline); 3D mesh line-of-sight with R-tree pruning and Moller-Trumbore tests.
* *Hardware:* Multicore CPU only (AMD EPYC 9T95, 64 cores, 128 GB, OpenMP, C++; also an Intel Core i7-14650HX), with no GPU.
* *Input / output:* Input is Hong Kong photogrammetric meshes of 500 m to 2000 m squares (6.8 M to 102 M triangles) with up to 10^8 target points, and output is a per-target occlusion flag and power density (size not stated).
* *Runtime:* Wall-clock core time for 10^8 targets is 5.92 s on 32 threads (3.17 s on 64), about 2.66x faster than a 1 m DSM method and 1.31x faster than an Open3D BVH baseline on 8 threads.
* *Code:* https://github.com/Lin-Hui-Jiao/Radar-Project and https://github.com/Lin-Hui-Jiao/buildindex
* *Gap for us:* It is CPU-only line-of-sight for a few radars to random targets, so a GPU/CUDA method for visibility between all point pairs (total viewshed) remains open.

*2026 · Kamath, Sudharsan, Singh, Wallenberg, Lindberg & Niyogi · SOLWEIG-GPU: GPU-Accelerated Thermal Comfort Modeling Framework for Urban Digital Twins · Journal of Open Source Software 11(118), 9535*

SOLWEIG is a model that estimates how hot it feels outdoors by working out sun, shade and how much sky each map cell can see (the sky-view factor). This package re-implements it in PyTorch so it runs on a GPU, cutting big maps into tiles. The sky-view factor is a visibility measure computed for every cell, but it is a means to a comfort estimate rather than a viewshed.

* *Type:* Total-like (sky-view factor for every cell, not a binary viewshed); approximate; sky-view-factor / shadow-casting radiation model (SOLWEIG 2022a).
* *Hardware:* NVIDIA A6000 (48 GB) via PyTorch (no hand-written CUDA kernels stated), against a CPU baseline of an Intel Core i7-10700 with 16 GB.
* *Input / output:* Input is building DSM, DEM, vegetation and hourly weather rasters in 1000 to 2000 cell-square tiles (resolution not stated), and output is SVF, mean radiant temperature, UTCI and shading rasters per tile and hour.
* *Runtime:* Wall-clock for SVF plus UTCI is 1187 s CPU versus 47 s GPU (about 25x) on 1000 x 1000 and 6487 s versus 158 s (about 41x) on 2000 x 2000, against the original SOLWEIG CPU code.
* *Code:* Open source (JOSS; docs at https://solweig-gpu.readthedocs.io; sample data on Zenodo), with the exact GitHub URL not in the PDF text.
* *Gap for us:* Its all-cells sky visibility is only at 1-2k tiles with a PyTorch speedup against an unstated CPU baseline, leaving true point-to-point total viewshed with custom CUDA kernels and large-DEM scaling open.

*2026 · Lukač & Žalik · GPU-Based Solar Irradiance Estimation over Digital Surface Models Using Structurally Lossless Viewshed Compression · Remote Sensing 18(17):3044*

To estimate yearly sunlight including reflections, every voxel (3D cell) of a city model needs a list of all other voxels it can see, which gets huge. The authors compress each list (sorted, differences stored, then run-length encoded) and decode it on the fly inside the CUDA kernel, so the full list never sits in GPU memory. This makes the largest tests run about 14 times faster than the uncompressed version.

* *Type:* Total (every voxel's hemispherical viewshed); approximate (4096 sampled directions, compression lossless); per-voxel ray casting with bit-aware run-length encoding.
* *Hardware:* One NVIDIA RTX 4090 (24 GB), CUDA 12.6, AMD Ryzen 9 7950X host, C++/CUDA.
* *Input / output:* Input is synthetic and real 1 m DSM voxel scenes of up to 11.9 M voxels (uncompressed viewshed up to 29.6 GB), and output is annual solar potential per voxel plus compressed viewshed streams (compression ratio about 2.5 to 3.4).
* *Runtime:* Wall-clock on the largest scene is 24,079 s uncompressed versus 1743 s compressed (about 13.8x), but compression is slower on small scenes and there is no CPU baseline.
* *Code:* https://github.com/SAMPA-Project/SolarGPU (data at https://doi.org/10.5281/zenodo.22145532).
* *Gap for us:* It uses a 4096-ray 3D voxel formulation with host-side encoding and no terrain-DEM baseline, so GPU-side compression of an exact DEM total viewshed and its throughput versus sweep methods is open.

*2026 · Meneses, Navarro, Ferrada, Verichev & Salazar-Concha · Ray Tracing Cores for General-Purpose Computing: A Literature Review · arXiv:2603.28771 (preprint)*

Modern NVIDIA GPUs have special hardware (RT cores) built to find where rays hit triangles, and researchers have started using it for non-graphics problems. This survey screens 59 papers and compares 35 of them to find which problems get big speedups. Problems that are about finding nearby things and can skip work gain most, while problems that depend on a few long rays or waiting on memory do poorly.

* *Type:* Other (survey; no viewshed problem among the 32 reviewed); n/a; RT-core / BVH ray-triangle query reformulation (OptiX-style).
* *Hardware:* No experiments of its own, with surveyed works using NVIDIA RT cores via OptiX/CUDA only (RTX 5090 with 170 RT cores cited as an example).
* *Input / output:* Input is 590 Scopus hits filtered to 59 papers and then 35 papers on 32 problems, and output is speedup and problem-taxonomy tables.
* *Runtime:* Metric is RT speedup versus the fastest non-RT GPU method, with a best of about 200x (nearest-neighbour search), radio wave propagation 5.25x best, and some problems slower than CUDA (BFS 0.4x).
* *Code:* None (review).
* *Gap for us:* It finds no terrain or viewshed use of RT cores, so whether total-viewshed horizon tests map onto RT cores and beat a CUDA sweep is an open question.

*2026 · Vujačić, Vujović & Šiljeg · Multi-resolution Copernicus DSM-based cumulative viewshed assessment of antenna towers for wildfire monitoring in Montenegro · Open Geosciences 18(1): 20251019*

This application study checks how much of Montenegro 99 existing antenna towers could watch for wildfires. It runs the standard GDAL viewshed tool once for each tower and each of 108 setting combinations (different map resolutions, heights and ranges), then merges the results into one coverage map. Depending on settings, 8.55% to 58.49% of the country is visible from at least one tower.

* *Type:* Multi-observer (cumulative union of 99 viewsheds); exact; sightline-free viewshed (Wang, Robinson & White 2000) via gdal_viewshed.
* *Hardware:* CPU, with no CPU/GPU model, core count or RAM stated (Python 3.11 with GDAL).
* *Input / output:* Input is Copernicus DSM at 10, 30 and 90 m over Montenegro (13,883 km²) with 99 towers and 108 scenarios, and output is a binary viewshed per tower and scenario merged into a binary cumulative layer.
* *Runtime:* Not stated, with the paper only noting that high-resolution viewshed over large extents "remains computationally demanding".
* *Code:* None found (data on request).
* *Gap for us:* It treats the viewshed as a black-box CPU call run 10,692 times with no performance data, leaving a fast GPU engine for many-observer or total viewsheds and a runtime-versus-resolution benchmark open.

*2026 · Wang, Zhu & Dou · Structured representation and compression method of viewshed: An example analysis of communication viewshed · Journal of Geographical Sciences 36(5):1299-1320*

A communication viewshed rates signal quality from 0 to 1 at every cell around a base station, and storing one for every possible station takes enormous space. This paper computes it for every cell as a station using CUDA, then keeps only the good-signal regions, fits each with a simple shape and stores seven numbers per region. This shrinks the data by hundreds of times (lossy) and speeds up choosing base-station sites.

* *Type:* Total (every cell as a base station); approximate (lossy shape-fit compression); CVCWS communication viewshed with threshold, morphology, contour extraction and shape fitting.
* *Hardware:* CUDA for the all-cells computation plus CPU post-processing, with GPU and CPU models not stated.
* *Input / output:* Input is a 500 x 500 1 m terrain (250,000 stations, up to 800 x 800 in other tests) and output is a structured region description per station (250,000 stations in 287.67 MB versus 107.29 GB as TIF).
* *Runtime:* Compression ratio is 407 at 500 x 500 and siting takes 12 s versus 2430 s for a genetic algorithm (about 200x), with no CUDA kernel time or all-cells total time reported.
* *Code:* Zenodo, doi 10.5281/zenodo.15589131 and doi 10.5281/zenodo.15589412.
* *Gap for us:* It lossily compresses the output of a slow model with undocumented CUDA and timings, so lossless or controllable-error on-GPU compression of exact total-viewshed output with reported throughput is open.

*2026 · Hodge & Barrientos Triñanes · City-Scale Visibility Graph Analysis via GPU-Accelerated HyperBall · arXiv preprint (cs.DC, 2604.08374)*

This paper scales Visibility Graph Analysis, a method from urban design that links every pair of grid points that can see each other on a city map with buildings, to city-sized areas. Mutual visibility is computed on the CPU with the sparkSieve2 angular sweep (ported to Rust), and the graph is stored compressed (delta-encoded varints, about 4x smaller). Metrics such as mean depth then use HyperBall, which estimates how many points are reachable in each number of steps using small probabilistic counters, and a fused CUDA kernel decodes the compressed graph and merges those counters on the GPU.

* *Type:* Total (all-pairs mutual visibility on a 2D building-obstacle map, not terrain); exact visibility edges via sparkSieve2 angular sweep, then approximate graph metrics via HyperBall (HyperLogLog counters).
* *Hardware:* Intel i7-12700H (20 logical cores, 32 GB RAM) and NVIDIA RTX 3080 Ti Laptop GPU (16 GB), CUDA 12.0 (sm_86), with the visibility step in Rust and Rayon on the CPU.
* *Input / output:* OpenStreetMap building footprints and a study-area boundary (Valdivia, Chile) on a regular grid in, a compressed visibility graph and per-cell metrics (for example Visual Mean Depth) out.
* *Runtime:* End-to-end time; 7.25 s versus 1732 s for depthmapX (239x) at 42705 cells and 161 million edges, and 137 s for 236000 cells (4.8 billion edges) at p = 10.
* *Code:* none found (the paper links only to depthmapX and Rayon, with a QGIS plugin listed as future work).
* *Gap for us:* The GPU accelerates only the graph metrics, not the visibility computation itself (done on CPU), and it targets 2D building maps, so a CUDA total viewshed on terrain DEMs is still open.

*2025 · Argudo et al. · Terrain descriptors for landscape synthesis, analysis and simulation · Computer Graphics Forum 44(2), e70080 (Eurographics 2025 STAR)*

This survey compares terrain measures from geomorphology with what Computer Graphics uses, including slope, drainage, landforms and visibility. The visibility section covers the viewshed, the cumulative viewshed and the visibility index (how much area is seen from each cell, which is effectively a total viewshed). The authors time naive single-thread C++ versions of every measure on one Alpine heightfield. The visibility index is by far the most expensive, and they propose cheap statistical proxies rather than faster algorithms.

* *Type:* Single + total (viewshed, cumulative viewshed, visibility index; one section of a broad survey); exact reference definitions; brute-force line-of-sight ray marching.
* *Hardware:* Unspecified CPU ("our machine"), single thread, no GPU.
* *Input / output:* Input is a 1024x1024 heightfield covering 952 km2 of the Alps (about 30 m spacing, derived), and output is per-metric rasters of the same size.
* *Runtime:* Cost relative to slope (18 ms); one viewshed is about 3,600 s (derived) and the visibility index over all cells is about 10^11 times slope, roughly 57 years single-thread (derived).
* *Code:* https://github.com/oargudo/terrain-descriptors
* *Gap for us:* It shows the visibility index is the extreme cost outlier and offers only proxies, so exact GPU total-viewshed acceleration on large grids is open.

*2025 · Chen, Li, Deng & Sun · Framework for Organizing Viewsheds From Multiple Observation Points: Enabling Bidirectional Landscape Visibility Information Queries · IEEE JSTARS 18:22392-22401*

This paper is about storing many viewsheds compactly and querying them, not about computing them. It stacks the binary viewsheds into a 3D array and packs the observer axis into 32-bit integers. Each cell's packed code is replaced by an index into a list of distinct visible-observer sets, and the index grid is run-length encoded. The result supports both "which observers see this location" and "which locations see these observers", and it is about 98% smaller than the raw array.

* *Type:* Multi-observer (organizes existing viewsheds, computes none); exact (lossless); 3D tensor with bit-packing, set deduplication and run-length encoding.
* *Hardware:* Intel Core i7-7700HQ (4 cores) with 16 GB RAM, C/C++ and Python, no GPU.
* *Input / output:* Input is 100 binary viewsheds on a 1775x2059 ALOS 12.5 m DEM of the Jinshanling Great Wall, and output is a compact store (for example 197.2 MB down to 3.0 MB for 100 observers at 1 M cells) plus query results.
* *Runtime:* Not a speed paper; storage and build time grow linearly, the deduplication step takes minutes for 1 M cells x 100 observers, and query times appear only in plots.
* *Code:* none found.
* *Gap for us:* It organizes at most 100 precomputed viewsheds on a laptop CPU, so storing visibility for every cell as observer, ideally compressed on the GPU, is left open.

*2025 · Laboy et al. · Advancing the GEOtiled Framework Through Scalable Terrain Parameter Computation · HPDC '25 (2-page extended abstract)*

GEOtiled 2.0 upgrades the earlier tile-based terrain workflow so it can use SAGA terrain functions, about five times more parameters than GDAL alone. It also makes three optimizations: cropping tiles concurrently, stitching tiles by cropping the borders and concatenating rather than averaging, and using one shared multiprocessing pool for all steps. These cut overhead so runtime keeps falling as tiles get smaller. It is CPU-only and about local terrain measures, not visibility.

* *Type:* Other (elevation, slope, aspect and other SAGA parameters; not visibility); exact (accuracy claimed maintained, no numbers); buffered tile decomposition with a multiprocessing pool.
* *Hardware:* Not stated (Python multiprocessing, no GPU).
* *Input / output:* Input is a 30 m DEM of Tennessee, and output is one mosaicked raster per terrain parameter (sizes not stated).
* *Runtime:* Wall-clock; up to about 7x faster than GEOtiled 1.0 at the smallest tile size (259.7 s vs 37 s).
* *Code:* Stated as openly available on GitHub but no URL is given (GEOtiled 1.0 is in TauferLab/SOMOSPIE).
* *Gap for us:* It only optimizes workflow overheads for local CPU parameters, so GPU kernels, long-range line-of-sight parameters and large-grid evaluation are open.

*2025 · Leenders et al. · Uncertainty in visibility: a scoping review of the probable and fuzzy viewshed for observer location optimization · International Journal of Geographical Information Science 40(7):1993-2022*

This is a literature review of viewsheds that are not simply "visible or not". It covers probable viewsheds (visibility as a probability, for example from terrain-data error) and fuzzy viewsheds (visibility as a degree, for example from distance or vegetation). It examines how these are used when choosing where to place observers. Of 617 records found, 48 papers were included. It proposes no algorithm, and the authors find that observer-placement work almost always uses plain yes/no visibility.

* *Type:* Multi-observer (siting / maximum coverage), literature review; not applicable (survey of non-boolean visibility); probable and fuzzy viewshed models.
* *Hardware:* Not stated (no experiments, no GPU content).
* *Input / output:* Input is a literature search (617 records, 48 papers included), and output is a taxonomy table and list of research gaps.
* *Runtime:* Not stated (it only notes a Choquet-integral fuzzy combination can cost O(n^2) instead of O(2^n) with a 2-additive measure).
* *Code:* none found.
* *Gap for us:* Computing probable or fuzzy viewsheds needs many viewshed runs per observer, so a CUDA implementation that makes Monte Carlo or fuzzy total viewsheds tractable is open.

*2025 · Moser & Grêt-Regamey · Visual exposure to buildings in Switzerland: spatial patterns and changes over six decades · Scientific Reports 15:34905*

This is an application study that asks how many buildings can see each spot in Switzerland, and how that changed from 1960 to 2024. For seven years, it treats every building as a viewpoint (about 1 to 2 million per year) and adds up how many buildings are visible from each map cell. The heavy viewshed work was run with a GPU-enabled Python library (xarray-spatial), but the paper gives no details of how it was done.

* *Type:* Multi-observer (binary and cumulative viewshed from about 1-2 M buildings); algorithm not stated (library viewshed, assumed exact-style but unverified); xarray-spatial viewshed.
* *Hardware:* GPU via the xarray-spatial Python package, with GPU model, CPU, RAM and CUDA use not stated.
* *Input / output:* Input is the 25 m DHM25 elevation raster of Switzerland (2 km visibility limit) plus 971,869 (1960) to 2,027,426 (2024) building observers, and output is a binary and a cumulative visible-building-count raster per year.
* *Runtime:* Not stated; no timings or speedups are reported.
* *Code:* None found (data on request; xarray-spatial itself is open source).
* *Gap for us:* A national-scale, many-observer count workload that needed a GPU but reports no hardware or timings, leaving room for a documented, benchmarked CUDA implementation.

*2025 · Schwencke, Stütz & Bulatov · AI-Accelerated Viewshed Computation for High-Resolution Elevation Models · VISIGRAPP 2025 (GRAPP/HUCAPP/IVAPP), pp. 251-258*

When checking whether a point is visible along a line of sight, most of the work is wasted on stretches that are already hidden. The authors train a neural network that reads the terrain and building heights along a line and predicts where the farthest visible point is, so the normal line-of-sight check can stop there. This avoids more than 90% of the point checks, at the cost of small errors (a few hundred metres on a 10 km line).

* *Type:* Single line-of-sight (motivated by total viewshed); approximate (learned stopping point); R3-style max-slope line-of-sight with neural early abort.
* *Hardware:* CPU only for the timed inference (Intel Xeon Gold 6154, about 0.03 ms per line of sight in batches of 10,000), with training hardware and any GPU use not stated.
* *Input / output:* Input is height profiles of 10,000 m lines from three DSM+DTM datasets (Latvia, Bavaria, Hesse; grid size not stated), and output is the predicted farthest-visible-point distance per line, not a full viewshed raster.
* *Runtime:* Metric is the share of per-point computations still needed versus no early abort, with the best figure 1.02% (Latvia) to 5.30% (Hesse) at over 99% accuracy, and no wall-clock speedup stated.
* *Code:* None found.
* *Gap for us:* It shows over 90% of each line never needs computing but gives no GPU kernel or end-to-end timing, so mapping early termination onto warp-coherent CUDA sweeps for a total viewshed is open.

*2025 · Wrozynski, Wrozynska & Pyszny · Application of 3D Ray Tracing for Water Surface Visibility Analysis · ISPRS International Journal of Geo-Information 14(9), 335*

The authors want to know from where on land a body of water can be seen. They build a 3D model of the area from LiDAR data and make the water glow in the free renderer Blender, then look straight down at the scene. Any pixel that is lit is a place from which water is visible, and eye height is handled with an offset copy of the terrain.

* *Type:* Single (one viewshed of a target surface, with observer-height variants); exact? (path tracing at 4096 samples per pixel, validated by photos at 2 points); render-based 3D ray tracing (Blender Cycles, top-down orthographic).
* *Hardware:* "GPU rendering" in Blender 4.3 Cycles, with GPU and CPU models not stated.
* *Input / output:* Input is a 44 km² LiDAR-derived 3D scene of Swinoujscie, Poland (1 m terrain TIN, 4,163 buildings, vegetation meshes), and output is a 1 m binary visible-water raster totalling 4.13 km².
* *Runtime:* Not stated; only qualitative comparison with a 2.5D GIS viewshed and photo checks.
* *Code:* None found (only the BlenderGIS add-on is linked).
* *Gap for us:* It uses an off-the-shelf GPU renderer with no timings, hardware or scaling data and one viewshed per run, so it does not address a CUDA total viewshed over a large DEM.

*2025 · Widlak & Orlof · Ray Tracing and Bound Block Elimination Methods for Viewshed Analysis with Efficiency Enhancements through Radial Segmentation and Parallel Processing · ECMS 2025 (Communications of the ECMS 39(1))*

The paper compares two ways of finding what one observer can see in a LiDAR point cloud: ordinary ray tracing on a triangulated surface, and Bound Block Elimination (BBE), which labels each 3D point as visible or hidden. BBE is accurate but very slow, so the authors cut the cloud into pie-slice sectors around the observer and process the slices in parallel on CPU cores. Using more sectors made it faster, and 300 sectors took 74 minutes instead of nearly 6 hours for 100.

* *Type:* Single observer; exact point-cloud visibility (BBE, compared with TIN ray tracing, which gave 11.69% visible versus 4.25% for BBE because they measure different things); radial sector decomposition of a point-cloud visibility test.
* *Hardware:* CPU only, not GPU (Python multiprocessing process pool on an Intel Core i9-14900K at 3.20 GHz with 64 GB RAM), limited mainly by RAM since a sector must fit in memory.
* *Input / output:* Input is a 2023 Krakow LiDAR point cloud near Wawel Castle (6,351,545 points, 566 m x 586 m, 12 points/m²), and output is a visible/hidden point cloud (orthographic, perspective or panoramic views) for one observer.
* *Runtime:* Total wall-clock of 352.76 min (100 sectors), 126.12 min (200) and 74.33 min (300), which the paper calls "179%" and "405%" speedups (derived: ratios of the slowest-sector times, 2.79x and 5.05x), with ray tracing taking 256 s and the sector sizes uneven (slowest sector 17,796 s at 100 sectors).
* *Code:* None found.
* *Gap for us:* It is a single-observer, CPU-only, hours-long method on a small 6 M point cloud with no GPU, so it leaves open a CUDA version and any total viewshed (all observers) of a comparable cloud.

*2025 · Zhang et al. · Efficient and fine-grained viewshed analysis in a three-dimensional urban complex environment (Q-View) · Environmental Modelling & Software 188:106359*

This paper stores a huge 3D city model in a spatial index (a tree of boxes) and then simply asks, for every target point, whether the straight line from the viewer hits anything. Each question is independent, so many CPU threads answer them in parallel, and the method also handles curved sight paths. It is compared with the raster methods R2, R3 and view-tree and with a GPU shadow-map tool.

* *Type:* Single (3D triangle mesh, not a DEM); exact against the mesh; 3D R-tree with one line-of-sight query per target.
* *Hardware:* 32-core Intel Xeon E5-2620 at 2.10 GHz with 512 GB RAM, C++ with OpenMP, no GPU.
* *Input / output:* Input is Hong Kong photogrammetry meshes from 0.87 M to 1.6 billion triangles, and output is a boolean visibility per target for one observer rendered as a 2D raster overlay.
* *Runtime:* Wall-clock; 0.19 s to 4.14 s for radius 200 to 1000 m on a 54.5 M-triangle mesh, about 2,000x to 12,200x faster than R3 (derived) and about 16x parallel speedup on 32 threads.
* *Code:* yes, https://github.com/Zoey-Fan/Q-view.
* *Gap for us:* It is single-observer, CPU-only and mesh-based, so it computes no viewshed size per cell and has no CUDA, though its independent parallel queries and queries-per-second metric could be reused.

*2024 · Guo, Wenz & Auffhammer · The visual effect of wind turbines on property values is small and diminishing in space and time · PNAS 121(13): e2309372121*

This is an economics study of whether seeing a wind turbine lowers home prices. The authors compute a viewshed from each of 68,649 US turbines rather than from each house, because visibility works both ways and turbines are far fewer than houses. They then check which of 180.7 million home sales fall inside a turbine's viewshed and compare prices. No new viewshed algorithm is developed, only an off-the-shelf GIS tool is used.

* *Type:* Multi-observer (one viewshed per turbine, aggregated); exact; GRASS GIS viewshed module (algorithm not named).
* *Hardware:* Not stated (GRASS GIS and ArcGIS geocoding on CPU).
* *Input / output:* Input is the 90 m SRTM DEM of the continental US, the USGS wind turbine database and ZTRAX home sales, and output is a binary viewshed per turbine (10 km range, 1.75 m receiver height) combined into a national visibility database.
* *Runtime:* Not stated.
* *Code:* Replication kit at https://github.com/weiguochina/Windmill_Visibility_House_Price_US_2024 (SRTM DEMs not included, ZTRAX needs a licence).
* *Gap for us:* It shows real demand for a continent-scale visibility layer built with a plain CPU viewshed and no timings, so a fast CUDA multi-observer viewshed at finer than 90 m is open.

*2024 · Romero et al. · SkewEngine: enhancing performance of intensive calculations on regular meshes · The Journal of Supercomputing 80:12910-12928*

SkewEngine turns the sDEM skew trick into a reusable C++ template library. For each of 180 directions it resamples the grid so that points along that direction share a memory row, runs any user-supplied kernel on every row independently, and un-skews and combines the results. Four pre-built input variants (normal or transposed, normal or mirrored) remove sDEM's per-angle branching, which is slow on GPUs, and allow non-square grids. Total viewshed is the main example, alongside an identity test, a cepstrum motion-blur filter and the Radon transform.

* *Type:* Total (also general mesh kernels); approximate (180 sectors, bilinear skew interpolation); rotational sweep on a skewed mesh (generalized sDEM).
* *Hardware:* Four machines, from an i7-13620H laptop with RTX 4050 up to a Xeon E5-2698 v4 node with 8x V100, using OpenMP, CUDA and OpenCL.
* *Input / output:* Input is a 25 M-point DEM of Sierra de las Nieves for viewshed (plus images for the other cases), and output is a per-cell visibility map.
* *Runtime:* Wall-clock; total viewshed on the 25 M-point DEM takes 3.5 s with a GPU vs 10.2 s for sDEM (about 2.9x, GPU model not stated), and 59 s on CPU only.
* *Code:* https://github.com/luisfromero/skewEngine
* *Gap for us:* It reports no accuracy analysis, no multi-GPU viewshed scaling and only a 25 M-point grid, so grid size, exactness against the 180-sector approximation and memory limits are still open.

*2024 · Willis et al. · GPU-Accelerated 3D Polygon Visibility Volumes for Synergistic Perception and Navigation · IEEE SoutheastCon 2024 (arXiv:2402.03135)*

This paper finds the 3D region of space from which an entire flat polygon (for example a building face) can be seen. For each polygon corner it renders six depth images (a cube map) with OpenGL to get a "visibility sphere" around that corner. The spheres are turned into meshes and intersected with exact boolean operations in Blender, with edges split further where the views of two neighbouring points differ. Only the depth rendering uses the GPU, and the paper gives a qualitative demo without timings.

* *Type:* Single (per convex polygon target, not a raster viewshed); exact mesh booleans on angularly discretized depth maps; cube-map depth rendering plus CSG intersection.
* *Hardware:* Not stated (OpenGL 4.6 rendering, Blender 3.2 or later booleans on CPU).
* *Input / output:* Input is an OpenStreetMap triangle mesh of Destin, FL plus a convex target polygon, and output is a watertight 3D visibility-volume mesh.
* *Runtime:* Not stated (qualitative demonstration only).
* *Code:* none found.
* *Gap for us:* With no hardware, timings or scale it gives nothing to benchmark against beyond the cube-map depth idea.

*2024 · Xiao et al. · Parallel algorithm for multi-viewpoint viewshed analysis on the GPU grounded in target cluster segmentation · International Journal of Digital Earth 17(1):2308707*

This paper speeds up shadow-map viewsheds, which render depth images from each viewpoint and compare them with the scene, for many viewpoints over terrain and 3D buildings. The CPU splits the area into a grid and groups the viewpoints into clusters with K-means. A WebGL fragment shader then tests each location only against the viewpoints in its own cluster and neighbouring cells. An adaptive depth bias fitted by regression removes most shadow-map artifacts.

* *Type:* Multi-observer (15-600 viewpoints, not total); approximate (shadow-map depth comparison, not checked against exact visibility); shadow map with grid and K-means clustering.
* *Hardware:* AMD 5700X with GeForce GTX 1080 Ti (and AMD 6850HS with integrated Radeon 680M), using WebGL fragment shaders, not CUDA.
* *Input / output:* Input is a 12.5 m ALOS DEM and about 17,000 buildings over 43.6 km2 of Changsha with 15-600 viewpoints, and output is a per-fragment visible/invisible colouring in Cesium with no stored raster.
* *Runtime:* Wall-clock after loading; about 8-10% faster than the plain shadow map on the same GPU at 150-600 viewpoints (for example 150.3 vs 162.2 ms at 600 viewpoints on the 1080 Ti), and slightly slower below 75.
* *Code:* none found (data on request).
* *Gap for us:* It handles at most 600 viewpoints with a roughly 10% gain and no exactness check, leaving millions of observers with exact per-observer output untouched.

*2023 · Meneses et al. · Accelerating Range Minimum Queries with Ray Tracing Cores · arXiv:2306.03282 (preprint, Future Generation Computer Systems)*

This paper answers "what is the smallest value between positions l and r of an array?" using the ray-tracing (RT) cores in recent NVIDIA GPUs. Each array element becomes a triangle whose position encodes its index and whose depth encodes its value. A ray fired for a query then hits the triangle holding the minimum first, so the hardware that finds the closest hit finds the answer. A block-matrix layout works around floating-point precision limits so arrays can exceed 2^24 elements. It is not a viewshed paper, but it shows how to recast a query as ray tracing.

* *Type:* Other (range-minimum query on a 1D array, not a viewshed); exact; BVH ray-triangle closest-hit (OptiX).
* *Hardware:* NVIDIA RTX 6000 Ada (48 GB, 142 RT cores) with 2x AMD EPYC 9654, OptiX and CUDA 12.1, plus scaling tests on TITAN RTX, RTX 3090 Ti, 4070 Ti, 4080 and 4090.
* *Input / output:* Input is a random float array of up to 2^26 elements with batches of 2^26 queries, and output is one argmin index per query.
* *Runtime:* ns per query; about 5 ns/query for large ranges and 2.5x to 5x faster than the 192-core CPU method HRMQ, but slower than the GPU LCA method on medium and large ranges.
* *Code:* https://github.com/temporal-hpc/rtxrmq
* *Gap for us:* Its 9x memory overhead and FP32 limits leave open whether a horizon-angle maximum along a sight line, shared by many observers, can use RT cores or whether plain CUDA sweeps win.

*2023 · Roa et al. · GEOtiled: A Scalable Workflow for Generating Large Datasets of High-Resolution Terrain Parameters · HPDC '23 (2-page extended abstract)*

GEOtiled computes slope, aspect and hillshade for very large elevation maps by cutting the map into tiles. Each tile carries a border of neighbouring cells, so edge cells can still be computed correctly. The tiles are processed independently with GDAL on separate cloud machines and then stitched back together, averaging the overlapping borders. It uses no GPU and does not compute visibility.

* *Type:* Other (slope, aspect, hillshade; not visibility); approximate (R2 = 0.794 vs SAGA at 1 km); buffered-tile data partitioning with GDAL.
* *Hardware:* 10 Jetstream 2 VMs with 8 CPU cores and 30 GB RAM each (CPU model not stated), GDAL, no GPU.
* *Input / output:* Input is the USGS 3DEP DEM of the continental US (341.1 GB at 10 m), and output is three terrain rasters (911.1 GB at 10 m).
* *Runtime:* Wall-clock; nearly 6x faster than GDAL at 10 m with 400 tiles (about 23,000 s vs about 3,800 s, read off a bar chart).
* *Code:* GEOtiled.ipynb in https://github.com/TauferLab/SOMOSPIE
* *Gap for us:* Buffered tiles suit small neighbourhoods, whereas a viewshed needs the full sight radius as buffer, which this paper does not address.

*2023 · Kimm, White & Burry · Extending Visuospatial Analysis in Design Computing: An Exploration with a Novel GPU-Based Algorithm and Form-Based Codes · CAADRIA 2023 (28th Int. Conf. of the Association for Computer-Aided Architectural Design Research in Asia), Vol. 1, pp. 655-664*

Instead of testing visibility ray by ray, this paper lets the GPU draw it. In the Unity game engine, one camera renders the building of interest and another renders everything that blocks it, then a small GPU program counts how much of the building is still visible in the picture. The authors pair this with a genetic algorithm (a trial-and-error search that evolves candidate solutions) that shapes building heights to hide volume from chosen viewpoints, as a tool for form-based planning codes.

* *Type:* Other (single feature seen from a few viewpoints, not a terrain viewshed); approximate (resolution set by a 512 x 256 render target); GPU rasterisation with a compute-shader histogram.
* *Hardware:* NVIDIA GeForce RTX 2070 laptop GPU (Intel i7-8750H, 32 GB, Windows 10) using Unity 3D shaders and compute shaders.
* *Input / output:* Input is a Unity 3D urban massing scene of 2.8k to 673k triangles with an 8 x 8 cell building envelope and two viewpoints, and output is visibility histogram statistics per building feeding the genetic algorithm.
* *Runtime:* Mean 901.83 ms per generation (150 individuals x 2 viewpoints; 1,130.4 ms in the tree-heavy scene) versus 58.54 s for the equivalent 39,321,600 hit tests, about 64x faster (the paper's figure) than a traditional hit-test method (hit-test hardware not stated).
* *Code:* None found.
* *Gap for us:* It shows rendering can replace ray tests for a few viewpoints at interactive speed but is not a DEM method and does not scale to all-observer visibility, so a CUDA total viewshed on terrain is untouched.

*2022 · Sánchez Fernández · Novel Parallel Approaches to Efficiently Solve Spatial Problems on Heterogeneous CPU-GPU Systems · PhD thesis, Universidad de Málaga*

This PhD thesis covers three spatial problems, and only the total-viewshed part (Chapter 4) and its use for UAV path planning (Chapter 5) matter here. Its method, called sDEM, rotates the terrain one sector at a time and rewrites it as a "skewed" grid so that every cell along a line of sight sits in the same memory row. One GPU thread per observer cell then sweeps along its row, and the results are un-skewed and added up. The sectors are split across several GPUs, which gives large speedups over the earlier single-threaded total-viewshed (TVS) code.

* *Type:* Total; approximate (discrete sectors, 1.18% absolute / 4.21% relative difference vs TVS); rotational sweep on a skewed DEM (sDEM).
* *Hardware:* Intel Xeon E5-2698 v3 (16 cores) with 4x GeForce GTX 980 (Maxwell, 4 GB each), CUDA 10.0, plus OpenMP on the CPU.
* *Input / output:* Input is a 10 m DEM of Montes de Málaga at 2000x2000, 4000x4000 or 8000x8000 cells, and output is one visibility ("heat") value per DEM cell.
* *Runtime:* Wall-clock; best case 827.3x with 4 GPUs on the 4000x4000 DEM (15.86 s vs 13121.14 s) against single-thread TVS, extrapolated from sectors 0-45 degrees.
* *Code:* sDEM on Figshare, https://doi.org/10.6084/m9.figshare.11370549 (single-core, multicore, single-GPU, multi-GPU).
* *Gap for us:* It was tested only up to 64 M cells on old Maxwell GPUs and is approximate, so exactness, bigger DEMs and modern GPUs remain open.

*2022 · Cheng & Dou · HiPDERL: An Improved Implementation of the PDERL Viewshed Algorithm and Accuracy Analysis · ISPRS International Journal of Geo-Information 11(10):506*

This paper fixes bugs in a recent fast viewshed algorithm (PDERL) so that it again matches the slow, trusted R3 method. The authors write a checking program that finds why PDERL goes wrong (skipped cells in the observer's column, a flawed floating-point step, and errors spreading through reused reference lines). They then measure how many cells are wrong before and after the fix. The algorithm is sequential.

* *Type:* Single; exact (matches R3, 0.001-0.009% cell errors); PDERL reference lines with data reuse.
* *Hardware:* Single-threaded C++ on a Windows 10 laptop with an Intel Core i7-9750H and 16 GB RAM, no GPU.
* *Input / output:* Input is three 3601 x 3601 ASTER GDEM tiles (30 m) with 1225 viewpoints each and a 100-cell radius, and output is a 201 x 201 binary viewshed per viewpoint.
* *Runtime:* Wall-clock for all 1225 viewsheds; HiPDERL took 45.7 / 45.2 / 44.7 s (mountains / hills / plains) vs R3 126.0 / 92.2 / 62.3 s, so about 2.8x / 2.0x / 1.4x faster than R3 (derived) and 13% slower than original PDERL (derived).
* *Code:* none found.
* *Gap for us:* It is CPU, single-viewpoint and limited to a 100-cell radius, so it offers a possible exact-accuracy reference and a warning about float errors in reuse-based methods but nothing on GPU or total-viewshed speed.

*2022 · Gu · Towards Optimal Line of Sight Coverage · Doctor of Engineering dissertation, Johns Hopkins University*

This dissertation finds good places for a few sensor or radio towers on real terrain. It computes all towers' line-of-sight coverage at once on a GPU, loading each large terrain tile into GPU memory at most twice by first finding the maximum slope in each tile and then combining slopes across tiles. A search method (simulated annealing or Bayesian optimization) then picks the tower layout that leaves the fewest gaps along an intruder's path.

* *Type:* Multi-observer (a few towers, one viewshed kept per tower); approximate (R2); tile-pipelined R2 line-segment sweep.
* *Hardware:* One desktop with a single CUDA GPU plus CPU, exact models not stated.
* *Input / output:* Input is a 5 m LiDAR surface model of 4096 x 4096 cells (and a small synthetic terrain) with 1 to 6 towers, and output is a per-tower 0/1 line-of-sight map turned into detection probabilities and tower coordinates.
* *Runtime:* Wall-clock; about 3 s for the GPU viewshed step per optimization iteration on 16.7 M cells, with no speedup against a baseline reported and the CPU path-cost step (about 1 minute) dominating.
* *Code:* none found (not public; contact the author).
* *Gap for us:* It batches R2 viewsheds for a handful of towers and never produces a viewshed size for every DEM cell, so scaling its tile-wise prefix-max idea to millions of observers is left open.

*2022 · Kuna et al. · Computing and testing extensive total viewsheds: a case of prehistoric burial mounds in Bohemia · Journal of Archaeological Science 142:105596*

The authors used a national supercomputer to compute how much land can be seen from points covering all of Bohemia, then tested whether prehistoric burial mounds sit at places with particular visibility. They sped up MATLAB's single-viewshed function and ran it from every 8th cell in parallel across eight radii (0.5 to 64 km). Most of the paper is archaeology, not computing.

* *Type:* Total (sampled, every 8th cell as observer); approximate (R2, 1/8 sampling); per-observer R2 run in parallel over observers.
* *Hardware:* IT4Innovations supercomputer (CPU, MATLAB parfor, 400,000 core-hours granted), with CPU model and node count not stated and no GPU.
* *Input / output:* Input is a 2 m LiDAR DTM of Bohemia (about 57,000 km2) resampled to 5-320 m cells (up to about 2.5 billion cells), and output is eight total-viewshed rasters (percent of cells visible) released on Zenodo.
* *Runtime:* CPU time, not wall-clock; about 256,000 CPU hours in total (derived sum), 194,137 h for the 0.5 km/5 m model alone, with the optimized MATLAB viewshed "almost fifty-fold" faster than stock MATLAB.
* *Code:* none found (modified MATLAB function not released; output datasets are public on Zenodo, doi 10.5281/zenodo.5764173).
* *Gap for us:* A cost of about 256,000 CPU hours for only every 8th observer shows the need for a faster all-cells method, and it gives no GPU implementation or like-for-like benchmark.

*2022 · Steger et al. · HORAYZON v1.2: an efficient and flexible ray-tracing algorithm to compute horizon and sky view factor · Geoscientific Model Development 15:6817-6840*

This paper computes, for every cell of a terrain model, how high the surrounding terrain rises above the horizon in every compass direction, and from that the fraction of sky visible. Instead of scanning cells, it shoots rays using a fast computer-graphics ray-tracing library (Intel Embree) over a triangle mesh of the terrain. Work is spread across cells on many CPU cores, and it reuses each direction's horizon as the starting point for the next.

* *Type:* Total-like (horizon and sky view factor for every cell, not a viewshed size); approximate (horizon within +/-0.25 degrees); BVH ray tracing over 360 azimuth sectors.
* *Hardware:* Multicore CPU (Embree + TBB) tested on an Intel Core i5 quad-core 3.4 GHz (exact model not stated) and an Intel Xeon Gold 3.4 GHz with 2 x 16 cores, no GPU.
* *Input / output:* Input is NASADEM (30 m), USGS (10 m) or swissALTI3D (2 m) terrain windows up to about 10^6 cells, and output is a horizon angle per cell per azimuth sector plus sky view factor.
* *Runtime:* Wall-clock; about 72x faster (30 m DEM) and 321x faster (10 m DEM) than the Buzzi 2008 OpenMP code at 10^6 cells and 50 km, with about 27x scaling on 32 cores.
* *Code:* yes, https://github.com/ChristianSteger/HORAYZON (MIT license).
* *Gap for us:* It gives per-cell horizons rather than visible-cell counts and runs only on CPUs, so turning its horizon idea into a CUDA viewshed-size kernel is left open.

*2021 · Chen & Chen · A parallel multipoint viewshed analysis method for urban 3D building scenes · Transactions in GIS 25(4)*

This paper uses a graphics card's 3D rendering pipeline, the same shadow-map trick video games use, to show which parts of city buildings can be seen from several viewpoints at once. Each viewpoint renders a depth map to decide which building surfaces are visible, and a cumulative index over all viewpoints colors each area as fully visible, partly visible or invisible. Visibility steps run on the GPU, while building selection runs on the CPU.

* *Type:* Multi-observer (1 to 5 viewpoints, 3D building scene, not a DEM); approximate (shadow-map rendering); shadow map.
* *Hardware:* Laptop with an Intel Core i7-9750H and an NVIDIA GeForce GTX 1650, using OpenGL shaders (GLSL) on osgEarth, not CUDA.
* *Input / output:* Input is 200 3D building models over 43.46 km2 with 1 to 5 viewpoints, and output is per-viewpoint visibility textures plus a color-graded cumulative visibility rendering.
* *Runtime:* Wall-clock; 0.2375 s for 1 viewpoint up to 1.4856 s for 5 viewpoints on 20 buildings, about 1.3x to 1.9x faster than its own CPU version for 2 to 3 viewpoints (derived) and 3.4x to 6.6x faster than a point-to-point line-of-sight method (derived).
* *Code:* none found (data on request from the author).
* *Gap for us:* It handles only a handful of viewpoints on 3D buildings with rendering-based visibility, so it says nothing about viewshed sizes for every cell of a large DEM in CUDA.

*2020 · Qarah · Efficient Viewshed Computation Algorithms On GPUs and CPUs · PhD dissertation, University of South Florida*

This thesis makes the exact "rotating line" viewshed method run on a GPU. Each cell creates events (insert, query, remove) that are sorted by angle and distance, the circle is cut into angular sectors, and each GPU thread sweeps its own sector with a fixed-size balanced tree. A second design processes the grid in sectors one at a time so larger maps fit in GPU memory.

* *Type:* Single; exact; radial (rotational) sweep with a status tree.
* *Hardware:* One NVIDIA Titan V (CUDA, 11 GB) with an AMD Ryzen Threadripper 1950X for the sequential CPU baselines.
* *Input / output:* Input is an SRTM DEM of Germany at 10,800 x 10,800 cells (also 5,000^2 to 30,000^2), and output is one visibility value per cell for a single observer.
* *Runtime:* Wall-clock; about 1.7 s for 10,800 x 10,800 (read from a chart), 3.5x faster than the GPU R3 of Zhao and about 2,000x faster than the sequential radial sweep.
* *Code:* none found.
* *Gap for us:* One viewshed costs about 1.7 s, so repeating it for every cell would take years (derived, about 6 years), and nothing here shares work between observers or uses multiple GPUs.

*2020 · Sánchez-Fernández et al. · A data relocation approach for terrain surface analysis on multi-GPU systems: a case study on the total viewshed problem · International Journal of Geographical Information Science*

This paper rearranges the elevation grid in memory, once per viewing direction, so that every line of sight lies along one row. Each GPU thread then does a simple running-maximum sweep along its row, so the visible area from every cell comes out at once. Each angular sector is independent, so sectors are handed out round-robin to several GPUs and the partial results are summed.

* *Type:* Total; approximate (sector-based, 1.18% absolute difference vs the earlier CPU TVS on the 2k DEM); azimuthal-sector sweep on a skewed DEM.
* *Hardware:* Intel Xeon E5-2698 v3 (16 cores) with 4 NVIDIA GTX 980 GPUs, CUDA 10.0 and OpenMP.
* *Input / output:* Input is a 2000^2, 4000^2 or 8000^2 DEM at 10 m (Montes de Malaga), and output is a same-size matrix giving the visible area from each cell.
* *Runtime:* Speedup and throughput on the 0-45 degree sectors only, extrapolated; up to 827x (4 GPUs, 4k DEM) vs the single-thread CPU total-viewshed algorithm of Tabik et al. 2014, and no full 360-degree wall-clock is reported.
* *Code:* yes, figshare doi 10.6084/m9.figshare.11370549 (preprint arXiv 2003.02200).
* *Gap for us:* This is the GPU total-viewshed baseline to beat, and it leaves open DEMs larger than GPU memory (the skewed copy is 2x the DEM), exactness against per-cell R2/R3, and tiling or load balancing for huge grids.

*2019 · Qarah & Tu · A Fast Exact Viewshed Algorithm on GPU · IEEE International Conference on Big Data (Big Data) 2019*

The paper makes the exact "rotating line" viewshed method (radial sweep, as accurate as R3 but far cheaper) run on a GPU for one observer. Every cell creates three events (add, check, remove) that are sorted by angle, so each GPU thread can handle its own slice of the circle. It replaces the usual pointer-based search tree with a pre-allocated static one, and packs three sort keys into one integer, which cuts memory and speeds up sorting. This is the conference version of the Qarah thesis, which extends it to larger maps.

* *Type:* Single observer; exact (same results as sequential radial sweep and R3); GPU radial sweep with merge-path sort, angle histogram, prefix sum and a static binary search tree.
* *Hardware:* NVIDIA Titan V for the GPU code and an AMD Ryzen Threadripper 1950X (16 cores, 2.15 GHz) for the sequential baselines, compiled with NVCC 10.
* *Input / output:* A 10,801 x 10,801 elevation grid of Germany (about 116 million cells, 0.43 GB) plus 5,000² and 10,000² SRTM grids in, and a visible/hidden value for every cell out.
* *Runtime:* 1.7 s on the 10,801² grid, about 3.5x faster than GPU R3 and about 6,200x faster than sequential R3 (just under 3 hours); the abstract says 1,627x over sequential radial sweep, but the body says 1,980x for that comparison and 1,627x for GPU R3 over CPU R3, so the abstract figure looks mislabeled.
* *Code:* none found.
* *Gap for us:* It computes one observer at a time and keeps about 5.9 GB of intermediate data for a 116-million-cell grid on one GPU, so repeating it for every cell is impractical and a total viewshed needs a different way of sharing work.

*2018 · Afshani et al. · An Efficient Algorithm for the 1D Total Visibility-Index Problem and Its Parallelization · ACM Journal of Experimental Algorithmics 23(2)*

This paper counts, for every point along a single line of terrain heights, how many other points it can see, without checking every pair. It splits the line in half again and again, and at each merge it counts the pairs that see each other across the two halves by turning sight lines into a geometry problem (counting where red and blue segments cross). This takes about n log^2 n steps instead of n^2, and two parallel versions spread the work over CPU cores. The 2D (map) version of the problem is left open.

* *Type:* Total (1D profile only, not a 2D DEM); exact; divide-and-conquer with red-blue segment intersection counting.
* *Hardware:* CPU only, C++ with OpenMP on an Intel Xeon E5-1620 (4 cores) and on two Intel Xeon E5-2680 (20 cores total).
* *Input / output:* Input is a 1D array of elevations (synthetic up to 2^27 points, real 2^16-point slices from a 90 m global dataset), and output is one visible-point count per vertex.
* *Runtime:* Wall-clock; over 100 M vertices in under 3 minutes on 16 threads, 5.6x to 18.7x faster than the general red-blue implementation of Palazzi and Snoeyink and "several orders of magnitude" faster than naive O(n^2) (exact factor not stated).
* *Code:* none found.
* *Gap for us:* It is 1D, exact and CPU-only with no known 2D or GPU extension, so a CUDA total-viewshed thesis can borrow only the count-instead-of-list idea and the memory-bandwidth lesson.

*2018 · Haverkort & Toma · A Comparison of I/O-Efficient Algorithms for Visibility Computation on Massive Grid Terrains · arXiv:1810.01946 (cs.DS; earlier ACM SIGSPATIAL 2009 and 2013)*

This paper compares five ways to compute what one viewpoint can see on terrain grids too big for memory, where the cost that matters is moving data between disk and RAM. Two sweep a ray around the viewpoint, one grows a visible region outward while keeping an approximate horizon, and two merge horizons under a linear-interpolation terrain model. All are sequential, and the contribution is fewer disk reads, not parallelism.

* *Type:* Single; exact (horizon merging) or approximate (radial and centrifugal sweeps); radial sweep, centrifugal sweep, horizon merging.
* *Hardware:* One core of a 2.83 GHz Intel CPU (exact model not stated) in an HP 220 blade with a 5400 rpm SATA disk and 512 MiB or 1 GiB RAM, no GPU.
* *Input / output:* Input is a real SRTM/USGS grid from 0.25 GiB up to 28.4 GiB (about 7.6 billion points), and output is a binary visibility grid of the same size for one viewpoint.
* *Runtime:* Wall-clock; io-radial3 took 3,009 s on a 3.97 GiB grid, more than 10x faster than the authors' 2008 io-radial1 (32,364 s), and io-centrifugal took 12,186 s on the 28.4 GiB grid, 2.1x faster than io-radial2 (derived).
* *Code:* none found.
* *Gap for us:* It is single-viewpoint, CPU and disk-bound with no total-viewshed or GPU result, so it offers only horizon-array and cache-friendly sweep ideas and an out-of-core baseline for grids larger than GPU memory.

*2017 · Li, Franklin, Magalhaes & Andrade · GPU-Accelerated Multiple Observer Siting · Photogrammetric Engineering & Remote Sensing 83(6)*

The paper makes a program that chooses the best spots for observers on a big terrain run much faster on a GPU. Its first step gives every cell a visibility index, using one thread per cell to test random targets within a limited radius. It then computes viewsheds for candidate observers and picks the final set greedily using bitwise operations. The CUDA version has seven kernels, and an OpenMP version is also provided.

* *Type:* Multi-observer siting; approximate (random-target sampling for the per-cell index, R2 viewsheds for candidates); Franklin-Vogt pipeline with greedy selection.
* *Hardware:* NVIDIA Tesla K20Xm with two Intel Xeon E5-2687W CPUs, nvcc 7.5.17, OpenMP run with 50 threads.
* *Input / output:* A 16,385x16,385 Puget Sound DEM in, and a 0-255 visibility index per cell plus about 15,000 sited observers out.
* *Runtime:* Wall-clock 17.2 s for CUDA versus 1,188.9 s for one sequential CPU core (about 69x), and 70.2 s for OpenMP (about 17x), against the authors' own sequential code.
* *Code:* https://github.com/wenlili/siting (from the reading list, not verified).
* *Gap for us:* It is not an exact total viewshed because it samples random targets and caps the radius at 100 cells, so exact or error-bounded all-cells visibility on a GPU is still open.

*2016 · Cervilla, Tabik, Vias, Merida & Romero · Total 3D-viewshed Map: Quantifying the Visible Volume in Digital Elevation Models · Transactions in GIS 21(3)*

Instead of counting visible cells, this paper estimates how much open space can be seen from every spot on the map. It sweeps the terrain direction by direction in 360 sectors and keeps a sliding band of nearby points that holds the candidate line of sight. Each visible segment's volume is estimated with a geometric formula, and loops are ordered to keep memory access local on a multicore CPU.

* *Type:* Total (new metric: visible volume); approximate (sectors and a fixed-width band, within about 10% of alternative volume estimates); sector sweep with a band-of-sight window.
* *Hardware:* Dual-socket Intel Xeon E5-2698 (core count not stated) and an Intel Sandy Bridge E5-2620, multicore CPU, no GPU.
* *Input / output:* A 2000x2000 (10 m) Malaga DEM in, and visible volume in cubic hectometres per cell out.
* *Runtime:* Wall-clock 1.3 min in the abstract (1.5 min in the body text, a discrepancy in the paper) for 4 M cells, with no speedup stated since no earlier method exists.
* *Code:* Validation code at a University of Malaga URL (probably www.ac.uma.es/~siham/Test.tar.gz, unverified); full code not stated as released.
* *Gap for us:* It is CPU-only and approximate with a sorting step per sector, so a CUDA port of the sector sweep and scaling far above 4 M cells remain open.

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

*2015 · Feng et al. · A parallel algorithm for viewshed analysis in three-dimensional Digital Earth · Computers & Geosciences 75*

The authors compute a viewshed inside a 3D globe viewer using the graphics pipeline instead of line-of-sight tests. Each terrain or building triangle is stretched away from the viewpoint to form a closed "shadow volume". A stencil buffer then counts how many volumes cover each screen pixel, and pixels with a count of zero are visible. Because this is just rendering, it works on terrain, buildings and trees together and needs no elevation interpolation.

* *Type:* single-observer; approximate (limited by mesh resolution and screen pixels); GPU shadow-volume / stencil-buffer rendering
* *Hardware:* NVIDIA GTX280, GeForce 9800 and GeForce 8600 GPUs (Core2 Q6600/Quad/Duo CPUs), using DirectX 9.0c vertex and pixel shaders in C#.
* *Input / output:* Triangle meshes extracted from a 30 m/6 m global DEM at three levels of detail (6, 2.5 and 0.6 m) in, a red/green visible-invisible screen overlay out.
* *Runtime:* Average time per viewshed, 5.2 ms versus 8.4 ms for a shadow-map GPU method on the GTX280 (about 35% faster on average), and 0.42 to 0.55 ms versus 0.97 to 4.56 ms for a CPU line-of-sight method across the three detail levels.
* *Code:* none found
* *Gap for us:* It is a graphics-pipeline method for one viewpoint on a screen-sized image, so it does not give an exact raster viewshed for every cell and cannot be reused directly in CUDA.

*2014 · Li, Franklin, Benedetti & Magalhaes · Parallel Multiple Observer Siting on Terrain · ACM SIGSPATIAL 2014*

The paper speeds up a program that picks good places for many lookout towers so that together they see most of a landscape. The program has four stages: estimate how visible each cell is from random targets, pick candidate spots, compute their viewsheds, and greedily choose the best set. The authors parallelize it with OpenMP on CPU cores and with CUDA on a GPU, after first improving the sequential code.

* *Type:* Multi-observer siting with an approximate per-cell visibility index; approximate (random-target sampling, R2 viewsheds, greedy set cover).
* *Hardware:* Two Intel Xeon E5-2687W CPUs (16 cores) and one NVIDIA Tesla K20Xm, OpenMP and CUDA.
* *Input / output:* Puget Sound DEMs from 1025x1025 up to 16385x16385 in, and a set of selected observers (25,374 covering 96.1% at the largest size) out.
* *Runtime:* Wall-clock on 16385x16385 was 975.52 s sequential, 75.67 s OpenMP (12.9x) and 29.61 s CUDA (32.9x), against the authors' own improved sequential code.
* *Code:* none found.
* *Gap for us:* Its visibility index is a random-sampled estimate used only to pick candidate sites, so exact all-cells visibility on a GPU is left open.

*2014 · Osterman, Benedicic & Ritosa · An IO-efficient parallel implementation of an R2 viewshed algorithm for large terrain maps on a CUDA GPU · International Journal of Geographical Information Science 28(11)*

Each CUDA thread walks one ray from the observer to the map edge and keeps the highest elevation angle seen so far. Cells take their value from the nearest ray, which gives a fast but slightly approximate viewshed. The map is cut into horizontal segments so it can be larger than memory, and disk reads, GPU copies and computation are overlapped. It is implemented as a GRASS GIS module called r.cuda.visibility.

* *Type:* Single-observer; approximate (99.52-99.89% cell agreement with R3); R2 with one thread per ray.
* *Hardware:* Intel Core i7-3930K, 32 GB RAM, SSD and one NVIDIA GeForce GTX 680, CUDA SDK 5.0.
* *Input / output:* Ten Slovenia DEMs from 0.48 GB up to 186 GB (50.5 billion cells, up-sampled) in, and a visibility map of the same size for one observer out.
* *Runtime:* Wall-clock 7 s total (0.7 s kernel) for 1.89 GB and 1,922 s total for 186 GB, about 10x faster than its own sequential version and about 200x to 3,000x faster than GRASS r.los at 1.89 GB (derived).
* *Code:* http://viewshed.s51mo.net (link from the paper, not verified).
* *Gap for us:* It uses only about 2(R+C) threads per observer and is approximate, so it offers little parallelism for one viewpoint and its errors would pile up over millions of observers.

*2012 · Tabik, Zapata & Romero · Simultaneous computation of total viewshed on large high resolution grids · International Journal of Geographical Information Science 27(4)*

The paper computes how much terrain is visible from every point of a map in one combined pass rather than redoing a single-viewpoint calculation for each point. For each of 360 one-degree directions it sorts the terrain points, then uses a tree of convex hulls to find each point's visible stretches quickly. A second, cheap stage fills in the opposite ends of those stretches from the reverse direction, which halves the storage and work.

* *Type:* Total viewshed; approximate (1-degree sectors); horizon / hull-tree sector sweep.
* *Hardware:* Intel Core 2 Duo 3.16 GHz PC with 3.25 GB RAM, C++ -O3, no GPU.
* *Input / output:* A 2000x2000 (10 m) Malaga DEM in, and a total viewshed map plus about 42 GB of intermediate per-sector files out.
* *Runtime:* Wall-clock about 6.5 h for the main stage, or 0.0063 s per viewpoint, versus about 10 s per viewpoint for ArcGIS (about 1,600x, derived) and 155 s for GRASS r.los (about 25,000x, derived).
* *Code:* none found.
* *Gap for us:* It is single-threaded CPU code that needs 42 GB of intermediate files for 4 M cells, so a GPU version must replace the hull tree and file storage and be checked against exact viewsheds.

*2012 · Timonen · Low-Complexity Intervisibility in Height Fields · Computer Graphics Forum 31(8)*

For every point of a heightmap and a fixed set of compass directions, the paper finds exactly where the line of sight is blocked and where it reopens. It sweeps the map along parallel lines and keeps a tree of convex hulls that is updated step by step, so the work depends only on how many visibility changes occur. On the GPU, one CUDA thread handles one line, and the per-direction results are added together.

* *Type:* Total-style (all-points horizons per direction, not a viewshed size map); exact per direction, approximate over K directions; incremental convex hull tree line sweep.
* *Hardware:* NVIDIA GTX 480 (Fermi) with CUDA 3 and OpenGL 4, CPU not stated.
* *Input / output:* Synthetic 1024x1024 height maps in, and per point and per direction a list of visibility horizons out.
* *Runtime:* Per-direction time on 1024x1024 was 0.52 to 8.85 ms versus 21.2 ms for the naive method (2.4x to 41x depending on the terrain).
* *Code:* none found.
* *Gap for us:* It does not produce viewshed sizes and was tested only on small synthetic grids, so a thesis would need to add area accumulation and test on large real DEMs.

*2012 · Zhao, Padmanabhan & Wang · A parallel computing approach to viewshed analysis of large terrain data using graphics processing units · International Journal of Geographical Information Science 27(2)*

For one observer, the GPU checks every terrain cell with its own thread using the exact R3 method. The terrain is cut into tiles so that maps far larger than memory can still be processed. Small sub-tiles are staged in fast on-chip shared memory so that memory access is regular. One version uses only global memory and another adds shared memory.

* *Type:* Single-observer; exact (R3, GPU differs by under 0.00002% from rounding); one CUDA thread per target cell.
* *Hardware:* One NCSA node with a 2.4 GHz AMD Opteron and one NVIDIA Tesla S1070 GPU (GT200, compute capability 1.3), CUDA SDK 4.0.
* *Input / output:* Utah rasters from 1.49 GB (20,000x20,000) up to 20.54 GB (74,256x74,256) in, and a visible/invisible map for one viewpoint out.
* *Runtime:* Wall-clock more than 50x faster than sequential R3 for datasets under 4 GB (best case 179 s versus 16,917 s, about 94x, derived), and 8,749 s compute for the 20.54 GB peak case.
* *Code:* none found.
* *Gap for us:* It computes one viewshed at O(n^3) per observer, so it says nothing about producing a viewshed size for every cell.

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

*2010 · Zhang, Yang, Sun & Lv · GPU-Accelerated Parallel Algorithms for Map Algebra · conference proceedings (venue not stated in the PDF text; pages around 882-885)*

This paper is not about viewshed or visibility. It shows how to move basic map algebra operations (cell-by-cell arithmetic on raster grids) to the GPU with CUDA, and it only implements one operator, SUM, which adds two raster layers cell by cell. It describes choosing block and thread sizes from the hardware limits and the data size, and it compares CPU and GPU times on five DEM sizes. It finds that the GPU wins on the largest grid, but data transfer cost makes it slower or no better on small grids.

* *Type:* Other (not viewshed or visibility; local map-algebra SUM operator); exact; trivially data-parallel per-cell kernel.
* *Hardware:* AMD 3600+ dual-core CPU at 1.9 GHz and NVIDIA GeForce 9500GT (128 MB), using CUDA 2.3.
* *Input / output:* Two DEM raster layers of the same size (largest 2775x3660) in, one summed raster out.
* *Runtime:* Not stated numerically in the text (results are only in a figure); the GPU is clearly faster only at the largest size (2775x3660) and is no better on small grids because of transfer time.
* *Code:* none found (a short SUM kernel is printed in the paper).
* *Gap for us:* Relevance low, because it involves no visibility or line-of-sight computation, only background on mapping raster operations to CUDA and on transfer overhead for small grids.

*2008 · Goradia, Ajmera & Chandran · GPU-based Hierarchical Computations for View Independent Visibility · Sixth Indian Conference on Computer Vision, Graphics & Image Processing (ICVGIP)*

The paper speeds up building a "visibility map" (V-map) that records which parts of a 3D point-cloud scene can see each other, which global-illumination rendering needs. Points are grouped in an octree, and groups that fully see each other get a single link. The GPU computes the visibility of every pair of leaf cells in parallel and stores it in a table, then the CPU walks the tree recursively and looks the answers up. This is view-independent: it computes visibility between all node pairs (hence all point pairs, at leaf-centroid granularity) rather than from one chosen eye. It is point-cloud visibility, not terrain viewsheds.

* *Type:* Other (all-pairs, view-independent visibility on 3D point clouds, not terrain); approximate/conservative (leaf centroids, sphere tests); octree hierarchy with CUDA leaf-pair parallelism.
* *Hardware:* An NVIDIA G80-class GPU (16 multiprocessors, 320 MB, exact card model not stated) with CUDA and a CPU host (CPU model not stated).
* *Input / output:* A point model (for example a Cornell room with a dragon, 100,000 to 130,000 points) in an adaptive octree goes in, and the V-map of visibility links for every node comes out.
* *Runtime:* Minutes to build the V-map; best speedup 19.17x at octree height 10 (Bunny: 1839.96 min on CPU vs 95.98 min on GPU), with an average of about 15x at height 10, and no gain at height 6 or below.
* *Code:* none found.
* *Gap for us:* It shows the idea of computing all-pairs visibility on a GPU with a hierarchy and reusing results, but it targets 3D point clouds on a 2008 G80 rather than total viewsheds on DEM grids.

*2004 · Llobera et al. · Calculating the Inherent Visual Structure of a Landscape ('Total Viewshed') Using High-Throughput Computing · CAA 2004 (proceedings 2010)*

The authors compute how much of a landscape is visible from every single point by splitting the work into 160 independent jobs. These jobs ran on idle university lab PCs managed by the Condor system. Each job runs a plain brute-force line-of-sight routine and stores only a count of visible cells. There is no algorithmic speed-up, so all of the gain comes from using many machines at once.

* *Type:* Total viewshed; approximate (no exactness claim); brute-force R3-style line of sight farmed out over a cluster.
* *Hardware:* Condor pool of 43 Windows NT PCs (13 Pentium 4 1.5 GHz and 30 Pentium 4 2 GHz), no GPU.
* *Input / output:* A 400x400 cell (50 m) study area inside a 1200x1200 DEM of Avebury, England in, and a 400x400 raster of visible-cell counts out.
* *Runtime:* Wall-clock 25 h 13 min on the pool versus an estimated 34 days on one similar PC (about 32x, derived).
* *Code:* none found.
* *Gap for us:* It only distributes an unoptimized per-cell CPU viewshed, so GPU mapping, memory-aware algorithms and comparison with modern exact methods are left open.

*2002 · Kaučič & Žalik · Comparison of Viewshed Algorithms on Regular Spaced Points · Spring Conference on Computer Graphics (SCCG), Budmerice*

This paper is a survey that implements and tests 11 variants of single-viewpoint viewshed algorithms on gridded terrain. It compares an exact method (R3 with linear interpolation) against faster approximate ones: R2, the XDraw family and line-drawing methods (Bresenham, 4-way stepping, 8-way stepping). Each is run for 20 view radii and 10 viewpoint positions, and judged by CPU time and by how many cells it labels wrongly compared with the exact method. The exact method is the most accurate but very slow, and the approximate ones have small errors that shrink as the radius grows.

* *Type:* Single viewpoint; exact (R3 interpolation) vs approximate (R2, XDraw variants, line-rasterisation methods); sequential CPU line-of-sight, wavefront and sweep families.
* *Hardware:* Portable PC with an Intel Celeron 500 MHz CPU, 64 MB RAM, Windows 98, C++ (no GPU).
* *Input / output:* A square-grid terrain (real Slovenia DTM at 100 m and 25 m resolution, plus fractal terrains) and a viewpoint 1.6 m above the ground go in, and a boolean visible/invisible matrix for the viewshed comes out.
* *Runtime:* CPU seconds averaged over 10 viewpoints; at radius 250 the exact method takes 33.844 s, R2 takes 0.606 s and 8WS takes 0.198 s (Table 1), and Table 5 puts 8WS at 0.8% of the exact method's time.
* *Code:* none found.
* *Gap for us:* It is purely sequential and single-viewpoint on a tiny 2002 CPU, so it only supplies the exact-versus-approximate accuracy and error-measurement baseline, not any total-viewshed or GPU method.

*1994 · Franklin & Ray · Higher isn't necessarily better: visibility algorithms and experiments · Advances in GIS Research, SDH '94*

This paper describes the three classic ways to work out what can be seen from one point on a terrain grid: R3 (check every cell separately), R2 (check only the edge cells and reuse the work along each ray) and XDraw (grow outward in square rings). It also shows a cheap way to estimate "how much can be seen from here" for every cell by firing only 16 to 32 rays from each cell. Experiments show that this visibility index is only weakly linked to elevation and that very few cells can see much of their surroundings.

* *Type:* Single-observer (R3, R2, XDraw) plus a total-style visibility index; R3 exact, R2 and XDraw approximate, index approximate (sampled rays).
* *Hardware:* Serial CPU only (Sun SPARCstation 10/30 and Sun IPC), no GPU.
* *Input / output:* 1201x1201 DTED level 1 elevation grids (about 90 m cells) in, and viewsheds or one weighted visibility index per cell out.
* *Runtime:* CPU seconds; R2 took 18 s versus 896 s for R3 from the same observer (derived ~50x), while the all-cells index took about 4.1 days per DEM cell with 32 rays on a Sun IPC (derived).
* *Code:* none found.
* *Gap for us:* Its all-cells index is a sampled estimate that took days of CPU time, so it gives no exact viewshed sizes and no GPU method.
