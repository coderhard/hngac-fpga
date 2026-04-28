# Benchmark Gap Analysis — IEEE IPCCC 2026

**Date:** 2026-04-26
**Scope:** Audit of benchmark coverage for the current 5D/IPCCC comparison and the remaining build/runtime gaps.

---

## Current harness inventory

| File | What it measures | Models |
|---|---|---|
| `benchmarks/ngac_benchmark.cpp` | Aggregate latency (single start/end, 100k iters) | Legacy 3D NGAC bitmask only |
| `benchmarks/ngac_jitter.cpp` | Per-call latency distribution (min/avg/p95/p99/max) | Legacy 3D NGAC bitmask only |
| `fpga/hls/bench/hngac_compare_benchmark.cpp` | Unified mean/p95/p99/max/stddev benchmark with 1,000-iteration warmup plus build/reload/memory stats | RBAC hash map, NGAC-DAG traversal, H-NGAC 3D, H-NGAC 4D, H-NGAC 5D, flattened 5D direct lookup, RBAC+modeled-state-lookup, optional RBAC+SQLite-state-lookup |
| `fpga/hls/scripts/run_local_compare.sh` | Builds and runs the unified benchmark, saves log output | same benchmark binary |
| `fpga/hls/scripts/run_local_compare_sweep.sh` | Sweeps modeled lookup delay, saves logs + CSV | parsed benchmark summary and comparison metrics |
| `data/final_data.log` | ROS2 runtime gatekeeper log (DCAS baseline) | Not a benchmark — per-event ns timestamps only |

### Important implementation notes

- `benchmarks/ngac_benchmark.cpp` and `benchmarks/ngac_jitter.cpp` still use the older `std::bitset<128>` + `std::vector<Hyperedge>` implementation. They are useful legacy context, but they are **not** the canonical IPCCC harness.
- The canonical IPCCC harness is `fpga/hls/bench/hngac_compare_benchmark.cpp`, which uses the HLS-kernel-aligned `Bitmask256` and fixed-structure data types for the bitmask paths.
- `RBAC hash map` is implemented as a packed `(subject_id << 16) | object_id` key to permission bitmask.
- `NGAC-DAG traversal` is implemented as a real adjacency-list BFS baseline in the unified harness.
- `H-NGAC 5D` is the canonical semantics path for the current paper scope.
- `Flattened 5D direct lookup` is a materialized allow-set hash-table baseline validated against H-NGAC 5D on the generated request corpus. It is included to answer reviewer fairness questions about direct decision lookup latency versus compact bitmask representation.
- `RBAC + state lookup` is a **parameter-driven simulated delay**. It models external lookup cost; it does not measure a real database or RPC path.
- `RBAC + SQLite state lookup` is now available when `SQLite3` is found at configure time. It uses an in-memory SQLite state table and a prepared query in the unified harness.
- The unified harness uses `std::chrono::steady_clock` and performs a 1,000-iteration warmup per model before timing.
- HLS kernel (`fpga/hls/src/hngac_kernel.cpp`) now includes host-build-tolerant HLS `INTERFACE` and `PIPELINE II=1` pragmas.
- OPA appears nowhere in the codebase.

---

## Required comparison paths — current state

### Model 1: RBAC — in-memory hash map, no external state

- **Implemented empirically:** Yes
- **Implementation:** `std::unordered_map<uint32_t, Bitmask256>` keyed by packed `(subject, object)` with permission bitmask values
- **Current limitation:** Static only; ignores runtime state and therefore over-authorizes the mixed scenario set by design
- **External dependencies:** None

### Model 2: RBAC + external state — configurable lookup delay or real SQLite call

- **Implemented empirically:** Yes
- **Implementation:** both modeled external lookup delay and optional SQLite-backed in-memory state lookup exist in the unified harness
- **Gap:** The SQLite path is a local in-process external-state baseline, not a remote service or networked store. The paper still needs to frame it correctly.
- **External dependencies:** `sqlite3` for the empirical SQLite path

### Model 3: NGAC-DAG — adjacency-list graph traversal

- **Implemented empirically:** Yes
- **Implementation:** real adjacency-list BFS baseline in the unified harness
- **Current limitation:** This path is a static graph baseline and does not encode the 4D state dimension
- **External dependencies:** None

### Model 4: H-NGAC 3D bitmask — HLS-kernel-typed baseline

- **Implemented empirically:** Yes
- **Implementation:** fixed-structure 3D bitmask path in the unified harness
- **Residual issue:** The repo still contains two divergent 3D implementations. The unified harness version is the one that should anchor the paper.
- **External dependencies:** None

