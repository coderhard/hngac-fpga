# hngac-fpga H-NGAC IPCCC 2026

**PI:** Hassan Karim, Stable Cyber LLC
**Venue:** IEEE IPCCC 2026 (abstract 7 Aug 2026, manuscript the following week)
**Repo:** `https://github.com/coderhard/hngac-fpga`

5D provenance-aware H-NGAC authorization primitive targeting FPGA via Vitis HLS.

**Terminology.** H-NGAC is **Hardware-NGAC**, presented at DCAS 2026 and *extended*
here. It is a different system from **HyperNGAC**, the hypergraph privilege analysis
of BigData 2025, and it compiles NGAC policy **graphs**, not hypergraphs. See the
TERMINOLOGY table in `docs/canonical-context.md`.

This is the IEEE IPCCC 2026 working repo. The core claim is that security
dimensionality is **free in time and nearly free in area** in hardware. **Measured
2026-08-05 (4D, 5D) and 2026-08-13 (3D)** on Zynq-7020 (`xc7z020-clg400-1`) at 100 MHz:
the 3D, 4D and 5D kernels resolve in an identical number of clock cycles at every policy
size tested, with identical II=1 and identical timing slack, while carrying both added
dimensions costs +1,336 LUT (+35.5%). On the same corpus the 3D kernel permits all 2,307
requests that the 5D kernel narrows to 864 — identical cycles, different security. The repo
implements the kernel, measures it locally against software baselines, and provides
the synthesis flow; the hardware evidence lives in `hngac-package-from-farouq/` (v1) and
`hngac-package-v2-from-farouq/` (v2 delta: 3D synthesis and board timing).

---

## Quick start — local build and test

```bash
# Build everything (keep off the Windows-backed tree if on WSL)
cmake -S fpga/hls -B /tmp/hngac-fpga-build
cmake --build /tmp/hngac-fpga-build

# Run the 45-case kernel testbench
ctest --test-dir /tmp/hngac-fpga-build --output-on-failure

# Run the local comparison benchmark (canonical: 200k iterations, 1k warmup)
/tmp/hngac-fpga-build/hngac_compare_benchmark 200000 100000
```

---

## Local Comparison Benchmark

The unified benchmark (`fpga/hls/bench/hngac_compare_benchmark.cpp`) measures seven always-on paths, plus an eighth SQLite path when SQLite is available at configure time. All paths run on the same 5D-aware request corpus.

| Model | Implementation | Correct? |
|---|---|---|
| RBAC hash map | `unordered_map<(subject,object), permission_mask>` | No — over-authorizes |
| NGAC-DAG traversal | adjacency-list BFS | No — over-authorizes |
| H-NGAC 3D bitmask | `Bitmask256` fixed-array scan | No — over-authorizes |
| **H-NGAC 4D state-aware** | 3D + `StateMask` containment check | **Yes** |
| **H-NGAC 5D provenance-aware** | 4D + provenance bitmask check | **Yes** |
| Flattened 5D direct lookup | materialized allow-set hash table | Yes — reviewer-fair lookup baseline |
| RBAC + modeled state | role table + busy-wait delay | Yes — **NOT empirical** |
| RBAC + SQLite state | role table + in-process SQLite lookup | Yes — empirical, optional |

Canonical run: `hngac_compare_benchmark 200000 100000` (200k iterations, 1k warmup). The second argument is the modeled RBAC external-state lookup delay in nanoseconds and only affects `RBAC + modeled state`.

Run a sweep across RBAC lookup delays and get CSV output:

```bash
./fpga/hls/scripts/run_local_compare_sweep.sh 20000 /tmp/hngac-fpga-sweep 1000 10000 100000
```

The sweep script emits `sweep_summary.csv` — one row per delay value, all parsed benchmark paths.

---

## Vitis HLS synthesis

