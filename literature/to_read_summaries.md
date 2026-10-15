1. Radial (rotational) sweep with event list

*2019 · Qarah & Tu · A Fast Exact Viewshed Algorithm on GPU · IEEE International Conference on Big Data 2019, pp. 3397–3405 · DOI not printed in the PDF · cited: not indexed*

My idea: it stores a lot of intermediary data and calculation. I am wondering whether that can be optimized with fancy iterators or a streaming approach. That way, there isn't much data stored. Also find a way to not store the full event list.

First GPU port of the exact radial sweep. Every cell emits three events (insert, query, remove). The events are sorted on the GPU (Merge Path) with angle, distance and type packed into one 64-bit key. A 360,000-bucket histogram plus prefix sum partitions the sorted list by angle, and each thread sweeps its own partition. Each thread has its own pre-allocated static BST status tree, initialised from the cells that the start ray crosses. Doubles are converted to integers to shrink intermediate data. Result: exact output, which the R2-style parallel sweep of Ferreira et al. does not give.

*Work ideas:* The sweep stage is 62% of runtime and the memory footprint (5.87 GB of events for 10K²) is the stated ceiling. Their own future work is multiple GPUs or streamed I/O with pinned host memory. The static-tree trick, which gave up to 44× on the sequential sweep alone. A parallel sweep that avoids storing the full event list would attack the limit directly.

*Optimizations:*
* *Static BST:* a pre-allocated array tree (32,767 slots per thread) replaces the pointer tree. No allocation or rebalancing, no pointers (about 80% less tree memory), and GPUs cannot allocate cheaply in global memory anyway. The cost is unused slots.
* *Doubles to integers:* angle and distance are scaled (90.233 becomes 90233) so one 64-bit key sorts all three fields, with event type breaking ties (delete, then query, then insert). Event data shrinks about 40%.
* *Sort and partition:* Merge Path sort in shared memory, then a histogram plus prefix sum split the list by angle so each thread owns one partition. They note the histogram could use shared memory, but it is only 7% of time.

* *Type:* Single; exact; radial sweep (Van Kreveld) on GPU, in-memory.
* *Hardware:* GPU: NVIDIA Titan V + AMD Threadripper 1950X (16 cores, 2.15 GHz), NVCC 10, Ubuntu 18.04.
* *Input / output:* Input is SRTM Germany 10,801×10,801 (0.43 GB); SRTM 5,000² and 10,000². Event list about 5.87 GB (compressed) and status trees about 0.61 GB for 5,000 threads, which is why 10K² is the ceiling on an 11 GB card. Output is per-cell visibility for one observer.
* *Runtime:* Wall-clock 1.7 s at 10,801². 3.5× vs GPU-R3 (6.5 s), about 1980× vs sequential radial sweep (56 min), about 6200× vs sequential R3 (just under 3 h). The abstract's "1627×" is GPU-R3 vs CPU-R3 *(derived)*, not the sweep. At 5,000² the gain over GPU-R3 is about 2×. Stage split: sweep 62%, sort 23%, histogram 7%, transfers about 5%.
* *Code:* None found.
* *Leads:* Zhao et al. 2013 (GPU-R3 baseline); Ferreira et al. 2014; Osterman et al. 2014; Stojanović & Stojanović 2013; Qarah 2020 (thesis, same author).

2. XDraw wavefront

