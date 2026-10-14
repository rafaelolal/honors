1. Radial (rotational) sweep with event list

*2019 · Qarah & Tu · A Fast Exact Viewshed Algorithm on GPU · IEEE International Conference on Big Data 2019, pp. 3397–3405 · DOI not printed in the PDF · cited: not indexed*

First GPU port of the exact radial sweep. Every cell emits three events (insert, query, remove). The events are sorted on the GPU (Merge Path) with angle, distance and type packed into one 64-bit key. A 360,000-bucket histogram plus prefix sum partitions the sorted list by angle, and each thread sweeps its own partition. Each thread has its own pre-allocated static BST status tree, initialised from the cells that the start ray crosses. Doubles are converted to integers to shrink intermediate data. Result: exact output, which the R2-style parallel sweep of Ferreira et al. does not give.

* *Type:* Single; exact; radial sweep (Van Kreveld) on GPU, in-memory.
* *Hardware:* GPU: NVIDIA Titan V + AMD Threadripper 1950X (16 cores, 2.15 GHz), NVCC 10, Ubuntu 18.04.
* *Input / output:* Input is SRTM Germany 10,801×10,801 (0.43 GB); SRTM 5,000² and 10,000². Event list about 5.87 GB (compressed) and status trees about 0.61 GB for 5,000 threads, which is why 10K² is the ceiling on an 11 GB card. Output is per-cell visibility for one observer.
* *Runtime:* Wall-clock 1.7 s at 10,801². 3.5× vs GPU-R3 (6.5 s), about 1980× vs sequential radial sweep (56 min), about 6200× vs sequential R3 (just under 3 h). The abstract's "1627×" is GPU-R3 vs CPU-R3 *(derived)*, not the sweep. At 5,000² the gain over GPU-R3 is about 2×. Stage split: sweep 62%, sort 23%, histogram 7%, transfers about 5%.
* *Code:* None found.
* *Leads:* Zhao et al. 2013 (GPU-R3 baseline); Ferreira et al. 2014; Osterman et al. 2014; Stojanović & Stojanović 2013; Qarah 2020 (thesis, same author).

2. XDraw wavefront