### Model 5: H-NGAC 4D state-aware bitmask

- **Implemented empirically:** Yes
- **Implementation:** active kernel path plus unified benchmark path
- **Gap:** None for local software comparison. Hardware synthesis/runtime data is still pending.
- **External dependencies:** None

### Model 6: H-NGAC 5D provenance-aware bitmask

- **Implemented empirically:** Yes
- **Implementation:** active kernel path plus unified benchmark path
- **Current role:** canonical authorization semantics for the IPCCC paper
- **Gap:** Hardware synthesis and co-simulation evidence are still pending the HW team.
- **External dependencies:** None

### Model 7: Flattened 5D direct lookup

- **Implemented empirically:** Yes
- **Implementation:** materialized 5D allow-set in a flat open-addressing hash table
- **Current role:** reviewer-fair direct lookup baseline for the same 5D decision relation
- **Gap:** Scaling sweep still needed to characterize memory, reload/build cost, and latency as policy size grows.
- **External dependencies:** None

---

## OPA — feasibility

OPA is still not present. Two integration approaches remain possible:

| Approach | Dependencies | Latency range | Recommendation |
|---|---|---|---|
| Subprocess (`popen` / `fork+exec opa eval`) | external `opa` binary | ms-range | separate chart only |
| HTTP client to `opa run --server` | `libcurl` | network-round-trip range | separate chart only |

OPA is still a stretch goal. It should not be placed on the same ns-scale chart as the in-process baselines.

---

## Summary table

| Model | Implemented? | Gap severity | Notes |
|---|---|---|---|
| RBAC (hash map, no state) | Yes | Low | best-case static software floor |
| RBAC + external state (real) | Yes | Low | SQLite-backed local empirical path now available |
| NGAC-DAG (adjacency list, BFS) | Yes | Low | static graph baseline only |
| H-NGAC 3D bitmask | Yes | Low | canonical path is unified harness, not legacy `benchmarks/` |
| H-NGAC 4D state-aware | Yes | Low | local comparison complete, hardware data pending |
| H-NGAC 5D provenance-aware | Yes | Low | canonical current paper semantics, hardware data pending |
| Flattened 5D direct lookup | Yes | Medium | local baseline exists; scaling/memory analysis needs fresh canonical run |
| OPA | No | Low | stretch goal only |

---

## Build system state

- `benchmarks/CMakeLists.txt`: CMake 3.5, C++17, `-O3 -Wall -Wextra`
- `fpga/hls/CMakeLists.txt`: CMake 3.16, C++17, `-O3 -Wall -Wextra -Wpedantic`, host-build suppression for HLS pragma warnings, optional `SQLite3` linkage for `hngac_compare_benchmark`
- No linked libraries in `benchmarks/`
- No LTO, no `-march=native`, no sanitizer flags

---

## What to build next (priority order)

1. **[DONE 2026-04-28]** Run canonical 8-model benchmark with flattened 5D baseline — 200k iterations, results in `data/benchmarks/benchmark_20260428_055248.log`.
2. **[DONE 2026-04-28]** Memory scaling calculations for 10/50/100/500 subject-object pairs — documented in `docs/local-benchmark-notes.md`. Memory figures are exact; latency requires runs.
3. **[NEXT] Run policy-size scaling latency sweep** — 200k iterations at rule_count = 10, 50, 100, 500. Binary already accepts `argv[3]` for rule_count. Save one log + meta per size. This populates the paper's scaling table (`paper/main.tex:601`) and resolves the `\todo{N}` fleet placeholder at line 515 (use 50 AGVs). Memory figures are already calculated; these runs add empirical latency and the cache-miss effect at 50+ rules.

   ```bash
   for N in 10 50 100 500; do
     /tmp/hngac-fpga-build/hngac_compare_benchmark 200000 100000 $N \
       | tee data/benchmarks/benchmark_scaling_${N}rules_$(date +%Y%m%d_%H%M%S).log
   done
   ```

4. **Capture lookup-delay sweep table from `run_local_compare_sweep.sh`** — modeled RBAC sensitivity analysis only; label clearly as modeled, not empirical.
5. **Report SQLite separately from modeled RBAC** — SQLite is empirical in-process; modeled lookup is sensitivity analysis.
6. **Vitis HLS synthesis reports** — pending HW-team (Badawy lab); leave paper placeholders until reports arrive.
7. **OPA** — stretch goal only; separate latency scale from in-process baselines.
