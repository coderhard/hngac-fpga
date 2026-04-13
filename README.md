# hngac-fpga

4D state-aware H-NGAC authorization primitive targeting FPGA via Vitis HLS.

This is the IEEE SOCC 2026 working repo. The core claim: a hardware-accelerated 4D bitmask can enforce runtime object-state constraints (battery, maintenance, safety, calibration) without the external lookup overhead that traditional RBAC deployments require. The repo implements that claim, measures it locally against four software baselines, and prepares the kernel for hardware synthesis.

---

## Quick start — local build and test

```bash
# Build everything (keep off the Windows-backed tree if on WSL)
cmake -S fpga/hls -B /tmp/hngac-fpga-build
cmake --build /tmp/hngac-fpga-build

# Run the 34-case kernel testbench
ctest --test-dir /tmp/hngac-fpga-build --output-on-failure

# Run the five-model comparison benchmark
/tmp/hngac-fpga-build/hngac_compare_benchmark 20000 100000
```

---

## Five-model local comparison

The unified benchmark (`fpga/hls/bench/hngac_compare_benchmark.cpp`) measures five authorization models on the same mixed request set (half state-satisfying, half state-failing):

| Model | Implementation | State-aware? |
|---|---|---|
| RBAC hash map | `unordered_map<(subject,object), permission_mask>` | No — over-authorizes mixed set |
| NGAC-DAG traversal | adjacency-list BFS | No — over-authorizes mixed set |
| H-NGAC 3D bitmask | `Bitmask256` fixed-array scan | No — over-authorizes mixed set |
| **H-NGAC 4D state-aware** | 3D + `StateMask` containment check | **Yes** |
| RBAC + state lookup | role table + modeled/SQLite external-state query | Yes |

Run a sweep across RBAC lookup delays and get CSV output:

```bash
./fpga/hls/scripts/run_local_compare_sweep.sh 20000 /tmp/hngac-fpga-sweep 1000 10000 100000
```

The sweep script emits `sweep_summary.csv` — one row per delay value, all five models.

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
| `fpga/hls/tb/` | 34-case CTest testbench |
| `fpga/hls/bench/` | Five-model comparison benchmark |
| `fpga/hls/scripts/` | Vitis HLS TCL, local benchmark runners, sweep script |
| `docs/` | Decision log, implementation plan, benchmark gap analysis, status log, coordination board |
| `benchmarks/` | Legacy DCAS 3D bitmask microbenchmarks (preserved, not the canonical SOCC harness) |
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

## 4D matching semantics

A rule permits a request when all four conditions hold:

1. Subject bit set in rule's subject bitmask
2. Object bit set in rule's object bitmask
3. All requested attribute bits present in rule's attribute bitmask
4. All required state bits present in request's object-state mask

Named state bits: `battery_low` · `maintenance_mode` · `safety_interlock` · `calibration_required`

A rule with `required_states = 0` is a state wildcard (always passes condition 4).

---

## DCAS baseline context

The imported software baseline (`benchmarks/`, `ros2_ws/`, `data/`) is from the original IEEE DCAS 2026 submission. It is preserved for reference. The files in `benchmarks/` use an older `std::bitset<128>` implementation — they are not the canonical SOCC harness. See `benchmarks/README.md` for details.

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