*2015 · Cauchi-Saunders & Lewis · GPU enabled XDraw viewshed analysis · Journal of Parallel and Distributed Computing 84, 87–93 · [DOI](https://doi.org/10.1016/j.jpdc.2015.07.001) · cited 24× (OpenAlex, 2026-10-13)*

First GPU XDraw. Each concentric square ring is computed in parallel (one thread per ring cell, using the two LOS values of the inner ring), with a barrier between rings. Eight octants are independent; the optimised XDraw-O merges them into two kernels. Branches are replaced by `max()`. A tiled buffer is filled per ring so neighbours are read in one transaction. R3, R2 and DDA are also ported to the GPU with few changes to serve as baselines.

* *Type:* Single; approximate (XDraw interpolates from two inner-ring cells); XDraw (compared with R3, R2, DDA on CPU and GPU).
* *Hardware:* CPU and GPU (C++ AMP, vendor-neutral). AMD "6870" GPU (presumably Radeon HD 6870) with an Intel i5-3570 and 12 GB RAM; C++ AMP on the DirectX 11 driver, inside the EonFusion GIS.
* *Input / output:* Input is five Tasmania DEMs of 818,055 to 9,077,344 points; three observer positions each; 5 runs averaged. Output is a visibility array for one observer, same size as input.
* *Runtime:* Wall-clock averaged over 15 cases, excluding host–device transfers. XDraw-O 0.0375 s vs GPU XDraw 0.133 s (72% less), vs GPU R3 0.108 s (about 2.9× *(derived)*), GPU R2 0.166 s, GPU DDA 0.084 s, CPU XDraw 0.662 s (about 18× *(derived)*). The plain GPU XDraw was slower than GPU DDA and R3, so the gain comes from the tiling. Every GPU port beat its CPU form (80–95% less time).
* *Code:* None found.
* *Leads:* Franklin, Ray & Mehta 1994 (XDraw); Dou et al. 2018/2019 (parallel XDraw scheduling, Spark XDraw); Zhao et al. 2013 and Chao et al. 2011 (GPU R3 and shadow map).

3. R2 / boundary-ray sightlines

*2012 · Osterman · Implementation of the r.cuda.los module in the open source GRASS GIS by using parallel computation on the NVIDIA CUDA graphic cards · Elektrotehniški vestnik 79(1–2), 19–24 · No DOI found · cited: not indexed*

A CUDA replacement for GRASS `r.los`. One thread per boundary cell (N = 2·C + 2·R threads) walks a ray from the observer to the map edge, keeps the running maximum vertical angle, and writes the angle to the output cell if it is not below that maximum. Earth-curvature correction is applied per step. Azimuth and tilt limits are added for radio-planning use. If the map exceeds about half of GPU memory it is processed in bands. The author reports that more than 90% of wall time is file and PCIe transfer.

* *Type:* Single; approximate (one ray per boundary cell, nearest-cell steps, no interpolation; paper compares only by eye with r.los); R2-style boundary rays.
* *Hardware:* Single GPU. NVIDIA GeForce GTX 560 Ti (336 CUDA cores, 1 GB, CC 2.1), Intel Core 2 Duo E8200 2.66 GHz, 2 GB RAM, Linux; nvcc inside GRASS.
* *Input / output:* Input is integer/short DEM rasters at 100 m, 25 m and 12.5 m; the whole of Slovenia, 28,161×17,921 at 12.5 m. Output is a raster of vertical angle per visible cell (null elsewhere), scaled to integer; same size as input.
* *Runtime:* Wall-clock vs r.los. Slovenia at 12.5 m: 18 s total. Table 1: 100 m map 50 km range 44 s → 0.15 s (about 290×); 25 m map 20 km 511 s → 0.6 s (about 850×); 12.5 m 10 km 516 s → 1.2 s (about 430×) *(derived)*. r.los did not finish for the larger cases. Paper claims 10–1000×.
* *Code:* GRASS module; no repository given in the paper (none found).
* *Leads:* Osterman, Benedičič & Ritoša 2014 (follow-up I/O-efficient R2; cited by Qarah, Zhao, Gu); Huang et al. 2007 (cluster GRASS).

<div style="page-break-after: always;"></div>

4. Reference-line (PDERL)

*2022 · Cheng & Dou · HiPDERL: An Improved Implementation of the PDERL Viewshed Algorithm and Accuracy Analysis · ISPRS International Journal of Geo-Information 11(10), 506 · [DOI](https://doi.org/10.3390/ijgi11100506) · cited 6× (OpenAlex, 2026-10-13)*

Re-implementation of Wu et al.'s PDERL, which reorganises R3's work in a PDE coordinate system so that results are reused through a "reference polyline" of maximum elevation per direction, giving O(n²) with R3-like interpolation. The paper adds a checker that compares PDERL with R3 per cell. It finds three flaws in the original implementation: observers on grid points are mishandled (cells in the observer's column skipped), the error-avoidance mechanism is wrong, and float rounding leaks through the linked-list reuse. HiPDERL changes the reference-line data structure and the error handling so that errors are not carried forward.

* *Type:* Single (total viewshed named as future work); exact in principle (the paper's goal is R3-equivalent output); PDERL (proximity-direction-elevation reference lines).
* *Hardware:* CPU (single thread). Windows 10, Intel Core i7-9750H 2.6 GHz, 16 GB; single-threaded C++.
* *Input / output:* Input is ASTER GDEM 3601×3601 at 30 m (mountain N28E097, hill N41E119, plain N34E114); 1,225 viewpoints, each viewshed a 201×201 square (range 100 cells, 49.49 M target cells per DEM). A 200×200 Málaga DEM (100 m) is used for the failure demos. Output is a binary viewshed per viewpoint; the paper reports error counts vs R3.
* *Runtime:* Wall-clock for 1,225 viewpoints. HiPDERL 45.7 / 45.2 / 44.7 s (mountain / hill / plain) vs R3 126.0 / 92.2 / 62.3 s (about 2.8× / 2.0× / 1.4× *(derived)*), PDERL about 40 s, XDraw about 37 s, HiXDraw about 44–45 s. Cells differing from R3: HiPDERL 0.001–0.009%; original PDERL 0.36–2.5%; XDraw 1.7–2.6%; HiXDraw 0.9–2.2%.
* *Code:* None found (the data statement says only that data are available online).
* *Leads:* Wu et al. (original PDERL); Zhu et al. (HiXDraw); Izraelevitz 2003 (XDraw back-tracing). Authors propose HiPDERL as an accuracy baseline for total-viewshed algorithms such as sDEM.

<div style="page-break-after: always;"></div>

5. Learned early termination of LoS

*2025 · Schwencke, Stütz & Bulatov · AI-Accelerated Viewshed Computation for High-Resolution Elevation Models · VISIGRAPP 2025 (GRAPP), pp. 251–258 · [DOI](https://doi.org/10.5220/0013168400003912) · cited 0× (OpenAlex, 2026-10-13)*

A fully connected network reads the DTM and nDSM profile along a LoS (plus target height) and predicts the farthest visible point, so the exact slope sweep can stop there instead of going to the maximum view distance. A modified Huber loss penalises underestimation more than overestimation, with a visibility-weighted term. Because the network output is a per-LoS cutoff, it composes with any LoS-based total-viewshed code and parallelises across rays.

* *Type:* Total (the LoS-based total-viewshed setting; the experiments are on single LoS profiles); approximate (a too-early prediction misses visible cells; unchanged otherwise); LoS sweep with a neural-network farthest-visible-point (FVP) cutoff.
* *Hardware:* CPU inference (no GPU). Intel Xeon Gold 6154 for inference (about 0.03 ms per LoS in batches of 10,000); no GPU used; training hardware not stated.
* *Input / output:* Input is DTM/DSM profiles of 10 km from Karsava (Latvia, flat, forested), Altmühltal (Bavaria, hilly) and Wetter (Hesse, hillier); 10% for training, 90% for validation; raster resolution not stated. Network 3: 2048-1024-512-256, input 2l+1. Output is a predicted FVP distance per LoS, plus the visibility result of the underlying LoS sweep.
* *Runtime:* Reported as share of computations, not wall-clock. Network alone needs 1.0–5.3% of the full computation at 99.5–99.75% accuracy; with a +250 m margin 3.4–7.7% at 99.8–99.9%; the simple 1° max-slope abort needs 7.8–19.5% at 99.8–99.99%; a 15° abort needs 38.7–54.0%. Absolute error about ±250 m (2% of range); worse in the hillier regions, and not tested on very mountainous terrain.
* *Code:* None found.
* *Leads:* Sánchez-Fernández 2021 (sDEM), Tabik et al. 2015, Qarah 2020, Wang et al. 2023 (reference planes), Zhang et al. 2021 (Spark XDraw). Needs combining with a GPU total-viewshed code to be compared on time.
