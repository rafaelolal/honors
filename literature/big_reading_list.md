# Reading List: Parallel Single & Total Viewshed Algorithms

Thesis angle: a **CUDA total-viewshed** implementation that fills a gap in, or improves on, existing work.
Last updated: 2026-09-28.

**How to read this list**
- Each section is in chronological order, **newest first**. The year is the first-publication year (online-first for journals).
- ★ = read first (most relevant to a CUDA total-viewshed thesis).
- 🆕 = added 2026-09-28 in a second pass limited to 2022–2026 work.
- **Coverage gap: Chinese-language journals.** Chinese journals (测绘学报, 地球信息科学学报, 武汉大学学报·信息科学版 …) register DOIs outside Crossref and sit behind CNKI, so this search could only reach them through a few journal websites. Many Chinese groups (Gao, Dou, Tang, Xiao, Wang) also publish in English and are covered here. For a full sweep, search CNKI through your library for 可视域 GPU / 可视域 并行 / 累积视域 / 通视分析, 2022 onward.
- **DOI** links go to the publisher. **PDF** means an open-access copy exists (free to download).
- Tags: `scope` (single / multi-observer / total) · `platform` · a one-line note.
- Citation counts come from OpenAlex (Sep 2026) and are shown only where they help.

---

## 0. Start here: the most relevant work from 2022–2026

The sections below keep the older classics because your thesis must cite them as baselines. Read these recent ones first. Full entries are in the sections named in the last column.