*2015 · Cauchi-Saunders & Lewis · GPU enabled XDraw viewshed analysis · Journal of Parallel and Distributed Computing 84, 87–93 · [DOI](https://doi.org/10.1016/j.jpdc.2015.07.001) · cited 24× (OpenAlex, 2026-10-13)*

My thoughts: Linearize the XDraw rings so each ring is a contiguous array. That gives a better memory access pattern and one thread per ring cell, with no octant-specific indexing or tile edge cases. It mentions this DDA algorithm that I saw in other papers but am not familiar with. In this paper, each thread computes one ring cell, using the computation from the two inner-ring cells which more specifically are the two cells that are closest to the observer along the line of sight. The paper balances merging the 8 octants into two kernels instead, and talk about removing branching from the kernel, which I already read is very important.

First GPU XDraw. Each concentric square ring is computed in parallel (one thread per ring cell, using the two LOS values of the inner ring), with a barrier between rings. Eight octants are independent; the optimised XDraw-O merges them into two kernels. Branches are replaced by `max()`. A tiled buffer is filled per ring so neighbours are read in one transaction. R3, R2 and DDA are also ported to the GPU with few changes to serve as baselines.

*Work ideas:* Suggests future work optimizing DDA, R3 and R2, which beat the naive GPU XDraw. XDraw-O (optimized) was fastest, but the comparison is not fair to the other algorithms, which were only naively converted to GPU. Whether XDraw-O still wins once the others get the same tuning is left open. The authors point to Zhao et al. (DDA memory optimizations) and Ferreira (optimized sweep) for that.

*Optimizations:*
* *Tiled buffer (the main gain):* each cell needs 2 inner-ring LoS values and 1 elevation (3 global reads, 1 write). Per ring and octant, those values are copied once into a per-multiprocessor buffer (about 2 × octant width), so threads stop stalling on global reads.
* *Branches to `max()`:* the LoS update becomes a max, so no divergent paths where both sides run.
* *Eight kernels to two:* octants are grouped (N/S-leaning in kernel 1, E/W-leaning in kernel 2) to cut branching on "is this octant finished".
* *Less host copying:* input arrays are not copied back, only the visibility array is.
* *Noted but not tested:* more kernel variants, multiple GPUs, unified CPU/GPU memory, better dispatch, and the effect of core count. Plain GPU XDraw was slower than DDA and R3, possibly because C++ AMP picked poor tile sizes automatically.
* *Their hypothesis:* the two-kernel version won on thread throughput even though eight kernels have better memory contiguity, so XDraw may respond more to throughput than to access pattern. This is speculation, not isolated by an experiment. XDraw-O also showed edge artifacts they attribute to a tile edge case.

* *Type:* Single; approximate (XDraw interpolates from two inner-ring cells); XDraw (compared with R3, R2, DDA on CPU and GPU).
* *Hardware:* CPU and GPU (C++ AMP, vendor-neutral). AMD "6870" GPU (presumably Radeon HD 6870) with an Intel i5-3570 and 12 GB RAM; C++ AMP on the DirectX 11 driver, inside the EonFusion GIS.
* *Input / output:* Input is five Tasmania DEMs of 818,055 to 9,077,344 points; three observer positions each; 5 runs averaged. Output is a visibility array for one observer, same size as input.
* *Runtime:* Wall-clock averaged over 15 cases, excluding host–device transfers. XDraw-O 0.0375 s vs GPU XDraw 0.133 s (72% less), vs GPU R3 0.108 s (about 2.9× *(derived)*), GPU R2 0.166 s, GPU DDA 0.084 s, CPU XDraw 0.662 s (about 18× *(derived)*). The plain GPU XDraw was slower than GPU DDA and R3, so the gain comes from the tiling. Every GPU port beat its CPU form (80–95% less time).
* *Code:* None found.
* *Leads:* Franklin, Ray & Mehta 1994 (XDraw); Dou et al. 2018/2019 (parallel XDraw scheduling, Spark XDraw); Zhao et al. 2013 and Chao et al. 2011 (GPU R3 and shadow map).

3. R2 / boundary-ray sightlines

*2012 · Osterman · Implementation of the r.cuda.los module in the open source GRASS GIS by using parallel computation on the NVIDIA CUDA graphic cards · Elektrotehniški vestnik 79(1–2), 19–24 · No DOI found · cited: not indexed*

My thoughts: 90% of the the time is spent in I/O. This paper itself is not super interesting, it would be more interesting to read the follow-up that fixes this I/O problem. Each thread computes one ray from the observer to the map edge, naive.

A CUDA replacement for GRASS `r.los`. One thread per boundary cell (N = 2·C + 2·R threads) walks a ray from the observer to the map edge, keeps the running maximum vertical angle, and writes the angle to the output cell if it is not below that maximum. Earth-curvature correction is applied per step. Azimuth and tilt limits are added for radio-planning use. If the map exceeds about half of GPU memory it is processed in bands. The author reports that more than 90% of wall time is file and PCIe transfer.

*Work ideas:* Mostly a practical baseline: R2 on a GPU with almost no tuning, already 300-800× faster than GRASS `r.los`. Since I/O is over 90% of its time, the open question is how much of the kernel is left once transfer is hidden (streams, pinned memory). The follow-up Osterman et al. 2014 addresses I/O-efficient R2. It also fits the "naive ports" side of the Cauchi comparison.

*Optimizations:*
* *Constant memory* for values fixed during the kernel (max distance, azimuth and slope limits).
* *No block cooperation:* threads are independent, so block layout does not matter. Four sub-threads per boundary cell (N, S, W, E) set the walking direction.
* *Banding:* maps over about half of GPU memory are split into bands, processed starting from the band containing the observer.
* *Parallel output init:* the null fill of the output is itself done on the GPU.
* No shared-memory or coalescing tuning; global memory is used throughout because the maps are too big.

* *Type:* Single; approximate (one ray per boundary cell, nearest-cell steps, no interpolation; paper compares only by eye with r.los); R2-style boundary rays.
* *Hardware:* Single GPU. NVIDIA GeForce GTX 560 Ti (336 CUDA cores, 1 GB, CC 2.1), Intel Core 2 Duo E8200 2.66 GHz, 2 GB RAM, Linux; nvcc inside GRASS.
* *Input / output:* Input is integer/short DEM rasters at 100 m, 25 m and 12.5 m; the whole of Slovenia, 28,161×17,921 at 12.5 m. Output is a raster of vertical angle per visible cell (null elsewhere), scaled to integer; same size as input.
* *Runtime:* Wall-clock vs r.los. Slovenia at 12.5 m: 18 s total. Table 1: 100 m map 50 km range 44 s → 0.15 s (about 290×); 25 m map 20 km 511 s → 0.6 s (about 850×); 12.5 m 10 km 516 s → 1.2 s (about 430×) *(derived)*. r.los did not finish for the larger cases. Paper claims 10–1000×.
* *Code:* GRASS module; no repository given in the paper (none found).
* *Leads:* Osterman, Benedičič & Ritoša 2014 (follow-up I/O-efficient R2; cited by Qarah, Zhao, Gu); Huang et al. 2007 (cluster GRASS).

*2014 · Osterman, Benedičič & Ritoša · An IO-efficient parallel implementation of an R2 viewshed algorithm for large terrain maps on a CUDA GPU · International Journal of Geographical Information Science 28(11), 2304–2327 · [DOI](https://doi.org/10.1080/13658816.2014.918319) · cited 6× (Taylor & Francis page in the PDF)*

The follow-up to r.cuda.los, now called `r.cuda.visibility`. It keeps one thread per boundary ray (rays always go to the map edge, so N ≈ 2C + 2R) and adds linear interpolation between the two neighbouring cells. Only the ray that passes nearest a cell's centre decides that cell's visibility; neighbouring rays compare their offsets through shared memory. The map is cut into horizontal strips (segments) that fit in host and GPU memory. Strips are processed outward from the observer's strip, and a "horizon vector" carries each ray's maximum angle into the next strip. Disk reads, copies, the kernel and disk writes overlap using double buffers and two CUDA streams. This lets it handle maps far larger than RAM (up to 186 GB, 50 billion cells).

*Work ideas:* This is the tuned version of the 2012 baseline, so it is the R2 to beat, not the naive one. Once transfers are overlapped, the whole run is limited by disk throughput: the kernel is only about 12% of total time at 186 GB *(derived)*. So kernel speedups only matter for in-memory runs or many observers. The authors measured that east/west rays take about 2× longer than north/south rays because their reads are strided. That is a concrete target: transposing the map for the east/west half would make those reads coalesced as well. Their future work is only other interpolation methods. They also flag a parallel version of Izraelevitz 2003 (XDraw/LoS hybrid) as an open challenge.

*Optimizations:*
* *Floats with a step counter (the occupancy fix):* the naive kernel used doubles (37 registers, about 66% occupancy). Plain floats gave Moiré artefacts because `x += xstep` accumulates rounding error. Instead, an integer step counter is incremented and x, y are recomputed from scratch each step. This allowed floats with 28 registers and about 90% occupancy, at 512 threads per block.
* *Coalesced access:* rays toward north/south read neighbouring cells in one row, so one transaction serves a warp. East/west rays need about 4 reads for the same data. The authors call coalescing during segmentation the main reason for the speed.
* *Overlapped I/O:* pinned host memory, double buffers and two streams overlap disk, PCIe and kernel work. Segment kernels must run in order because of the horizon vector, but the copies and disk I/O around them overlap.
* *Intertwined blocks:* blocks cannot share memory, so the last two threads of one block and the first two of the next compute the same rays. This costs 2 extra threads per block (about 0.4% at 512 threads) and avoids a second kernel.
* *Tested and rejected:* texture memory gave about 10% faster kernels with hardware interpolation, but it fixes the segment size and limits interpolation choices, so it was dropped.
* *Segment count:* 3 to 50 segments barely changes runtime; past about 10,000 segments kernel launch overhead adds about 10%.

* *Type:* Single; approximate (R2 with interpolation and nearest-ray rule; 99.52–99.89% of cells agree with R3 over 10 observers, vs 93.2–97.5% for TiledVS); R2-style boundary rays, external memory.
* *Hardware:* Single GPU. NVIDIA GeForce GTX 680, Intel Core i7-3930K (6 cores, 3.2 GHz), 32 GB RAM, Corsair Force GT SSD, Ubuntu 11.04; CUDA 5.0.
* *Input / output:* Input is Slovenia DEMs at 25 m (14,081×8,961, 0.48 GB) and 12.5 m (28,161×17,921, 1.89 GB), plus eight upsampled versions up to 1.25 m (281,610×179,210, 186 GB); 4-byte integer heights. Output is a visibility raster of the same size, written to disk per segment.
* *Runtime:* Wall-clock including disk I/O. 0.48 GB: 1.8 s total (0.2 s kernel) vs R3 310 s (pit) / 2,776 s (peak) and TiledVS 28 s. 1.89 GB: 7 s vs R3 1,426 / 21,045 s. 186 GB: 1,922 s (235 s kernel, 218 segments) vs sequential `r.cpu.visibility` 20,474 s (about 10.7× *(derived)*); the table gives no TiledVS time at that size. Unlike R3, runtime does not depend on how much is visible.
* *Code:* Stated as available at http://viewshed.s51mo.net (not checked); also part of GRASS-RaPlaT.
* *Leads:* Ferreira et al. 2012 (TiledVS, external memory); Zhao et al. 2013 (GPU R3); Strnad 2011 and Gao et al. 2011 (texture-memory GPU viewsheds); Izraelevitz 2003; Toma 2012 (I/O-efficient viewsheds survey).

<div style="page-break-after: always;"></div>

4. Reference-line (PDERL)

*2022 · Cheng & Dou · HiPDERL: An Improved Implementation of the PDERL Viewshed Algorithm and Accuracy Analysis · ISPRS International Journal of Geo-Information 11(10), 506 · [DOI](https://doi.org/10.3390/ijgi11100506) · cited 6× (OpenAlex, 2026-10-13)*

Re-implementation of Wu et al.'s PDERL, which reorganises R3's work in a PDE coordinate system so that results are reused through a "reference polyline" of maximum elevation per direction, giving O(n²) with R3-like interpolation. The paper adds a checker that compares PDERL with R3 per cell. It finds three flaws in the original implementation: observers on grid points are mishandled (cells in the observer's column skipped), the error-avoidance mechanism is wrong, and float rounding leaks through the linked-list reuse. HiPDERL changes the reference-line data structure and the error handling so that errors are not carried forward.

*Work ideas:* Single-threaded CPU only, so not a GPU method. Its value is as an accuracy checker: HiPDERL is within 0.01% of R3 at near-XDraw speed, so it could be the ground truth for judging approximate GPU outputs on large grids where R3 is too slow. The authors suggest it as the baseline for total-viewshed work. Whether the reference polyline maps to a GPU is untested.

*Optimizations:*
* *Store coordinates, not slopes:* each linked-list node holds the (D, E) point directly, so E is read without traversing the list and rounding error does not accumulate.
* *Independent ΔE:* ΔE is recomputed from the current polyline rather than the previous ΔE, so errors are not passed along.
* *Extra intersection value:* the intersection's E is now also computed, which costs a little speed (about 45 s vs about 40 s for the original PDERL).
* *Fixes to the original:* correct handling of observers on grid points and a repaired error-avoidance step.
* Garbage collection for the linked list is implemented. No parallelism is used.

* *Type:* Single (total viewshed named as future work); exact in principle (the paper's goal is R3-equivalent output); PDERL (proximity-direction-elevation reference lines).
* *Hardware:* CPU (single thread). Windows 10, Intel Core i7-9750H 2.6 GHz, 16 GB; single-threaded C++.
* *Input / output:* Input is ASTER GDEM 3601×3601 at 30 m (mountain N28E097, hill N41E119, plain N34E114); 1,225 viewpoints, each viewshed a 201×201 square (range 100 cells, 49.49 M target cells per DEM). A 200×200 Málaga DEM (100 m) is used for the failure demos. Output is a binary viewshed per viewpoint; the paper reports error counts vs R3.
* *Runtime:* Wall-clock for 1,225 viewpoints. HiPDERL 45.7 / 45.2 / 44.7 s (mountain / hill / plain) vs R3 126.0 / 92.2 / 62.3 s (about 2.8× / 2.0× / 1.4× *(derived)*), PDERL about 40 s, XDraw about 37 s, HiXDraw about 44–45 s. Cells differing from R3: HiPDERL 0.001–0.009%; original PDERL 0.36–2.5%; XDraw 1.7–2.6%; HiXDraw 0.9–2.2%.
* *Code:* None found (the data statement says only that data are available online).
* *Leads:* Wu et al. (original PDERL); Zhu et al. (HiXDraw); Izraelevitz 2003 (XDraw back-tracing). Authors propose HiPDERL as an accuracy baseline for total-viewshed algorithms such as sDEM.

<div style="page-break-after: always;"></div>

5. Learned early termination of LoS

*2025 · Schwencke, Stütz & Bulatov · AI-Accelerated Viewshed Computation for High-Resolution Elevation Models · VISIGRAPP 2025 (GRAPP), pp. 251–258 · [DOI](https://doi.org/10.5220/0013168400003912) · cited 0× (OpenAlex, 2026-10-13)*

My thoughts: not particularly interesting. All of the results are essentially the accuracy of the neural network, and how much work the neural network saves the algorithm from doing. It doesn't actually have numbers for how much faster the neural network + viewshed is. It says the neural network takes 0.03 ms per LoS, but doesn't talk about whether NN + viewshed computation is faster than just the computation itself. I guess we could have one GPU running the NN and another GPU running the computation, and that would be very interesting...

A fully connected network reads the DTM and nDSM profile along a LoS (plus target height) and predicts the farthest visible point, so the exact slope sweep can stop there instead of going to the maximum view distance. A modified Huber loss penalises underestimation more than overestimation, with a visibility-weighted term. Because the network output is a per-LoS cutoff, it composes with any LoS-based total-viewshed code and parallelises across rays.

*Work ideas:* An orthogonal idea: it does not change how visibility is computed, only how far each ray goes. It could be bolted onto an R2 or GPU total-viewshed kernel to cut work. The gap is that it is only evaluated as a share of operations, not wall-clock on a GPU, and the inference cost (0.03 ms per LoS) has to be weighed against short rays. Their future work is skip connections, parallel networks and CNNs, not deeper nets.

*Optimizations:*
* *Early abort at the predicted farthest visible point (FVP):* before the FVP the sweep is exact, so only cells beyond the prediction can be wrong.
* *Asymmetric loss:* the modified Huber loss punishes predicting too early more than too late, and a safety margin (about 250 m, or 2% of range) shifts errors toward the safe side.
* *Compared with a fixed rule:* a simple 1° max-slope abort already helps (7.8-19.5% of the work), but the network does better (1.0-5.3%).
* *Batching:* inference runs in batches of 10,000 LoS.

* *Type:* Total (the LoS-based total-viewshed setting; the experiments are on single LoS profiles); approximate (a too-early prediction misses visible cells; unchanged otherwise); LoS sweep with a neural-network farthest-visible-point (FVP) cutoff.
* *Hardware:* CPU inference (no GPU). Intel Xeon Gold 6154 for inference (about 0.03 ms per LoS in batches of 10,000); no GPU used; training hardware not stated.
* *Input / output:* Input is DTM/DSM profiles of 10 km from Karsava (Latvia, flat, forested), Altmühltal (Bavaria, hilly) and Wetter (Hesse, hillier); 10% for training, 90% for validation; raster resolution not stated. Network 3: 2048-1024-512-256, input 2l+1. Output is a predicted FVP distance per LoS, plus the visibility result of the underlying LoS sweep.
* *Runtime:* Reported as share of computations, not wall-clock. Network alone needs 1.0–5.3% of the full computation at 99.5–99.75% accuracy; with a +250 m margin 3.4–7.7% at 99.8–99.9%; the simple 1° max-slope abort needs 7.8–19.5% at 99.8–99.99%; a 15° abort needs 38.7–54.0%. Absolute error about ±250 m (2% of range); worse in the hillier regions, and not tested on very mountainous terrain.
* *Code:* None found.
* *Leads:* Sánchez-Fernández 2021 (sDEM), Tabik et al. 2015, Qarah 2020, Wang et al. 2023 (reference planes), Zhang et al. 2021 (Spark XDraw). Needs combining with a GPU total-viewshed code to be compared on time.

<div style="page-break-after: always;"></div>

6. Max-pooling skip + reference-plane LOS hybrid (Matryoshka doll)

*2020 · Pan, Tang, Tjahjadi, Wu & Xiao · A Novel Rapid Method for Viewshed Computation on DEM through Max-Pooling and Min-Expected Height · ISPRS International Journal of Geo-Information 9(11), 633 · [DOI](https://doi.org/10.3390/ijgi9110633) · cited 5× (OpenAlex, 2026-10-14)*

A CPU hybrid of LOS and reference-plane methods. The DEM is split into eight symmetric sectors, each a Pascal-triangle of layers. The Matryoshka doll step uses LOS for the cells directly below and on the 45° diagonals of the current judgment point. It then uses the judgment point plus two neighbours as a reference plane for the middle cell of the next two layers, and moves the judgment point inward. On top of that, the DEM is max-pooled into 1 km × 1 km blocks. While walking a LOS, the minimum height needed to be visible in a block is compared with the block's maximum, and the whole block is skipped if it cannot be seen. Visibility itself is a cross-product test (two multiplications, three subtractions), and a hidden run in a valley is traversed by adding the LOS slope each step.

*Work ideas:* The block-skip idea is cheap and independent of the sweep, so it could be bolted onto a GPU R2 or sweep kernel as a per-ray early exit (a coarse max-height pyramid as the lookup). The paper reports a gain mainly on plains and little in mountains, which is the case where a GPU kernel has the most divergence to lose. The gap is that nothing is quantified: all results are figures, with no table of times, no hardware and no speedup factors. Accuracy is only compared against R3 as a similar/different rate per observer, so this is a lead rather than a baseline. The authors list GPU parallelisation as future work.

*Optimizations:*
* *Max-pooling skip:* the DEM is pooled to the maximum per block. If a block's maximum is below the minimum expected height of the LOS, none of its cells can be visible and the walk jumps to the next block.
* *Global termination:* once the minimum expected height passes 8,848 m the rest of the ray is dropped. They suggest using a per-region maximum instead as future work.
* *Earth-curvature correction:* heights are corrected once, before the sweep, rather than per step.
* *Reference-plane reuse:* each judgment point's two neighbours give visibility for two layers, so inner cells are not re-walked from the observer.
* *Cross product plus incremental slope:* the cross-product test is used for visible cells and plain slope addition inside hidden runs, so a valley costs one addition per cell.

* *Type:* Single; approximate (DEM points stand in for interpolated LOS points; accuracy falls with coarser DEMs and rougher terrain); LOS / reference-planes hybrid with block skipping.
* *Hardware:* Not stated. CPU implementation; no GPU used (GPU listed as future work).
* *Input / output:* Input is two areas of about 40 km² near Changsha (one flat with low hills, one mountainous), at 10 m (DLR SRTM X-SAR) and 30 m (ASTER GDEM V2); 12 observer points. Output is a binary viewshed per observer; the paper does not give grid sizes in cells.
* *Runtime:* Wall-clock, shown only as plots (Figures 18-19), so no numbers. The paper states that Matryoshka and reference planes are faster than R2 and LOS at 10 m, and R2 is much slower on high-resolution data. The pooling gain is large on plains and small in mountains. Accuracy: similar/different rate vs R3, shown as plots; the text says only "good performance" and that error grows with terrain relief and DEM cell size.
* *Code:* None found.
* *Leads:* Cohen (cross-product visibility test, ref. 26); Ferreira et al. (parallel radial sweep, ref. 19); Osterman (r.los on CUDA, ref. 20); Franklin & Ray 1994 (R2 / reference planes); Kaučič & Žalik 2002 (comparison of viewshed algorithms).

<div style="page-break-after: always;"></div>

7. View-tree (occlusion tree over the grid)

*2022 · Wang, Xiong, Guo, Zhang & Tang · A view-tree method to compute viewsheds from digital elevation models · International Journal of Geographical Information Science (online 2022) · [DOI](https://doi.org/10.1080/13658816.2022.2094385) · cited 14× (OpenAlex, 2026-10-14)*

The DEM is turned into a tree rooted at the observer. A cell C "view-blocks" B (seen from A) if C lies within a buffer of radius R around segment AB and between A and B (checked with a distance-to-line and two dot-product tests). Each cell's parent is the neighbour that blocks it, with the smallest R chosen if several do. The viewshed is then a depth-first or breadth-first traversal. Each node compares its actual slope with the running minimum visible slope inherited from its parent, so the LOS is never walked explicitly. When several neighbours block a cell, four variants are defined (Quick, Avg, Min, Max view-tree) for how the minimum visible slope is combined. Theoretical cost is O(N²) time and space.

*Work ideas:* The result is an XDraw-like cost (about 10% slower with DFS) with R2-like accuracy, and R is a knob between the two (about 0.5 L behaves like R3/R2, 0.7 L and above like XDraw). The interesting part is that the parent pointers do not depend on the elevations, only on the geometry and the observer cell. So the tree could be built once per observer position and reused across terrains, or the tree shape could be precomputed for a fixed relative layout. For a GPU, depth-first traversal is a poor fit, so the breadth-first variant would map to level-synchronous kernels much like XDraw rings (the tree depth is on the order of the ring count). No GPU version is described, and the paper gives no absolute times, so a GPU port is untested.

*Optimizations:*
* *View-block buffer radius R:* the only tuning knob, set between 0.45 L and 0.80 L (L is cell size). Below about 0.40 L the tree only covers the four cardinal rays and the result is meaningless; larger R is slower (+19% time from 0.45 L to 0.80 L with BFS).
* *Precomputation:* since the observer is fixed, vector moduli and the actual slopes can be computed in advance, trading O(N²) memory for time.
* *Traversal choice:* depth-first was fastest (about 10% slower than XDraw); breadth-first was about 19% slower. DFS is not recommended with the Avg/Min/Max variants since it cannot guarantee a node is visited after its parent's siblings.
* *Quick-view-tree:* decides each node from its parent only, and skips revisits.
* No parallelism is used.

* *Type:* Single; approximate (R3 used as ground truth; the output depends on R); view-tree / dynamic programming over a tree of neighbour occlusions.
* *Hardware:* Not stated (the paper says only that all methods ran on the same hardware); CPU, no GPU.
* *Input / output:* Input is 2 m DEMs of loess hilly-gully terrain (Suide County, Shanxi; mean altitude 1,035 m), plus a bedrock mountain region (Qinling, 2,146 m) and a loess ridge-gully region (Yan'an, 1,512 m); 15 observers at peaks, runoff nodes and saddles. A series of 10 DEMs from 50 to 500 cells radius is used for scaling. Output is a viewshed raster plus a minimum visible slope per cell.
* *Runtime:* Reported as a ratio to R3 (plots, no absolute times). The text says XDraw is fastest, view-tree DFS about 10% slower than XDraw, view-tree BFS about 19% slower, and R2 about 1.8× XDraw (so the abstract's "40% faster than R2" is consistent with this, *(derived)*). Accuracy vs R3 (loess hilly-gully, average of 15 observers): error rate / omission rate 2.78% / 8.45% for view-tree, 2.80% / 2.68% for R2, 6.56% / 17.07% for XDraw. Across three terrains the averages are 3.88% / 8.30% (view-tree), 4.28% / 2.81% (R2), 8.11% / 20.24% (XDraw). Omission is the weak point: more visible cells are marked invisible than with R2.
* *Code:* None found.
* *Leads:* Wu et al. 2021 (PDERL); Zhu et al. 2019 (HiXDraw); Izraelevitz 2003 (backtracking XDraw); Wang, Robinson & White 2000 (reference planes); Kaučič & Žalik 2002 (line rasterization R2); Sánchez-Fernández et al. 2021 (skewed DEM).
