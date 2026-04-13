# Good science - Run before Installing ROS2
To ensure good science, the prudent scientist should run the Phase 1 
test before installing ROS 2.


### Why Run Phase 1 Now?

1. **Isolation:** You want to measure the "raw" speed of your C++ bitmask 
logic without any background noise from ROS 2 daemons.

2. **Immediate Progress:** The ROS 2 installation is large (2GB+) and can 
take time. Phase 1 takes seconds to compile and run.

3. **Sanity Check:** If the microbenchmark fails (e.g., uses >10KB RAM), 
you know you need to fix the algorithm *before* you waste time wrapping 
it in a ROS node.

### How to Run Phase 1 

You only need `g++`, `cmake`, and `valgrind`. If you haven't installed 
the massive ROS bundle yet, just install these three small tools:

```bash
sudo apt update && sudo apt install -y build-essential cmake valgrind

```

Then, go into the benchmarks folder and run the test:

```bash
cd ~/projects/robo-ngac-hypergraph-experiment/benchmarks

# 1. Create a build directory (standard C++ practice)
mkdir -p build && cd build

# 2. Compile the code
cmake ..
make

# 3. Run the speed test
./ngac_benchmark

```

### What to Look For

You will see output like this. If the numbers look good, you can proceed to the ROS install.
Take this opportunity to write up the "Microbenchmark" pre-ROS results.

* **Average Latency:** Should be **< 1000 ns (1 µs)**.
* **Memory Footprint:** Should be **small** (the code prints the static size; we will use Valgrind next for the heap).

**Want to check the Heap memory (<10KB) next?**
Run this command in the same `build/` directory:

```bash
valgrind --tool=massif --massif-out-file=massif.out ./ngac_benchmark
ms_print massif.out | head -n 20

```