```bash
export HNGAC_HLS_PART=xcu250-figd2104-2L-e   # your target part
export HNGAC_HLS_CLOCK_NS=5.0                 # 200 MHz target
export HNGAC_HLS_WORKDIR=/tmp/hngac-fpga-hls  # keep off repo tree

# Optional: HNGAC_HLS_COSIM=1 to run co-simulation after synthesis
# Optional: HNGAC_HLS_EXPORT=1 to export IP catalog

vitis_hls -f fpga/hls/scripts/vitis_hls.tcl
```

Reports land in `$HNGAC_HLS_WORKDIR/hngac_authorize/sol1/syn/report/`.

---

## Repository layout

| Path | Purpose |
|---|---|
| `fpga/hls/src/` | HLS kernel — `hngac_authorize()` with INTERFACE and PIPELINE pragmas |
| `fpga/hls/include/` | `Bitmask256`, `StateMask`, `PolicyRule`, `AuthorizationRequest` types |
| `fpga/hls/tb/` | 45-case CTest testbench (3D/4D/5D correctness) |
| `fpga/hls/bench/` | Local comparison benchmark with H-NGAC, graph, RBAC, SQLite, and flattened lookup paths |
| `fpga/hls/scripts/` | Vitis HLS TCL, local benchmark runners, sweep script |
| `docs/` | Decision log, implementation plan, benchmark gap analysis, status log, coordination board |
| `benchmarks/` | Legacy DCAS 3D bitmask microbenchmarks (preserved, not the canonical IPCCC harness) |
| `ros2_ws/`, `analysis/`, `data/` | Preserved DCAS software baseline artifacts |

---

## Key docs

| File | Contents |
|---|---|
| `docs/implementation-plan-4d-hngac.md` | Full phased roadmap |
| `docs/decision-log.md` | Architecture decisions (Decision / Reason / Alternatives / Impact) |
| `docs/benchmark-gap-analysis.md` | Model-by-model audit of the comparison harness |
| `docs/local-benchmark-notes.md` | Logged smoke-run results |
| `docs/status-log.md` | Per-session agent activity log |
| `docs/coordination-board.md` | Multi-agent file ownership board |
| `CLAUDE.md` | Repo-local guidance for Claude Code sessions |

---

## 5D Matching Semantics

A rule permits a request when all five conditions hold:

1. Subject bit set in rule's subject bitmask
2. Object bit set in rule's object bitmask
3. All requested attribute bits present in rule's attribute bitmask
4. All required state bits present in request's object-state mask
5. Required provenance is either a wildcard or matches the request's source provenance

Named state bits: `battery_low` · `maintenance_mode` · `safety_interlock` · `calibration_required`

Named provenance bits: `authenticated_ros2_node` · `local_terminal` · `remote_operator`

A rule with `required_states = 0` is a state wildcard (always passes condition 4).

A rule with `required_provenance = 0` is a provenance wildcard. Otherwise, provenance passes when at least one required provenance bit is present in the request's `source_provenance` mask.

---

## DCAS baseline context

The imported software baseline (`benchmarks/`, `ros2_ws/`, `data/`) is from the original IEEE DCAS 2026 submission. It is preserved for reference. The files in `benchmarks/` use an older `std::bitset<128>` implementation — they are not the canonical IPCCC harness. See `benchmarks/README.md` for details.

Representative prior results from the DCAS ROS2 gatekeeper:

| Metric | Value |
|---|---|
| Average latency (ROS 2) | 1.05 μs |
| p99 latency | 6.37 μs |
| Worst-case (OS preemption) | 157 μs |
| Pure C++ microbenchmark | 38 ns |
| Memory footprint | < 10 KB |

---

## License

Licensed under the [Business Source License 1.1 (BUSL-1.1)](https://mariadb.com/bsl11/).

- **Licensor:** Stable Cyber LLC
- **Licensed Work:** Hardware-Accelerated NGAC Authorization for Real-Time Multi-Robot Systems
- **Additional Use Grant:** Academic research, education, and non-commercial experimentation permitted with citation.
- **Change Date:** 2031-04-30 → converts to MIT License
