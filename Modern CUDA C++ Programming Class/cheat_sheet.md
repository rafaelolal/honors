# Section 01 Reference Sheet: CUDA Made Easy

### Compilation and execution spaces (01.02)
1. **`nvcc`**: the CUDA compiler. Common flags in the course: `-x cu` (treat the file as CUDA), `-arch=native` (build for the local GPU), `--extended-lambda` (allow `__device__` lambdas) and `-std=c++17`.
2. **Heterogeneous programming model**: a program always starts on the **host** (CPU). Nothing runs on the **device** (GPU) unless you ask for it explicitly.
3. **Execution space specifiers**:
   - `__host__` means the function runs on the CPU. Functions are `__host__` by default.
   - `__device__` means it runs on the GPU.
   - `__host__ __device__` means it can run on both.
4. **Device lambdas**: `[=] __host__ __device__ (float x) { ... }`. Capture by value with `[=]` or `[name]`.
5. **`thrust::universal_vector<T>`**: a container whose memory is visible to both the CPU and the GPU and migrates between them automatically.
6. **Execution policies**:
   - `thrust::device` runs the algorithm on the GPU.
   - `thrust::host` runs it on the CPU.

   Either one goes in as the first argument to a Thrust algorithm.
7. **`thrust::transform`**: the GPU version of `std::transform`. It has a unary form and a binary form that takes two input sequences.
8. **`thrust::for_each`**: applies a function to each element.
9. **`thrust::sort`**: moving from CPU to GPU is often just changing `std::` to `thrust::` and adding a policy.

### Extending algorithms with fancy iterators (01.03)
10. **`thrust::reduce`**: reduces a sequence to one value. It takes an optional initial value and operator, for example `thrust::maximum<float>{}`.
11. **Materialization overhead**: storing intermediate results in memory wastes bandwidth and space. These algorithms are **memory-bound**, so fewer memory accesses directly means more speed.
12. **Iterators as generalized pointers**: they are built on operator overloading of `*`, `++` and `[]`. The notebook builds hand-written counting, transform, zip, transform-output and discard iterators to show how they work.
13. **`thrust::make_zip_iterator(a.begin(), b.begin())`**: combines several sequences into one sequence of `thrust::tuple`s.
14. **`thrust::tuple` / `thrust::get<N>(t)`**: the tuple type that zip iterators produce, and how you read its elements.
15. **`thrust::make_transform_iterator(it, f)`**: applies `f` whenever the iterator is dereferenced, which avoids storing intermediate results. It can be combined with others, for example zip + transform to get `|a-b|` without a temporary array.
16. **Iterator arithmetic**: `transform + a.size()` gives you the end iterator.
17. **`thrust::sequence`**: fills a range with 0, 1, 2, …. The course also uses `rbegin()`/`rend()` with it.
18. **Timing with `std::chrono::high_resolution_clock`**: used to measure speedups.

### Vocabulary types (01.04)
19. **`thrust::make_counting_iterator(0)`**: an infinite sequence of indices that takes up no memory.
20. **`thrust::raw_pointer_cast(vec.data())`**: gets a raw pointer from a Thrust vector so a lambda can capture it.
21. **`thrust::tabulate(policy, begin, end, f)`**: stores `f(index)` at each index. It is equivalent to running `transform` over a counting iterator.
22. **Host-only `std::` functions can't be called on the device**: for example `std::make_pair` gives the error *"calling a `__host__` function from a `__host__ __device__` function"*.
23. **libcu++ (`cuda::std::`)**: device-compatible versions of standard types, such as `cuda::std::pair` and `cuda::std::make_pair`. It's a drop-in swap: put `cuda::` in front of `std::`.
24. **Structured bindings**: `auto [row, col] = row_col(id, width);`
25. **`cuda::std::mdspan`**: a multidimensional view over a flat buffer.
    - Construct it with `mdspan md(ptr, height, width)`.
    - Access elements with `md(row, col)`.
    - Get dimensions with `md.extent(0)`/`md.extent(1)` and the element count with `md.size()`.
    - This replaces manual index calculations like `row * width + col`.
26. **`cuda::std::array`**: the device-compatible version of `std::array`.

### Serial vs parallel (01.05)
27. **Code inside a device lambda runs serially**: a `for` loop in a `tabulate` operator is not parallelized for you. Keep the parallelism in the algorithm, not in the loop.
28. **Achieved throughput (GB/s)**: the performance metric for memory-bound code. Compare it with the GPU's peak bandwidth.
29. **Segmented problems**: for example a segmented sum or segmented mean, one result per row.
30. **`thrust::reduce_by_key`**: reduces each run of equal consecutive keys to one value. It takes input keys, input values, output keys and output values.
31. **`thrust::make_discard_iterator()`**: an output iterator that throws away whatever is written to it, so outputs you don't need (such as the keys) cost nothing.
32. **Counting + transform iterator for keys**: `make_transform_iterator(make_counting_iterator(0), i / width)` generates row IDs without storing them in memory.
33. **`thrust::make_transform_output_iterator(out.begin(), f)`**: applies `f` to each value as it is written. The course uses it to fuse the "sum → mean" step into `reduce_by_key`.
34. **Functors**: a `struct` with a `__host__ __device__ operator()` and state, for example `mean_functor{width}`.
35. **`thrust::fill`**: fills a range with a value.

### Memory spaces (01.06)
36. **Host memory vs device memory**: code can only use data that lives in a memory space it can reach.
37. **Implicit migration cost**: when the CPU touches a `universal_vector`, pages move back and forth between CPU and GPU. This caused the roughly 100× slowdown in the example.
38. **`thrust::host_vector<T>`**: explicit CPU memory.
39. **`thrust::device_vector<T>`**: explicit GPU memory.
40. **`thrust::copy` / `thrust::copy_n`**: copy data between memory spaces. Copying is one of the few algorithms that accepts mixed memory spaces.
41. **`vec.swap(other)`**: swaps buffers cheaply, used for the double-buffered prev/next pattern.

### Advanced (01.08)
42. **You can't pass a `__device__` function by name to an algorithm called from the host**: taking the address of a device function in host code is not allowed, and at runtime it fails with an "invalid program counter" error. Wrap the call in a lambda or use a function object instead.
43. **A lambda is a function object**: it's an unnamed `struct` with an `operator()`. That's why a lambda works where a plain function pointer fails.

Items 1–43 come from the notebook text, the solution blocks and the full solution files under `Solutions/`. The helpers in `ach.h` (`ach::where_am_I`, `ach::init`, `ach::store`, `ach::simulate`) are course scaffolding, so I left them out.