| Year | Work | Why it matters for a CUDA total viewshed | § |
|---|---|---|---|
| 2026 | AllTheLines / CacheTVS ([code](https://github.com/AllTheLines/CacheTVS)) | Whole-Earth total viewshed on CPUs; Vulkan GPU kernel removed May 2026, CUDA backend "TBC" | 1 |
| 2026 | Cheng & Dou, "Beyond Efficiency" (**paper not published yet**; only its [dataset](https://doi.org/10.21227/e8hd-2q44) is public) | Benchmark of four total-viewshed algorithms, probably including accuracy. Submitted to IEEE JSTARS; the dataset was deposited 2026-08-12 and links to no article (checked 2026-10-02) | 1 |
| 2026 | Lukač & Žalik ([DOI](https://doi.org/10.3390/rs18173044)) | Every cell's viewshed stored compactly on a GPU | 1 |
| 2026 | Wang, Zhu & Dou ([DOI](https://doi.org/10.1007/s11442-026-2493-x)) | Compressing viewshed output (niche 3) | 5 |
| 2025 | Gao et al., ML-R3 ([DOI](https://doi.org/10.1080/13658816.2025.2578421)) | Newest exact CUDA viewshed, with public code | 2 |
| 2025 | Chen et al. ([DOI](https://doi.org/10.1109/JSTARS.2025.3602482)) | Storing many observers' viewsheds (98% smaller) | 1 |
| 2024 | SkewEngine ([DOI](https://doi.org/10.1007/s11227-024-05923-2)) | The sDEM trick generalized; CUDA code on GitHub | 1 |
| 2024 | Xiao et al. ([DOI](https://doi.org/10.1080/17538947.2024.2308707)) | GPU multi-viewpoint viewshed (shadow maps) | 2 |
| 2022 | Sánchez-Fernández PhD thesis ([handle](https://hdl.handle.net/10630/24362)) | Full write-up of sDEM on CPU+GPU | 1 |
| 2022 | Kuna et al. ([DOI](https://doi.org/10.1016/j.jas.2022.105596)) | Large total viewshed on a cluster (every 4th cell) | 1 |
| 2026 | SOLWEIG-GPU ([DOI](https://doi.org/10.21105/joss.09535)) | Open-source GPU code for a per-cell sky-view factor over whole cities | 1 |
| 2025–26 | van der Waal SVF-GPU ([DOI](https://doi.org/10.1007/978-3-031-97657-5_24)); Hodge HyperBall ([arXiv](https://arxiv.org/pdf/2604.08374)); Fritsch horizon maps ([DOI](https://doi.org/10.2312/hpg.20251171)) | Current GPU techniques for all-points visibility | 1 |
| 2026 | Meneses et al., RT-cores review ([arXiv](https://arxiv.org/pdf/2603.28771)) | Whether RT cores could do the line-of-sight tests | 2 |
| 2022–26 | view-tree ([DOI](https://doi.org/10.1080/13658816.2022.2094385)), XPDERL ([DOI](https://doi.org/10.1016/j.cageo.2022.105213)), LoS zoning ([DOI](https://doi.org/10.1080/13658816.2023.2254825)), DPDERL ([DOI](https://doi.org/10.1080/13658816.2024.2408283)), MRVd ([DOI](https://doi.org/10.1080/13658816.2026.2624786)) | Recent sequential algorithms; this search found no GPU version of any of them | 5 |
| 2024 | EPVPS ([DOI](https://doi.org/10.1080/13658816.2024.2408279)); SMV ([DOI](https://doi.org/10.1080/13658816.2024.2354822)) | Observer siting that needs every cell's viewshed size | 4 |

---

## 1. Total viewshed: parallel / GPU / high-throughput

- **2026** · Lukač & Žalik · *GPU-Based Solar Irradiance Estimation over Digital Surface Models Using Structurally Lossless Viewshed Compression* · Remote Sensing 18(17)
  [DOI](https://doi.org/10.3390/rs18173044) (open access) · `all-points viewsheds` · `GPU (24 GB)` · Stores every cell's viewshed compactly on the GPU; 29.6 GB of viewshed data made to fit, 6.7 h → 0.5 h.
- ★ **2026** · Buckley-Houston & Berger · **AllTheLines / CacheTVS: total viewshed of the whole Earth** (code + blog posts, no paper)
  [Code](https://github.com/AllTheLines/CacheTVS) · [Blog 1](https://ryan.berge.rs/posts/total-viewshed-algorithm/) · [Blog 2](https://tombh.co.uk/longest-line-of-sight) · [alltheviews.world](https://alltheviews.world) · `total` · `CPU SIMD (AVX2/AVX-512)`
  - **Algorithm:** Tabik 2014's band-of-sight sectors over 360 angles. For each angle it *rotates* the DEM (where sDEM skews it) so lines of sight are contiguous in memory, then runs a prefix-max scan along each line, with Earth curvature and refraction.
  - **Scale:** about 4.5 billion viewpoints at about 100 m resolution, 5 × 96-core AMD machines for about 18 h, about 200 GB of output.
  - **GPU:** its Vulkan GPU kernel was slower than the CPU kernel and was removed in May 2026, and the CUDA backend is listed as "TBC". **This is a direct opening for a CUDA thesis.**
- **2026** · Cheng & Dou (Nanjing Normal Univ.) · *"Beyond Efficiency: …" on total viewshed algorithms* (submitted to IEEE JSTARS; not yet published)
  [Dataset DOI](https://doi.org/10.21227/e8hd-2q44) · [DataPort page](https://ieee-dataport.org/documents/detailed-data-figure-9-research-article-viewshed-analysis-algorithms-bosbasic-and-rip) · `total` · Per-angle runtimes of four total-viewshed algorithms: TVS (Tabik 2014), sDEM (Sánchez-Fernández 2020), BOSBASIC and RIP. BOSBASIC and RIP appear nowhere else and are probably introduced in this paper. The title suggests it benchmarks **accuracy**, not just speed. **Watch for publication.**
- **2025** · Chen et al. · *Framework for Organizing Viewsheds From Multiple Observation Points: Enabling Bidirectional Landscape Visibility Information Queries* · IEEE JSTARS
  [DOI](https://doi.org/10.1109/JSTARS.2025.3602482) (open access) · `multi-observer (storage)` · `CPU` · An observer × row × col tensor with run-length encoding uses 98% less storage. Relevant to storing total-viewshed *output*.
- ★ **2024** · F. Romero, Ortigosa, Bandera & L.F. Romero · *SkewEngine: enhancing performance of intensive calculations on regular meshes* · J. Supercomputing
  [DOI](https://doi.org/10.1007/s11227-024-05923-2) · [PDF](https://link.springer.com/content/pdf/10.1007/s11227-024-05923-2.pdf) · `total (case study)` · Generalizes the "skewed DEM" memory-reorganization trick into a reusable engine.
- **2023** · Mistick, Campbell, Thompson & Dennison · *Using airborne lidar and machine learning to predict visibility across diverse vegetation and terrain conditions* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2023.2224421) · `total (visibility index, predicted)` · `CPU + random forests` · `approx (ML)` · An ML alternative to computing every viewshed.
- **2022** · Kuna et al. · *Computing and testing extensive total viewsheds: a case of prehistoric burial mounds in Bohemia* · J. Archaeological Science
  [DOI](https://doi.org/10.1016/j.jas.2022.105596) · [Data (Zenodo)](https://zenodo.org/records/5764173) · `total (sampled: every 4th cell)` · `supercomputer (IT4Innovations), MATLAB R2` · `approx (R2)` · About 57,000 km² of Bohemia; eight radius/resolution models from 0.5 km at 5 m to 64 km at 320 m.
- **2022** · Sánchez-Fernández · *Novel parallel approaches to efficiently solve spatial problems on heterogeneous CPU-GPU systems* · PhD thesis, Univ. of Málaga
  [Link](https://hdl.handle.net/10630/24362) · `total` · `CPU-GPU` · Collects sDEM, VPP and ALFI. **The full details behind the 2020 paper.**
- **2022** · Cimburová & Blumentrath · *Viewshed-based modelling of visual exposure to urban greenery – An efficient GIS tool for practical planning applications* · Landscape & Urban Planning
  [DOI](https://doi.org/10.1016/j.landurbplan.2022.104395) (open access) · `exposure at every cell` · `CPU multicore (GRASS r.viewshed.exposure)` · Sampling density trades accuracy for time.
- **2022** · Brinkmann et al. · *Modelling eye-level visibility of urban green space: Optimising city-wide point-based viewshed computations through prototyping* · AGILE GIScience Series 3
  [DOI](https://doi.org/10.5194/agile-giss-3-27-2022) (open access) · `city-wide total` · `CPU` · `exact` · Reuses one precomputed line-of-sight "template" for every point; 34% faster.
- **2021** · Sánchez-Fernández, Romero, Bandera & Tabik · *VPP: Visibility-Based Path Planning Heuristic for Monitoring Large Regions of Complex Terrain Using a UAV Onboard Camera* · IEEE JSTARS
  [DOI](https://doi.org/10.1109/jstars.2021.3134948) · [PDF](https://ieeexplore.ieee.org/ielx7/4609443/9656571/09647997.pdf) · `total (application)` · Uses fast total viewshed as a building block.
- ★ **2020** · Sánchez-Fernández, Romero, Bandera & Tabik · *A data relocation approach for terrain surface analysis on multi-GPU systems: a case study on the total viewshed problem* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2020.1844207) · [PDF (arXiv)](https://arxiv.org/pdf/2003.02200) · [Code](https://doi.org/10.6084/m9.figshare.11370549) · `total` · `single-core, multicore, GPU, multi-GPU` · sDEM: 8.8× faster than GIS software on average, and 827× faster than the previous state of the art in the best case. **Likely the paper to beat.**
- **2019** · Wu, Ma & Chen · *HiVewshed: An Interactive Online Viewshed Analysis System for Multiple Observers* · CSAE '19
  [DOI](https://doi.org/10.1145/3331453.3361327) · `multi-observer / total` · `cluster (MPI + OpenMP)` · Several hundred observers at once.
- **2018** · Cervilla · *Algoritmos multicore para el cálculo de parámetros de visibilidad en SIG* · PhD thesis (Spanish), Univ. of Málaga
  [Link](https://dialnet.unirioja.es/servlet/tesis?codigo=256397) · `total / 3D` · `CPU multicore` · Thesis behind the 2015–2016 papers.
- **2018** · Afshani, de Berg, Casanova, Karsin, et al. · *An Efficient Algorithm for the 1D Total Visibility-Index Problem and Its Parallelization* · ACM J. Experimental Algorithmics
  [DOI](https://doi.org/10.1145/3209685) · [PDF](https://dl.acm.org/doi/pdf/10.1145/3209685) · `total (1D)` · `CPU multicore (16 cores)` · First sub-quadratic exact algorithm, O(n log² n); 100M vertices in under 3 min at 85% parallel efficiency. Conference version: ALENEX 2017 [DOI](https://doi.org/10.1137/1.9781611974768.18).
- **2018** · Prescott · *An Output Sensitive Algorithm for Computing Viewsheds and Total Viewsheds on 2D Terrains* · Bowdoin College **honors thesis**
  [Link](https://digitalcommons.bowdoin.edu/honorsprojects/93) · `single + total` · `CPU (sequential)` · Exact multiresolution approach, over 20× faster than the previous fastest. Also a useful model for scoping an honors thesis.
- **2018** · Prescott & Toma · *A multiresolution approach for viewsheds on 2D terrains* · ACM SIGSPATIAL '18
  [DOI](https://doi.org/10.1145/3274895.3274990) · `single/total` · `CPU`
- **2016** · Cervilla, Tabik, Vías, Mérida & Romero · *Total 3D-viewshed Map: Quantifying the Visible Volume in Digital Elevation Models* · Transactions in GIS
  [DOI](https://doi.org/10.1111/tgis.12216) · [PDF](https://hdl.handle.net/10630/29115) · `total (3D volume)` · `CPU multicore (Xeon E5-2698)` · 4M-point DEM in 1.3 min.
- **2015** · Cervilla, Tabik & Romero · *3D-viewshed map* · ACM SIGSPATIAL '15
  [DOI](https://doi.org/10.1145/2820783.2820865) · `total (3D)` · Short conference version of the 2016 paper.
- **2015** · Cervilla, Tabik & Romero · *Siting Multiple Observers for Maximum Coverage: An Accurate Approach* · Procedia Computer Science (ICCS)
  [DOI](https://doi.org/10.1016/j.procs.2015.05.255) · [PDF](https://www.sciencedirect.com/science/article/pii/S1877050915010637/pdf) · `total → siting` · "Masked" total-viewshed maps.
- ★ **2014** · Tabik, Cervilla, Zapata & Romero · *Efficient Data Structure and Highly Scalable Algorithm for Total-Viewshed Computation* · IEEE JSTARS
  [DOI](https://doi.org/10.1109/jstars.2014.2326252) · [Code](https://github.com/SihamTabik/Total_Viewshed) · `total` · `CPU multicore` · Basis of the "TVS" algorithm that the AllTheLines/CacheTVS project reimplements (§6).
- ★ **2012** · Tabik, Zapata & Romero · *Simultaneous computation of total viewshed on large high resolution grids* · IJGIS 27(4)
  [DOI](https://doi.org/10.1080/13658816.2012.677538) · `total` · `CPU` · Sector-based ("visible heights" + ring-sector) algorithm; the foundational total-viewshed paper, cited 61×.
- **2007** · Blum, Turner & Winterbottom · *Cumulative Viewshed Analysis using GRID Computing* · GeoComputation 2007
  [PDF](http://www.geocomputation.org/2007/6B-Algorithms_and_Architecture_2/6B1.pdf) · `cumulative/total` · `grid computing (HTC)` · Extended abstract.
- ★ **2004** · Llobera, Wheatley, Steele, Cox & Parchment · *Calculating the inherent visual structure of a landscape ('total viewshed') using high-throughput computing* · CAA 2004 (proceedings published 2010)
  [PDF](https://proceedings.caaconference.org/files/2004/27_Llobera_et_al_CAA_2004.pdf) · [UCL record](https://discovery.ucl.ac.uk/15486/) · `total` · `HTC (Condor, 43 workstations)` · 34 days → 25 h. **The "high-throughput" origin paper.**
- **2003** · Mineter et al. · *High-throughput Computing to Enhance Intervisibility Analysis* · GeoComputation 2003
  [Semantic Scholar](https://www.semanticscholar.org/paper/3e979577db141c373e450e3e7dbcf23c5f0c53f5) · `multi-observer intervisibility` · `HTC` · Predates Llobera; cited by Tabik 2014.
- ★ **1997** · Kidner, Rallings & Ware · *Parallel Processing for Terrain Analysis in GIS: Visibility as a Case Study* · GeoInformatica
  [DOI](https://doi.org/10.1023/A:1009740712769) · `total (visibility indices)` · `Transputer network` · **The earliest parallel total-visibility work** (48 citations on Semantic Scholar); cited by Tabik 2012 and Afshani 2018.

### All-points horizons & related all-pairs visibility (closely related to total viewshed)
- 🆕 **2026** · Kamath, Sudharsan, Singh et al. · *SOLWEIG-GPU: GPU-Accelerated Thermal Comfort Modeling Framework for Urban Digital Twins* · J. Open Source Software
  [DOI](https://doi.org/10.21105/joss.09535) · [PDF](https://joss.theoj.org/papers/10.21105/joss.09535.pdf) · `all-points sky-view factor + shading` · `GPU (Python)` · Runs the SOLWEIG model on GPUs at metre resolution over whole cities, including a sky-view factor for every cell. Open source: a working all-cells, visibility-type GPU code.
- 🆕 **2026** · Zonato, Kamath, Sudharsan et al. · *GLIDE-SOL: a GPU-accelerated global lightweight infrastructure for diagnostic environmental modeling with SOLWEIG* · Geoscientific Model Development 19
  [DOI](https://doi.org/10.5194/gmd-19-7389-2026) (open access) · `all-points SVF / shadows` · `GPU` · Scales the GPU SOLWEIG to multi-city runs from global datasets.
- **2026** · Hodge et al. · *City-Scale Visibility Graph Analysis via GPU-Accelerated HyperBall* · arXiv
  [DOI](https://doi.org/10.48550/arXiv.2604.08374) · [PDF (arXiv)](https://arxiv.org/pdf/2604.08374) · `all-pairs (urban visibility graph, not a DEM)` · `GPU (CUDA)` · `approx (HyperLogLog)` · Tangential, but shows current CUDA techniques for all-pairs visibility.
- 🆕 **2025** · Lukač, Mongus, Štumberger & Bizjak · *GPU-accelerated Sparse Voxel Octree for Irradiance Simulation using Remote Sensing Data* · ICRERA 2025
  [DOI](https://doi.org/10.1109/icrera66237.2025.11284254) · `all-points visibility (3D voxels)` · `GPGPU` · Spherical ray casting through a sparse voxel octree built from LiDAR (Vaihingen, New York City). Same group as Lukač & Žalik 2026 (§1).
- **2025** · Fritsch et al. · *Fast Planetary Shadows using Fourier-Compressed Horizon Maps* · High Performance Graphics 2025
  [DOI](https://doi.org/10.2312/hpg.20251171) · `all-points horizon` · `GPU, real-time` · Compressed per-texel horizon storage; cites Tabik 2011 and Stewart 1998.
- **2025** · van der Waal et al. · *Continuous Sky View Factor Calculations Using a Parallel GPU Workflow* · ICCSA 2025 Workshops (LNCS 15898)
  [DOI](https://doi.org/10.1007/978-3-031-97657-5_24) · [Abstract (open access)](https://doi.org/10.5194/icuc12-179) · `all-points horizon (SVF)` · `GPU (PyCUDA)` · Rotterdam 1 m DSM; about 99% less runtime than the CPU tool (UMEP).
- 🆕 **2024** · Sovetkin, Gerber & Pieters · *Fast Horizon Approximation: Impacts on Integrated Photovoltaic Irradiation Simulations* · Solar RRL
  [DOI](https://doi.org/10.1002/solr.202400474) · [PDF](https://onlinelibrary.wiley.com/doi/pdfdirect/10.1002/solr.202400474) · `all-points horizon` · `approx` · Same accuracy about 4× faster than earlier horizon methods. Companion to their 2024 IJGIS paper.
- **2024** · Lukač et al. · *Novel GPU-accelerated high-resolution solar potential estimation in urban areas by using a modified diffuse irradiance model* · Applied Energy 353
  [DOI](https://doi.org/10.1016/j.apenergy.2023.122129) (open access) · `all-points view factors` · `GPU (CUDA)` · 70× faster than multithreaded CPU. Predecessor of Lukač & Žalik 2026.
- **2024** · Sovetkin, Gerber & Pieters · *Improving horizon computation algorithm with quasirandom sequences* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2024.2408751) (open access) · `all-points horizon` · `CPU` · `approx` · 4× faster than the previous approximation at equal accuracy; cites Tabik 2014.
- 🆕 **2022** · Li & Wang · *GPU Parallel Computing for Predicting and Mapping Sun Glare* · SSRN preprint
  [DOI](https://doi.org/10.2139/ssrn.4076763) · `sun visibility from LiDAR` · `GPU` · Preprint. GPU line-of-sight to the sun at every location, at city scale.
- **2022** · Steger, Steger & Schär · *HORAYZON v1.2: an efficient and flexible ray-tracing algorithm to compute horizon and sky view factor* · Geoscientific Model Development
  [DOI](https://doi.org/10.5194/gmd-15-6817-2022) (open access) · `all-points horizon` · `ray tracing`
- **2021** · Dozier · *Revisiting Topographic Horizons in the Era of Big Data and Parallel Computing* · IEEE GRSL
  [DOI](https://doi.org/10.1109/lgrs.2021.3125278) · `all-points horizon` · `CPU multicore` · Parallelizes across rotated-grid columns, angles or tiles.
- **2021** · Li et al. · *GPU parallel computing for mapping urban outdoor heat exposure* · Theoretical and Applied Climatology
  [DOI](https://doi.org/10.1007/s00704-021-03692-z) · [Preprint PDF](https://www.researchsquare.com/article/rs-279602/latest.pdf) · `all-points SVF` · `GPU (NVIDIA, PC)` · All of Philadelphia in under 20 min.
- **2013** · Tabik et al. · *Optimal tilt and orientation maps: a multi-algorithm approach for heterogeneous multicore-GPU systems* · J. Supercomputing
  [DOI](https://doi.org/10.1007/s11227-013-0891-1) · `all-points horizon (solar)` · `multicore CPU + GPU` · Málaga group's heterogeneous horizon work. Companion paper: Tabik et al. 2012, *A Fast GIS-tool to Compute the Maximum Solar Energy on Very Large Terrains*, Procedia CS [DOI](https://doi.org/10.1016/j.procs.2012.04.039) · [PDF](https://www.sciencedirect.com/science/article/pii/S1877050912001603/pdf) (GPU–CPU).
- ★ **2012** · Timonen · *Low-Complexity Intervisibility in Height Fields* · Computer Graphics Forum
  [DOI](https://doi.org/10.1111/j.1467-8659.2012.03155.x) · [PDF](http://wili.cc/research/hfivisib/hfivisib.pdf) · `all points (horizons in discrete directions)` · `GPU (CUDA + OpenGL, Fermi)` · Incremental "convex hull tree" sweep: 100× fewer iterations, 2.4–41× faster. **Essentially a CUDA sector-sweep template for total viewshed.**
- **2011** · Tabik, Romero & Zapata · *High-performance three-horizon composition algorithm for large-scale terrains* · IJGIS
  [DOI](https://doi.org/10.1080/13658810903149995) · `all-points horizon` · `multicore CPU` · Stewart-based; the direct ancestor of Tabik's total viewshed.
- **2010** · Timonen & Westerholm · *Scalable Height Field Self-Shadowing* · Computer Graphics Forum
  [DOI](https://doi.org/10.1111/j.1467-8659.2009.01642.x) · `all-points horizon` · `GPGPU` · Precursor to Timonen 2012.
- **1998** · Stewart · *Fast horizon computation at all points of a terrain with visibility and shading applications* · IEEE TVCG
  [DOI](https://doi.org/10.1109/2945.675656) · `all-points horizon` · Classic sweep-based horizon algorithm.


### Recent large-scale cumulative-viewshed studies (why speed matters; 2022–2026)
Application papers, not algorithm papers. Useful in your introduction: each ran cumulative viewsheds over a whole region or country, which is the workload a fast CUDA total viewshed would serve.

- 🆕 **2026** · Vujačić, Vujović & Šiljeg · *Multi-resolution Copernicus DSM-based cumulative viewshed assessment of antenna towers for wildfire monitoring in Montenegro* · Open Geosciences
  [DOI](https://doi.org/10.1515/geo-2025-1019) (open access) · `cumulative` · national scale, multi-resolution DSM.
- 🆕 **2025** · Moser & Grêt-Regamey · *Visual exposure to buildings in Switzerland: spatial patterns and changes over six decades* · Scientific Reports
  [DOI](https://doi.org/10.1038/s41598-025-18772-7) · [PDF](https://www.nature.com/articles/s41598-025-18772-7.pdf) · `cumulative` · Whole-country binary and cumulative viewsheds, 1960–2024.
- 🆕 **2024** · Guo, Wenz & Auffhammer · *The visual effect of wind turbines on property values is small and diminishing in space and time* · PNAS
  [DOI](https://doi.org/10.1073/pnas.2309372121) · [PDF](https://www.pnas.org/doi/pdf/10.1073/pnas.2309372121) · `multi-observer, US-wide` · Visibility of every US utility-scale turbine; see also their reply on DEM vs DSM, [DOI](https://doi.org/10.1073/pnas.2409845121).
- 🆕 **2023** · Arnon, Uzan, Handel, Cain & Toledo · *ViewShedR: a new open-source tool for cumulative, subtractive and elevated line-of-sight analysis* · Royal Society Open Science
  [DOI](https://doi.org/10.1098/rsos.221333) (open access) · `cumulative` · `tool` · Free GUI for cumulative and subtractive viewsheds. A usability reference, not a speed one.
- 🆕 **2023** · Parent & Lei-Parent · *Rapid viewshed analyses: A case study with visibilities limited by trees and buildings* · Applied Geography
  [DOI](https://doi.org/10.1016/j.apgeog.2023.102942) · [Repository](https://digitalcommons.uri.edu/nrs_facpubs/674/) · `approx` · "2DView" skips the DSM and uses land cover plus building footprints to get viewsheds for more than 1000 waterbodies (6000 km²). An example of trading accuracy for scale.
- 🆕 **2022** · Quinn · *What can we see from the road? Applications of a cumulative viewshed analysis on a US state highway network* · Geographica Helvetica
  [DOI](https://doi.org/10.5194/gh-77-165-2022) (open access) · `cumulative` · Observers along a whole state highway network.

---

## 2. Single viewshed: GPU

- ★ **2025** · Gao, Lü, Zhang, Gan & Rong · *Machine-learning-enhanced parallelization of the R3 viewshed algorithm on graphic processing units* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2025.2578421) · `single` · `GPU (consumer laptop)` · `exact (R3)` · Ring-tessellation decomposition plus ML load balancing; 87.47× peak, bringing R3 close to R2/XDraw speed.
- **2025** · Schwencke et al. · *AI-Accelerated Viewshed Computation for High-Resolution Elevation Models* · VISIGRAPP 2025
  [DOI](https://doi.org/10.5220/0013168400003912) · [PDF](https://www.scitepress.org/Papers/2025/131684/131684.pdf) · `single` · `CPU + neural net (not parallel)` · `approx` · Predicts where each ray stops being visible so it can end early: over 99% accuracy with more than 90% fewer computations. The idea could carry over to CUDA ray kernels. Cites Tabik 2012/2014 and Sánchez-Fernández 2020.
- **2025** · Wróżyński et al. · *Application of 3D Ray Tracing for Water Surface Visibility Analysis* · ISPRS IJGI
  [DOI](https://doi.org/10.3390/ijgi14090335) · [PDF](https://www.mdpi.com/2220-9964/14/9/335/pdf) · `single` · `3D renderer ray tracing` · Companion paper: *Reaching beyond GIS for comprehensive 3D visibility analysis* (2024, Landscape & Urban Planning) [DOI](https://doi.org/10.1016/j.landurbplan.2024.105074).
- **2024** · Willis et al. · *GPU-Accelerated 3D Polygon Visibility Volumes for Synergistic Perception and Navigation* · IEEE SoutheastCon
  [DOI](https://doi.org/10.1109/southeastcon52093.2024.10500285) · [PDF (arXiv)](https://arxiv.org/pdf/2402.03135) · `inverse (region → 3D volume)` · `GPU depth maps` · Tangential (UAV planning).
- **2024** · Xiao, Deng, Wen & Gu · *Parallel algorithm for multi-viewpoint viewshed analysis on the GPU grounded in target cluster segmentation* · Int. J. Digital Earth
  [DOI](https://doi.org/10.1080/17538947.2024.2308707) · [PDF](https://www.tandfonline.com/doi/pdf/10.1080/17538947.2024.2308707?download=true) · `multi-viewpoint (150–600)` · `GPU render pipeline (shadow map)`
- **2023** · Kimm et al. · *Extending Visuospatial Analysis in Design Computing: An Exploration With a Novel GPU-Based Algorithm and Form-Based Codes* · CAADRIA 2023
  [DOI](https://doi.org/10.52842/conf.caadria.2023.1.655) · [PDF](http://papers.cumincad.org/data/works/att/caadria2023_340.pdf) · `multi-observer (urban 3D)` · `GPU shader (Unity)` · Tangential (urban design, not DEMs).
- **2022** · Saxena & Kaur · *Comparison of R2 and R3 Approaches over CPU vs. GPU Implementation* · INDIACom 2022
  [DOI](https://doi.org/10.23919/indiacom54597.2022.9763106) · `single` · `GPU (NVIDIA)` · Speedups of 5.06× (R2) and 15.08× (R3).
- **2021** · Chen & Chen · *A parallel multipoint viewshed analysis method for urban 3D building scenes* · Transactions in GIS
  [DOI](https://doi.org/10.1111/tgis.12759) · `multi-observer (cumulative index)` · `GPU depth`
- **2020** · Huang, Zhu & Li · *A novel approach for radar network's detection power analysis based on GPU* · IEEE CBD 2020
  [DOI](https://doi.org/10.1109/CBD51900.2020.00015) · `LOS coverage` · `GPU` · Minor.
- ★ **2020** · Qarah · *Efficient Viewshed Computation Algorithms On GPUs and CPUs* · PhD dissertation, Univ. of South Florida
  [Link](https://digitalcommons.usf.edu/etd/8480) · [PDF](https://digitalcommons.usf.edu/context/etd/article/9677/viewcontent/etd__8480.pdf) · `single` · `GPU (CUDA)` · `exact radial sweep` · 4.4× faster than the fastest prior exact GPU algorithm.
- ★ **2019** · Qarah & Tu · *A Fast Exact Viewshed Algorithm on GPU* · IEEE BigData 2019
  [DOI](https://doi.org/10.1109/bigdata47090.2019.9006353) · `single` · `GPU` · `exact radial sweep` · 10801² grid in 1.7 s; 3.5× faster than GPU-R3 and 6200× faster than sequential R3.
- **2017** · Yılmaz · *Accelerating Line of Sight Analysis Algorithms with Parallel Programming* · MSc thesis, METU
  [PDF](https://etd.lib.metu.edu.tr/upload/12620986/index.pdf) · `single` · `GPU (CUDA)` · Van Kreveld (partly parallel) 1.5×, R2 10.5×, R3 160×. A useful comparison of all three algorithm families on one GPU.
- **2016** · Heilmair · *GPU-Based Visualisation of Viewshed from Roads or Areas in a 3D Environment* · MSc thesis, Linköping
  [Link](http://urn.kb.se/resolve?urn=urn:nbn:se:liu:diva-130699) · [PDF](https://liu.diva-portal.org/smash/get/diva2:954165/FULLTEXT01.pdf) · `multi-observer` · `GPU (OpenGL pipeline)`
- ★ **2015** · Cauchi-Saunders & Lewis · *GPU enabled XDraw viewshed analysis* · J. Parallel and Distributed Computing
  [DOI](https://doi.org/10.1016/j.jpdc.2015.07.001) · [PDF](https://figshare.com/articles/journal_contribution/GPU_enabled_XDraw_viewshed_analysis/22927073) · `single` · `GPU` · `approx (XDraw)` · Beats CPU and GPU versions of R3, R2 and XDraw.
- **2015** · Axell & Fridén · *Comparison between GPU and parallel CPU optimizations in viewshed analysis* · MSc thesis, Chalmers
  [PDF](http://publications.lib.chalmers.se/records/fulltext/219740/219740.pdf) · `single` · `GPU vs CPU (threads + SIMD)` · GPU about 3× faster than the parallel CPU version.
- **2014** · Wang, Wang, Pan, Liu, Yang & Wang · *A parallel algorithm for viewshed analysis in three-dimensional Digital Earth* · Computers & Geosciences
  [DOI](https://doi.org/10.1016/j.cageo.2014.10.012) · [PDF](https://www.sci.utah.edu/~feng/assets/papers/viewshedAnalysis.pdf) · `single` · `parallel (platform: check PDF)`
- ★ **2014** · Osterman, Benedičič & Ritoša · *An IO-efficient parallel implementation of an R2 viewshed algorithm for large terrain maps on a CUDA GPU* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2014.918319) · [PDF](https://www.tandfonline.com/doi/pdf/10.1080/13658816.2014.918319?needAccess=true&role=button) · `single` · `GPU (CUDA)` · `approx (R2)` · Uses coalesced memory access; the implementation is `r.cuda.visibility` (GRASS).
- **2014** · Stojanović & Stojanović · *High Performance Processing and Analysis of Geospatial Data Using CUDA on GPU* · Adv. Electrical & Computer Eng.
  [DOI](https://doi.org/10.4316/aece.2014.04017) (open access) · `single` · `GPU (CUDA)`
- **2013** · Stojanović & Stojanović · *Performance improvement of viewshed analysis using GPU* · TELSIKS 2013
  [DOI](https://doi.org/10.1109/telsks.2013.6704407) · `single` · `GPU (CUDA)`
- ★ **2012** · Zhao, Padmanabhan & Wang · *A parallel computing approach to viewshed analysis of large terrain data using graphics processing units* · IJGIS 27(2)
  [DOI](https://doi.org/10.1080/13658816.2012.692372) · `single` · `GPU + CPU I/O` · Two-level domain decomposition. **The most-cited GPU viewshed paper (100×).**
- **2012** · Osterman · *Implementation of the r.cuda.los module in the open source GRASS GIS by using parallel computation on the NVIDIA CUDA graphic cards* · Elektrotehniški vestnik
  [PDF](https://ev.fe.uni-lj.si/1-2-2012/Osterman.pdf) · `single` · `GPU (CUDA)` · Predecessor of Osterman 2014; a 28161×17921 DEM in 18 s.
- **2012** · Liu et al. · *Research on GPU-Based Computation Method for Line-of-Sight Queries* · PADS 2012
  [DOI](https://doi.org/10.1109/pads.2012.37) · `batched LOS queries` · `GPU` · "Combine-and-partition" batching of many queries into one kernel launch.
- **2012** · Zhu et al. · *Accelerating Line-of-Sight computation with structure guided hybrid approach* · IEEE ESPA 2012
  [DOI](https://doi.org/10.1109/ESPA.2012.6152456) · `batched LOS` · `GPU culling + CPU exact` · About 50% faster.
- **2012** · Zhu X.-D. · *GPU-accelerated viewshed analysis in the fire fighting high altitude lookout technology* · Fire Science and Technology (in Chinese)
  [OpenAlex](https://openalex.org/W2377252431) · `single` · `GPU` · Little detail available.
- **2011** · Strnad · *Parallel terrain visibility calculation on the graphics processing unit* · Concurrency & Computation: P&E
  [DOI](https://doi.org/10.1002/cpe.1808) · [PDF](https://dk.um.si/IzpisGradiva.php?id=26932) · `multi-viewpoint` · `GPU vs multithreaded CPU` · Two levels of parallelism (rays within a viewpoint, and across viewpoints); about 10× faster.
- **2011** · Fang, Yang, Chen, Yao & Guo · *Parallel algorithm for viewshed analysis on a modern GPU* · Int. J. Digital Earth
  [DOI](https://doi.org/10.1080/17538947.2011.555565) · [PDF](https://www.tandfonline.com/doi/pdf/10.1080/17538947.2011.555565?needAccess=true) · `single` · `GPU shaders (DirectX 9)` · Shadow-map style, real-time.
- **2011** · Xia, Kuang & Li · *Accelerating geospatial analysis on GPUs using CUDA* · J. Zhejiang Univ.-Sci. C
  [DOI](https://doi.org/10.1631/jzus.c1100051) · `single` · `GPU (CUDA)` · 28–925× speedup for viewshed.
- **2011** · Gao, Yu, Liu, Liu, Liu & Zhao · *Optimization for viewshed analysis on GPU* · Geoinformatics 2011
  [DOI](https://doi.org/10.1109/geoinformatics.2011.5980830) · `single` · `GPU (CUDA)` · `exact (interpolation / R3-like)` · About 70× faster.
- **2010** · Xia, Yang & Shi · *Parallel Viewshed Analysis on GPU Using CUDA* · CSO 2010
  [DOI](https://doi.org/10.1109/cso.2010.12) · `single` · `GPU (CUDA)` · Parallel matrix traversal with sequential ray traversal (PMSR) works best.
- **2004** · Salomon, Govindaraju, Sud, et al. · *Accelerating Line of Sight Computation Using Graphics Processing Units* · Army Science Conf./DTIC
  [PDF](https://apps.dtic.mil/sti/pdfs/ADA433414.pdf) · `LOS queries` · `GPU + CPU hybrid` · The earliest GPU line-of-sight work. Follow-up: Verdesca et al. 2006, *Using Graphics Processor Units to Accelerate OneSAF* (J. Defense Modeling & Simulation) [DOI](https://doi.org/10.1177/154851290600300305).


### GPU hardware techniques worth borrowing (not about terrain)
- 🆕 **2026** · Meneses, Navarro, Ferrada & Verichev · *Ray Tracing Cores for General-Purpose Computing: A Literature Review* · arXiv
  [PDF (arXiv)](https://arxiv.org/pdf/2603.28771) · `technique` · `GPU RT cores` · When RT cores beat regular CUDA cores on non-graphics problems. Read before deciding whether RT cores could do the line-of-sight tests.
- 🆕 **2023** · Meneses, Navarro, Ferrada & Quezada · *Accelerating Range Minimum Queries with Ray Tracing Cores* (RTXRMQ) · arXiv
  [PDF (arXiv)](https://arxiv.org/pdf/2306.03282) · `technique` · `GPU RT cores` · Turns range-minimum queries into ray–triangle hits. Total viewshed's inner loop is a running maximum of slopes along each line, so this is the closest RT-core result.

---

## 3. Single viewshed: other parallel (multicore, SIMD, cluster, Spark)

- 🆕 **2026** · Zhang, Du, Feng et al. · *SIA-LOS: A Spatial-Index Accelerated Line-of-Sight Simulation System for Large-Scale Urban Air Mobility* · ISAES 2026
  [DOI](https://doi.org/10.1109/isaes70982.2026.11669289) · `massive LOS (3D urban)` · platform: check PDF.
- 🆕 **2026** · Jiao, Yang, Jia et al. · *VRPF: A Fine-Grained 3D Radar Power-Density Computation Framework Based on Photogrammetric City Models for Urban Observation* · Remote Sensing 18
  [DOI](https://doi.org/10.3390/rs18121936) (open access) · `radar LOS over 3D meshes` · AABB pruning then ray–triangle tests over a reusable spatial index; platform: check PDF.
- 🆕 **2025** · Gao, Lu, Gan, Zhang, Rong & Tang (高凡, 路威, 甘麟露, 章繁, 荣凤娟, 汤士涵) · *An intelligent parallel geocomputation engine framework* (智能驱动的并行地理计算引擎框架) · 测绘学报 / Acta Geodaetica et Cartographica Sinica 54(10):1877–1892 · **in Chinese**
  [Journal page](http://xb.chinasmp.com/CN/10.11947/j.AGCS.2025.20250127) (the printed DOI 10.11947/j.AGCS.2025.20250127 does not resolve yet) · `single (case study)` · `parallel, ML-driven scheduling` · ML/DL models predict each sub-domain's compute load; biggest-tasks-first plus work stealing; tested on polygon intersection and **viewshed**; more than 20× better load balance. Chinese-language companion to LPGE 2025 and ML-R3 2025 (same group).
- 🆕 **2025** · Laboy, Olaya, Marquez et al. · *Advancing the GEOtiled Framework Through Scalable Terrain Parameter Computation* · HPDC '25 (also Roa et al., *GEOtiled*, HPDC '23, [DOI](https://doi.org/10.1145/3588195.3595941))
  [DOI](https://doi.org/10.1145/3731545.3735117) · `terrain parameters (not viewshed)` · `HPC tiling` · Tiles the whole continental US for slope/aspect/hillshade; 6× faster than GDAL. A reference for tiling and mosaicking large DEMs.
- **2025** · Widłak et al. · *Ray Tracing and Bound Block Elimination Methods for Viewshed Analysis with Efficiency Enhancements Through Radial Segmentation and Parallel Processing* · ECMS 2025
  [DOI](https://doi.org/10.7148/2025-0631) · [PDF](http://www.scs-europe.net/dlib/2025/ecms2025acceptedpapers/0631_3dmod_ecms2025_0029.pdf) · `single (LiDAR point clouds)` · `CPU parallel` · Radial sectors: going from 100 to 200/300 sectors gives 179%/405% speed-up.
- **2025** · Gao et al. · *A learning-based parallel geocomputation engine for heterogeneous spatial domain (LPGE)* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2025.2528964) · [Code](https://doi.org/10.6084/m9.figshare.28121900) · `single-viewshed workloads` · `parallel CPU + ML load balancing` · 20× better load balance; same group as the 2025 GPU R3 paper.
- **2025** · Zhang, Ma, Li, Yang, et al. · *Efficient and fine-grained viewshed analysis in a three-dimensional urban complex environment* (Q-view) · Environmental Modelling & Software
  [DOI](https://doi.org/10.1016/j.envsoft.2025.106359) · [Code](https://github.com/Zoey-Fan/Q-view) · `single (3D mesh)` · `multicore CPU (3D R-tree + parallel LOS queries)`
- 🆕 **2024** · Wróżyński, Pyszny & Wróżyńska · *Reaching beyond GIS for comprehensive 3D visibility analysis* · Landscape & Urban Planning 247
  [DOI](https://doi.org/10.1016/j.landurbplan.2024.105074) · `3D single/multi` · `3D graphics software (Blender) + LiDAR` · Earlier work by the group behind Wróżyński 2025 (§2).
- 🆕 **2023** · Yu, Zhou & Li · *A parallel strategy to accelerate neighborhood operation for raster data coordinating CPU and GPU* · Cartography & GIS
  [DOI](https://doi.org/10.1080/15230406.2023.2272660) · `raster ops (not viewshed)` · `CPU+GPU together` · Asynchronous CPU+GPU task queue; 29.6× on a 1.7 GB raster. A template for a hybrid CPU+GPU total viewshed.
- **2023** · Dong & Zhang · *A Multi-Level Distributed Computing Approach to XDraw Viewshed Analysis Using Apache Spark* · Remote Sensing
  [DOI](https://doi.org/10.3390/rs15030761) (open access) · `single` · `Spark cluster`
- 🆕 **2022** · Brown, Czapiga, Jotshi et al. · *Planning Wireless Backhaul Links by Testing Line of Sight and Fresnel Zone Clearance* · ACM Trans. Spatial Algorithms & Systems
  [DOI](https://doi.org/10.1145/3517382) · `batched point-to-point LOS` · Elevation model of the whole USA; thousands of tower pairs in minutes.
- **2021** · Zhang, Zhao & Ye · *Spark-Enabled XDraw Viewshed Analysis* · IEEE JSTARS
  [DOI](https://doi.org/10.1109/jstars.2021.3051210) · [PDF](https://ieeexplore.ieee.org/ielx7/4609443/9314330/09320526.pdf) · `single` · `Spark cluster`
- **2021** · Orlof et al. · *TIN Surface and Radial Viewshed Determination Algorithm Parallelisation on Multiple Computing Machines* · Symmetry
  [DOI](https://doi.org/10.3390/sym13030424) · [PDF](https://www.mdpi.com/2073-8994/13/3/424/pdf) · `single (TIN)` · `distributed CPU`
- **2019** · Zhang, Chen, Liang, Xia & Zhou · *A Parallel Implementation of an XDraw Viewshed Algorithm with Spark* · IEEE HPCC 2019
  [DOI](https://doi.org/10.1109/hpcc/smartcity/dss.2019.00018) · `single` · `Spark`
- **2019** · Dou, Li & Wang · *An equal-area triangulated partition method for parallel Xdraw viewshed analysis* · CCPE
  [DOI](https://doi.org/10.1002/cpe.5216) · `single` · `cluster`
- **2018** · Dou, Li & Wang · *A fine-granularity scheduling algorithm for parallel XDraw viewshed analysis* · Earth Science Informatics
  [DOI](https://doi.org/10.1007/s12145-018-0339-5) · `single` · `cluster`
- **2018** · Dou & Li · *A fault-tolerant computing method for Xdraw parallel algorithm* · J. Supercomputing
  [DOI](https://doi.org/10.1007/s11227-018-2321-x) · `single` · `cluster`
- **2017** · Li Y. et al. · *Design and Implementation of Parallel XDraw Algorithm Based on Triangle Region Division* · DCABES 2017
  [DOI](https://doi.org/10.1109/dcabes.2017.16) · `single` · `parallel CPU` · Precursor to Dou et al.'s triangulated partitioning.
- **2016** · Song, Tang, Liu, Dou & Li · *Parallel viewshed analysis on a PC cluster system using triple-based irregular partition scheme* · Earth Science Informatics
  [DOI](https://doi.org/10.1007/s12145-016-0263-5) · `single` · `PC cluster`
- **2016** · Johansson & Lundberg · *Distributed Viewshed Analysis: An Evaluation of Distribution Frameworks for Geospatial Information Systems* · MSc thesis, Chalmers
  [PDF](https://odr.chalmers.se/bitstream/20.500.12380/238036/1/238036.pdf) · `single` · `Spark vs multicore` · Spark was only worthwhile for R3 (1.6×). **A useful negative result.**
- **2016** · Ferreira, Andrade, Magalhães & Franklin · *An Efficient External Memory Algorithm for Terrain Viewshed Computation* (TiledVS) · ACM TSAS
  [DOI](https://doi.org/10.1145/2903206) · `single` · `CPU, external memory` · A 104,000² terrain with 512 MB of RAM in 1.5 h; source code available.
- **2015** · Carabaño, Sarjakoski & Westerholm · *Efficient Implementation of a Fast Viewshed Algorithm on SIMD Architectures* · Euromicro PDP 2015
  [DOI](https://doi.org/10.1109/pdp.2015.62) · [PDF](http://hdl.handle.net/10138/224749) · `single` · `CPU SIMD (AVX2, quad-core)` · About 10⁹ points/s with XDraw.
- **2014** · Ferreira, Andrade, Magalhães, Franklin & Pena · *A Parallel Algorithm for Viewshed Computation on Grid Terrains* · J. Information and Data Management 5(2)
  [Link](https://wrfranklin.org/nikola/pubdetails/chaulio-jidm-2014.html) · [dblp](https://dblp.uni-trier.de/rec/html/journals/jidm/FerreiraAMFP14) · `single` · `CPU OpenMP` · 12× on 16 threads (5000² DEM).
- **2013** · Ferreira, Andrade, Magalhães, Franklin & Pena · *A Parallel Sweep Line Algorithm for Visibility Computation* · GeoInfo 2013
  [PDF](https://wrfranklin.org/p/174-geoinfo2013-chaulio-sweep.pdf) · `single` · `CPU OpenMP` · Parallel van Kreveld sweep.
- **2011** · Lu et al. (IBM) · *High performance computing of line of sight viewshed* · IEEE ICME 2011
  [DOI](https://doi.org/10.1109/icme.2011.6011875) · `single` · `Cell Broadband Engine (SIMD accelerator)` · 4.19M points in under 25.5 ms.
- **1998** · Ware, Kidner & Rallings · *Parallel distributed viewshed analysis* · ACM GIS '98
  [DOI](https://doi.org/10.1145/288692.288719) · `single/multi` · `distributed workstations`
- **1994** · De Floriani, Montani & Scopigno · *Parallelizing visibility computations on triangulated terrains* · IJGIS
  [DOI](https://doi.org/10.1080/02693799408902019) · `single (TIN)` · `parallel`
- **1993** · Teng, DeMenthon & Davis · *Region-to-region visibility analysis using data parallel machines* · Concurrency: Practice & Experience
  [DOI](https://doi.org/10.1002/cpe.4330050502) · `region-to-region` · `SIMD data-parallel`

---

## 4. Multi-observer & observer siting (the bridge between single and total)

- 🆕 **2026** · Pan, Li, Hu, Wang & Xu · *Surveillance layout optimization for improved 3D intervisible regional coverage* · Information Sciences
  [DOI](https://doi.org/10.1016/j.ins.2025.122848) · `multi-observer (3D)`
- 🆕 **2026** · Ma, Hu, Xue, Guo & Zhao · *An efficient viewshed evaluation method for distributed probing nodes based on the visibility consistency principle* · IoTCEAI 2026 (SPIE)
  [DOI](https://doi.org/10.1117/12.3122702) · `multi-observer`
- 🆕 **2026** · Yuan, Zou, Li et al. · *Edge server placement problem based on communication viewshed* · Computer Communications
  [DOI](https://doi.org/10.1016/j.comcom.2026.108456) · `multi-observer (siting)`
- 🆕 **2024** · Tang, Yan, Dai et al. · *An efficient and precise multi-candidate viewpoint filtering algorithm for terrain viewshed selection* (EPVPS) · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2024.2408279) · `multi-observer` · Beats CVF and simulated annealing on speed and coverage. Cites five of the GPU/total-viewshed papers in this list.
- 🆕 **2024** · Wang, Ma & Li · *Multi-observation points setting problem based on stepwise maximum viewshed approach* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2024.2354822) · `multi-observer` · Picks observers by repeatedly taking the maximum viewshed, so it needs every candidate's viewshed size (a total-viewshed input).
- 🆕 **2024** · Zhou, Wang, Huang et al. · *Sparse Multi-viewshed Analysis for Assisting Video Surveillance Network Location Optimization* · CCDC 2024
  [DOI](https://doi.org/10.1109/ccdc62350.2024.10587971) · `multi-observer`
- 🆕 **2022** · Salehi, Halabian, Ahmadi Beni et al. · *A fast serial computation approach for computing viewshed of a region by detecting key spots on the terrain* (RVAP) · Geocarto International
  [DOI](https://doi.org/10.1080/10106049.2022.2129841) · `regional (union of many observers)` · `serial` · Finds the few "key" pixels that decide a region's combined viewshed; same coverage as running every pixel, much faster (SRTM 90 m, ~555 × 555 km).
- **2022** · Gu, Budavári, Galante & Burns · *Towards Optimal Line of Sight Coverage* · IEEE eScience 2022
  [DOI](https://doi.org/10.1109/eScience55777.2022.00028) · [JHU repository](https://jscholarship.library.jhu.edu/items/7b878cdb-1ca5-4344-ad16-a414daddf9a4) · `multi-observer` · `parallel (platform not stated)` · Coverage-quality objective plus sensor-placement optimization.
- **2021** · Wang, He & Dou · *Parallelization implementation of topographic viewpoint filtering algorithm based on terrain viewshed using MPI and OpenMP* · DCABES 2021
  [DOI](https://doi.org/10.1109/dcabes52998.2021.00037) · `multi-observer` · `MPI + OpenMP`
- **2020** · Franklin, Magalhães & Li · *Siting thousands of radio transmitter towers on terrains with billions of points* · arXiv
  [PDF (arXiv)](https://arxiv.org/pdf/2006.16783) · `multi-observer` · `CPU OpenMP` · Terrains of up to 2 billion points.
- **2020** · Zheng et al. · *Parallel Landscape Visibility Analysis: A Case Study in Archaeology* · in *High Performance Computing for Geospatial Applications* (Springer)
  [DOI](https://doi.org/10.1007/978-3-030-47998-5_5) · `multi-observer (batch)` · `HPC cluster`
- ★ **2017** · Li, Franklin, Magalhães & Andrade · *GPU-Accelerated Multiple Observer Siting* · Photogrammetric Eng. & Remote Sensing
  [DOI](https://doi.org/10.14358/pers.83.6.439) · `multi-observer (≈15k observers)` · `GPU (CUDA) + OpenMP (16 cores)` · Step 1 computes a visibility index for **every cell**, which is a sampled total viewshed. CUDA is 70× faster than sequential (17 s on a 16,385² grid); OpenMP is 17×.
- **2017** · Lai, Huang & Shi · *Accelerating the calculation of minimum set of viewpoints for maximum coverage over DEM data by hybrid computer architecture and systems* · SIGSPATIAL Special
  [DOI](https://doi.org/10.1145/3100243.3100246) · `multi-observer` · `Xeon Phi (MIC) cluster` · Over 100×.
- **2017** · Wang, Tang, Fan, Mao, Yang & Zhu · *Efficient visibility analysis for massive observers* · Procedia Computer Science
  [DOI](https://doi.org/10.1016/j.procs.2017.06.018) · [PDF](https://www.sciencedirect.com/science/article/pii/S1877050917311912/pdf) · `millions of observers` · `MapReduce`
- **2016** · Shi et al. · *Deriving a minimum set of viewpoints for maximum coverage over any given digital elevation model data* · Int. J. Digital Earth
  [DOI](https://doi.org/10.1080/17538947.2016.1207718) · [PDF](https://www.tandfonline.com/doi/pdf/10.1080/17538947.2016.1207718) · `multi-observer` · `MPI cluster`
- **2016** · Li W. · *GPU-accelerated terrain processing* · PhD thesis, Rensselaer Polytechnic Institute
  [PDF](https://wrfranklin.org/theses/wenli-phd.pdf) · `multi-observer` · `GPU (CUDA)` · Thesis version of the 2014/2017 siting work.
- **2014** · Li, Franklin, Benedetti & Magalhães · *Parallel multiple observer siting on terrain* · ACM SIGSPATIAL '14
  [DOI](https://doi.org/10.1145/2666310.2666486) · [PDF](https://wrfranklin.org/p/182-acmgis2014-par-siting.pdf) · `multi-observer` · `OpenMP vs CUDA` · 16385² terrain: CUDA about 30 s vs OpenMP under 2 min.
- **2014** · Benedičič et al. · *A GRASS GIS parallel module for radio-propagation predictions* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2013.879151) · [PDF (arXiv)](https://arxiv.org/pdf/1402.4010) · `multi-transmitter LoS coverage` · `MPI cluster`
- **2014** · Pena et al. · *An Improved Parallel Algorithm Using GPU for Siting Observers on Terrain* · ICEIS 2014
  [DOI](https://doi.org/10.5220/0004884303670375) · `multi-observer` · `GPU` · More than 1200× faster than sequential.
- **2014** · Pena et al. · *An efficient GPU multiple-observer siting method based on sparse-matrix multiplication* · BigSpatial @ SIGSPATIAL 2014
  [DOI](https://doi.org/10.1145/2676536.2676544) · `multi-observer` · `GPU`
- **2010** · Coll et al. · *Good-visibility maps visualization* · The Visual Computer
  [DOI](https://doi.org/10.1007/s00371-009-0380-y) · `multi-viewpoint` · `GPU graphics pipeline`
- **2006** · Coll et al. · *Computing Terrain Multi-visibility Maps for a Set of View Segments Using Graphics Hardware* · ICCSA 2006 (LNCS)
  [DOI](https://doi.org/10.1007/11751540_10) · `multi (view segments)` · `GPU graphics pipeline`

---

## 5. Background: surveys & the sequential baselines you'll compare against

- 🆕 **2026** · Wang, Wang, Zhao, Luo et al. · *MRVd: a viewshed method for coupling multi-resolution terrain data* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2026.2624786) · `single` · `CPU` · High-resolution DEM near the observer, coarse far away; 2–3× better accuracy for little extra time. Not yet on a GPU.
- 🆕 **2026** · Wang, Zhu & Dou · *Structured representation and compression method of viewshed: An example analysis of communication viewshed* · J. Geographical Sciences 36:1299–1320
  [DOI](https://doi.org/10.1007/s11442-026-2493-x) · `output representation / compression` · Splits a viewshed into subregions described by seven parameters. Same Nanjing Normal group as Cheng & Dou (§1); relevant to niche 3 (output size).
- 🆕 **2026** · Aleksiejūnas & Stankevičius · *Application of XDraw Algorithm to Diffraction Modeling Using 3D Vector Building Data* · Transactions in GIS
  [DOI](https://doi.org/10.1111/tgis.70253) · `single (radio)` · XDraw ring-by-ring on 3D vector buildings (Amsterdam 3D BAG).
- 🆕 **2025** · Leenders, van Oijen, Lindelauf & Čule · *Uncertainty in visibility: a scoping review of the probable and fuzzy viewshed for observer location optimization* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2025.2581833) · [PDF](https://www.tandfonline.com/doi/pdf/10.1080/13658816.2025.2581833) · Recent review of non-binary viewsheds; supports an "accuracy, not just speed" angle.
- 🆕 **2025** · Argudo, Guérin, Schott & Galin · *Terrain descriptors for landscape synthesis, analysis and simulation* · Computer Graphics Forum
  [DOI](https://doi.org/10.1111/cgf.70080) (open access) · Implements and times many terrain metrics, including visibility-type ones; the graphics-side view.
- 🆕 **2024** · Tang, Yan, Dai et al. · *Dynamic viewshed analysis based on the digital elevation model: dynamic range proximity-direction-elevation reference line* (DPDERL) · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2024.2408283) · `single` · `approx` · Less than half of XDraw's time, within 5% of R3. Newest member of the PDERL family.
- 🆕 **2024** · Wang & Dou · *Accurate calculation method of terrain viewshed for wireless signal as line of sight* · Transactions in GIS
  [DOI](https://doi.org/10.1111/tgis.13188) · [PDF](https://onlinelibrary.wiley.com/doi/pdfdirect/10.1111/tgis.13188) · `single (Fresnel-zone LOS)`
- 🆕 **2024** · Orlof, Ozimek, Łabędź, Widłak & Ozimek · *Generating viewsheds based on the Digital Surface Model (DSM) and point cloud* · PLoS ONE
  [DOI](https://doi.org/10.1371/journal.pone.0312146) · [PDF](https://journals.plos.org/plosone/article/file?id=10.1371/journal.pone.0312146&type=printable) · `single` · `CPU only (i9-14900K, 64 GB)` · Point cloud 98 min vs DSM 9 min; the point-cloud viewsheds differ by 1.4–5.9%.
- **2024** · Zhao et al. · *Overview and Prospects of Visibility Analysis Approaches* · Proceedings (MDPI) 110(1)
  [DOI](https://doi.org/10.3390/proceedings2024110028) · [PDF](https://www.mdpi.com/2504-3900/110/1/28/pdf?version=1734774980) · Recent survey.
- 🆕 **2023** · Wang, Niu, Liu, Luo, Yu et al. · *A line-of-sight zoning method for intervisibility computation by considering terrain relief* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2023.2254825) · `LOS` · `approx` · Only samples the "potentially obscuring zones" along each line of sight.
- 🆕 **2023** · Wu, Wang, Gan, Zou, Dong et al. · *A Survey of the Landscape Visibility Analysis Tools and Technical Improvements* · IJERPH 20(3)
  [DOI](https://doi.org/10.3390/ijerph20031788) · [PDF](https://www.mdpi.com/1660-4601/20/3/1788/pdf) · Recent tools survey.
- 🆕 **2022** · Wang, Xiong, Guo, Zhang & Tang · *A view-tree method to compute viewsheds from digital elevation models* · IJGIS
  [DOI](https://doi.org/10.1080/13658816.2022.2094385) · `single` · 40% faster than R2; about 60% fewer errors than XDraw.
- 🆕 **2022** · Guan, Wu, Xia, Chen & Li · *Fast approximate viewshed analysis based on the regular-grid digital elevation model: X-type partition proximity-direction-elevation spatial reference line algorithm* (XPDERL) · Computers & Geosciences
  [DOI](https://doi.org/10.1016/j.cageo.2022.105213) · `single` · `approx` · PDERL family.
- 🆕 **2022** · Kang · *Intervisibility, Line-of-Sight, and Viewsheds* · GIS&T Body of Knowledge
  [DOI](https://doi.org/10.22224/gistbok/2022.2.8) (open access) · Short, current textbook-style introduction.
- **2022** · Inglis, Vukomanovic, Costanza & Singh · *From viewsheds to viewscapes: Trends in landscape visibility and visual quality research* · Landscape & Urban Planning
  [DOI](https://doi.org/10.1016/j.landurbplan.2022.104424) · Application-side review (motivation for why total viewshed matters).
- **2022** · Cheng & Dou · *HiPDERL: An Improved Implementation of the PDERL Viewshed Algorithm and Accuracy Analysis* · IJGI
  [DOI](https://doi.org/10.3390/ijgi11100506) (open access) · `exact-accuracy baseline`
- **2020** · Wu et al. · *PDERL: an accurate and fast algorithm with a novel perspective on solving the old viewshed analysis problem* · Earth Science Informatics
  [DOI](https://doi.org/10.1007/s12145-020-00545-7)
- **2019** · Zhu et al. · *HiXDraw: An Improved XDraw Algorithm Free of Chunk Distortion* · IJGI
  [DOI](https://doi.org/10.3390/ijgi8030153) (open access)
- **2018** · Haverkort & Toma · *A Comparison of I/O-Efficient Algorithms for Visibility Computation on Massive Grid Terrains* · arXiv
  [PDF (arXiv)](https://arxiv.org/pdf/1810.01946) · Good taxonomy of radial, centrifugal and horizon-based sweeps.
- **2009** · Fishman, Haverkort & Toma · *Improved visibility computation on massive grid terrains* · ACM SIGSPATIAL '09
  [DOI](https://doi.org/10.1145/1653771.1653791)
- **2009** · Haverkort, Toma & Zhuang · *Computing visibility on terrains in external memory* · ACM J. Experimental Algorithmics
  [DOI](https://doi.org/10.1145/1412228.1412233)
- **2009** · Andrade, Magalhães, Magalhães, Franklin & Cutler · *Efficient viewshed computation on terrain in external memory* · GeoInformatica
  [DOI](https://doi.org/10.1007/s10707-009-0100-9)
- **2003** · De Floriani & Magillo · *Algorithms for visibility computation on terrains: a survey* · Environment & Planning B
  [DOI](https://doi.org/10.1068/b12979) · The classic survey (cited 120×).
- **2003** · Izraelevitz · *A Fast Algorithm for Approximate Viewshed Computation* · PE&RS 69(7)
  [DOI](https://doi.org/10.14358/pers.69.7.767)
- **2003** · Llobera · *Extending GIS-based visual analysis: the concept of visualscapes* · IJGIS
  [DOI](https://doi.org/10.1080/713811741)
- **2002** · Kaučič & Žalik · *Comparison of viewshed algorithms on regular spaced points* · SCCG '02
  [DOI](https://doi.org/10.1145/584458.584487)
- **2000** · Wang, Robinson & White · *Generating viewsheds without using sightlines* · PE&RS 66(1):87–90 (no DOI)
  [ResearchGate](https://www.researchgate.net/publication/265205505_Generating_Viewsheds_without_Using_Sightlines) · The reference-plane algorithm GDAL's `gdal_viewshed` uses.
- **1996** · van Kreveld · *Variations on sweep algorithms: efficient computation of extended viewsheds and class intervals* · SDH '96, pp. 15–27
  [CORE](https://core.ac.uk/outputs/39715044/) · The O(n log n) radial sweep behind most exact GPU work (Qarah, Ferreira).
- **1995** · Wheatley · *Cumulative viewshed analysis: a GIS-based method for investigating intervisibility, and its archaeological application* · in *Archaeology and GIS* (Lock & Stančič, eds.)
  [DOI (2022 reissue)](https://doi.org/10.1201/9780367810467-13) · The origin of cumulative viewsheds.
- **1994** · Franklin & Ray · *Higher isn't necessarily better: visibility algorithms and experiments* · SDH '94 (defines **R3, R2, XDraw**)
  [PDF](https://wrfranklin.org/p/84-edinburgh-sdh94-higher-not-better.pdf)

---

## 6. Code & datasets

| What | Link | Notes |
|---|---|---|
**Total viewshed: research code (baselines to benchmark against)**

| What | Link | Notes |
|---|---|---|
| sDEM (Sánchez-Fernández 2020) | https://doi.org/10.6084/m9.figshare.11370549 | Figshare tarball, CC BY 4.0; covers single-core, multicore, single-GPU and multi-GPU. **The main CUDA baseline** |
| SkewEngine (Romero 2024) | https://github.com/luisfromero/skewEngine | CUDA/C++ (includes `kernelSDEM.h`) plus Jupyter notebooks; last active 2023 |
| Tabik's total viewshed (2014 algorithm) | https://github.com/SihamTabik/Total_Viewshed | C++/OpenMP; total, total-3D, single and horizon modes; 2017 |
| AllTheLines / CacheTVS | https://github.com/AllTheLines/CacheTVS | Rust, GPL-3.0. Tabik 2014 with rotation plus prefix-max scan; CPU SIMD; GPU kernel removed, CUDA "TBC"; active 2026 |
| AllTheLines / viewview | https://github.com/AllTheLines/viewview | Planet-scale pipeline around CacheTVS |
| Write-ups | [tombh.co.uk/longest-line-of-sight](https://tombh.co.uk/longest-line-of-sight) · [ryan.berge.rs total-viewshed algorithm](https://ryan.berge.rs/posts/total-viewshed-algorithm/) · [alltheviews.world](https://alltheviews.world) | Blog posts explaining the algorithm and scale |
| wenlili / siting | https://github.com/wenlili/siting | CUDA + OpenMP visibility index + siting (Li/Franklin 2014–2017); 2016 |

**Total viewshed: student/course projects (useful to see what's been tried; not peer-reviewed)**

| What | Link | Notes |
|---|---|---|
| JeremyCodes20 / total-viewshed-computation | https://github.com/JeremyCodes20/total-viewshed-computation | C++ with OpenMP, MPI and **CUDA**; 2022 |
| Bosser16 / awanna-cu | https://github.com/Bosser16/awanna-cu | **CUDA**, CUDA+MPI, OpenMP, MPI; visible count per cell; 2025 |
| annadeckmyn / total_viewshed_qgis_plugin | https://github.com/annadeckmyn/total_viewshed_qgis_plugin | Python QGIS plugin with optional CuPy GPU; 2026 |
| adtodesco / totalViewshed | https://github.com/adtodesco/totalViewshed | C/MPI, Bowdoin course (L. Toma); 2015 |
| AbstractBeliefs / hons | https://github.com/AbstractBeliefs/hons | C/OpenCL single viewshed; Napier **honours project**, 2017 |

**Single viewshed: GPU code**

| What | Link | Notes |
|---|---|---|
| Gao 2025 ML-R3 | https://doi.org/10.6084/m9.figshare.28910060 | CUDA (.cu) + Python ML. **The newest exact GPU single-viewshed code** |
| Gao 2025 LPGE | https://doi.org/10.6084/m9.figshare.28121900 | C++/Spark + Python |
| xarray-spatial (+ makepath/rtxpy) | https://github.com/xarray-contrib/xarray-spatial | Python/Numba; GPU via CuPy + **NVIDIA OptiX** ray tracing; active 2026 |
| mi-v / rfshed | https://github.com/mi-v/rfshed | CUDA + Go; 2021 |
| ayush5harma / ViewshedAnalysis | https://github.com/ayush5harma/ViewshedAnalysis | CUDA C++ student project; 2021 |

**Single viewshed & horizons: CPU code**

| What | Link | Notes |
|---|---|---|
| TiledVS (Ferreira 2016) | https://github.com/chaulio/TiledVS | C++ external memory |
| Q-view (Zhang 2025) | https://github.com/Zoey-Fan/Q-view | C++ multicore 3D |
| PDERL / DPDERL ports | https://github.com/blct-w/Pedrl-Algorithm · https://github.com/team-tgq/DPDERL_viewshed_algorithm | C# / Python; also R3 and XDraw reference versions |
| HORAYZON | https://github.com/ChristianSteger/HORAYZON | Embree ray tracing; horizons/SVF at all points |
| Dozier topographic horizons | https://github.com/DozierJeff/Topographic-Horizons | MATLAB |

No public code was found for Osterman's r.cuda.los, Qarah 2019/2020, Zhao 2013 or Afshani 2018. The Cauchi-Saunders figshare record has the article only.

**Standard tools & datasets**

| What | Link | Notes |
|---|---|---|
| Total Viewshed of Bohemia | https://zenodo.org/records/5764173 | Data from Kuna et al. 2022 |
| Cheng & Dou benchmark data | https://doi.org/10.21227/e8hd-2q44 | Runtimes for TVS, sDEM, BOSBASIC, RIP |
| GDAL `gdal_viewshed` | https://gdal.org/en/stable/programs/gdal_viewshed.html | Standard CPU single-viewshed baseline; a cumulative (multi-observer) mode was added in Sept 2024 (PR #10674) |
| WhiteboxTools `VisibilityIndex` | https://www.whiteboxgeo.com/manual/wbt_book/available_tools/geomorphometric_analysis.html#VisibilityIndex | Sampled total viewshed on CPU threads (`res_factor`), a common GIS baseline |
| GRASS `r.viewshed` / `r.horizon` / `r.viewshed.exposure` | https://grass.osgeo.org/grass-stable/manuals/r.viewshed.html | Single (Haverkort–Toma sweep) / all-points horizons (OpenMP) / cumulative exposure (Cimburová 2022) |

**Most-cited parallel/GPU viewshed papers** (Semantic Scholar, Sep 2026; OpenAlex in brackets)

| # | Paper | Citations |
|---|---|---|
| 1 | Zhao, Padmanabhan & Wang 2012, IJGIS (GPU) | 102 [100] |
| 2 | Tabik, Zapata & Romero 2012, IJGIS (total viewshed) | 59 [61] |
| 3 | Kidner, Rallings & Ware 1997, GeoInformatica (parallel total visibility) | 48 [37] |
| 4 | Osterman et al. 2014, IJGIS (CUDA R2) | 44 [39] |
| 5 | Fang et al. 2011, IJDE (GPU shaders) | 40 [42] |
| 6 | Wang F. et al. 2014, Computers & Geosciences | 39 [43] |
| 7 | Llobera et al. 2004, CAA (high-throughput total viewshed) | 38 [38] |
| 8 | Xia, Kuang & Li 2011, JZUS-C (CUDA) | 36 [33] |
| 9 | Tabik et al. 2014, JSTARS (total viewshed) | 27 [22] |
| 10 | Cauchi-Saunders & Lewis 2015, JPDC (GPU XDraw) | 22 [24] |

For context: Sánchez-Fernández 2020 has 11 [14], Qarah & Tu 2019 has 6 [5], and Afshani 2018 has 3 [2]. The newest work hasn't built up citations yet.

---

## 7. Possible thesis niches (from this search; verify as you read)

1. **No modern, peer-reviewed CUDA total viewshed exists.** The peer-reviewed GPU total-viewshed work is essentially sDEM (2020; code on figshare). CacheTVS (2026) abandoned its GPU kernel in favour of AVX-512 CPUs. A few CUDA course projects exist on GitHub (§6) but are unpublished and unbenchmarked. Nobody has compared TVS, sDEM (skew), CacheTVS (rotation) and a current-GPU CUDA kernel head-to-head. A per-sector CUDA kernel using warp-level prefix-max scans is a well-defined contribution; Timonen 2012 is a useful algorithmic template.
2. **Accuracy is becoming the second axis.** HiPDERL (2022) and Cheng & Dou's "Beyond Efficiency" (under review) point to reviewers expecting accuracy measured against an exact reference (R3 / HiPDERL), not just speedups.
3. **Output size is a bottleneck.** Most tools return only the viewshed *size* per cell. A CUDA kernel that also emits compressed per-point viewsheds (see Lukač & Žalik 2026, Chen et al. 2025, Wang, Zhu & Dou 2026, CacheTVS "ring data") is largely unexplored.
4. **Exact, all-cells runs at very large scale** have only been done on CPUs (CacheTVS; Kuna 2022 sampled every 4th cell). A multi-GPU version has not been published.
5. **Techniques from related GPU problems** (sky-view-factor and horizon ray tracing, ML early termination) haven't been applied to exact total viewshed.
6. **Recent CPU algorithms have no GPU versions** *(added in the 2022–2026 pass)*. The 2022–2026 IJGIS / C&G algorithms (view-tree, XPDERL, DPDERL, line-of-sight zoning, MRVd) are all CPU-only in this search. MRVd's idea (full resolution near the observer, coarse far away) could cut total-viewshed work a lot, and could be measured with the accuracy metrics from niche 2.
7. **RT cores are untried for terrain viewshed.** This search found no paper using NVIDIA RT cores (OptiX) for DEM viewsheds, although the RT-core literature (Meneses et al. 2023, 2026) covers similar max/min queries. A comparison of CUDA cores vs RT cores for the line-of-sight step would be new.
8. **A ready-made use case:** 2024 observer-siting papers (EPVPS, SMV) need every candidate cell's viewshed size, which is exactly total-viewshed output. A fast CUDA total viewshed could be demonstrated as the first stage of such a pipeline.
