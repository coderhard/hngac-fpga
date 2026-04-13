# benchmarks/ — Legacy DCAS Baseline

These files are from the original IEEE DCAS 2026 software baseline. They are **preserved for reference** but are not the canonical harness for the SOCC 2026 paper.

## What is here

| File | What it does |
|---|---|
| `ngac_benchmark.cpp` | Aggregate-latency microbenchmark for the 3D NGAC bitmask engine. 100k iterations, single start/end timer, stdout only. |
| `ngac_jitter.cpp` | Per-call latency distribution (min / avg / p95 / p99 / max) for the same 3D engine. |
| `CMakeLists.txt` | Builds both binaries with `-O3`. No external dependencies. |
| `run_memory_profile.sh` | Shell wrapper for `valgrind --tool=massif` memory profiling. |

## Why these are not the SOCC harness

- Both use `std::bitset<128>` + `std::vector<Hyperedge>` — a different 3D implementation than the kernel types (`Bitmask256`, fixed arrays) in `fpga/hls/`.
- Neither measures RBAC, NGAC-DAG, or the 4D state-aware path.
- No CLI arguments — policy size, iteration count, and request pattern are hardcoded.

## Canonical SOCC harness

Use `fpga/hls/bench/hngac_compare_benchmark.cpp`, which:
- Uses the HLS-kernel-aligned `Bitmask256` types throughout
- Benchmarks all five models on the same mixed request set
- Takes `<iterations> <rbac_lookup_delay_ns>` arguments
- Optionally links SQLite for a real external-state lookup baseline
- Has a sweep script with CSV output

```bash
cmake -S fpga/hls -B /tmp/hngac-fpga-build
cmake --build /tmp/hngac-fpga-build
/tmp/hngac-fpga-build/hngac_compare_benchmark 20000 100000
```
