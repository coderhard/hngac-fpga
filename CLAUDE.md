# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**hngac-fpga** implements a 4D state-aware H-NGAC (Hardware-accelerated Narrow Graph Authorization Constraint) authorization primitive targeting FPGA via Vitis HLS. It extends an IEEE DCAS 2026 software baseline by adding a **state** dimension (battery, maintenance, safety, calibration) on top of the classic subject/object/attribute model, then pushes the authorize primitive into hardware.

The core research question: can hardware-accelerated 4D authorization outperform traditional RBAC systems that require external state lookups?

## Build and Test

### Local C++ (no Vitis required)

```bash
cmake -S fpga/hls -B fpga/hls/build
cmake --build fpga/hls/build
ctest --test-dir fpga/hls/build --output-on-failure
```

Run a single test by name:
```bash
ctest --test-dir fpga/hls/build -R <test_name> --output-on-failure
```

### Benchmark (comparison runner)

```bash
# Args: <iterations> <rbac_state_lookup_delay_ns>
./fpga/hls/build/hngac_compare_benchmark 20000 100000

# Or via script (saves results to a directory)
./fpga/hls/scripts/run_local_compare.sh 20000 100000 /tmp/hngac-fpga-local-compare
```

The benchmark compares three approaches on the same request set: 3D baseline, 4D state-aware, and RBAC+state-lookup. Output includes mean, p99, and max latency plus allow/deny counts.

### Vitis HLS synthesis

```bash
export HNGAC_HLS_PART=<part-id>          # e.g. xcu250-figd2104-2L-e
export HNGAC_HLS_CLOCK_NS=5.0
export HNGAC_HLS_WORKDIR=/tmp/hngac-fpga-hls   # keep off repo tree
vitis_hls -f fpga/hls/scripts/vitis_hls.tcl
```

HLS work products go to `HNGAC_HLS_WORKDIR` (defaults to `/tmp/hngac-fpga-hls`) — never into the repo tree.

## Architecture

### Directory layout

| Path | Purpose |
|------|---------|
| `fpga/hls/src/` | HLS kernel (single translation unit) |
| `fpga/hls/include/` | Public headers — types, constants, bitmask helpers |
| `fpga/hls/tb/` | CTest testbench |
| `fpga/hls/bench/` | Local comparison benchmark |
| `fpga/hls/scripts/` | TCL (Vitis HLS) and Bash (local runner) scripts |
| `benchmarks/`, `ros2_ws/`, `analysis/`, `data/` | IEEE DCAS software baseline (preserved, not modified) |
| `docs/` | Implementation plan and decision log |

### Core kernel

`hngac_authorize(rules[], rule_count, request) -> bool` in `fpga/hls/src/hngac_kernel.cpp`

Algorithm: linear scan, first-match-wins, early exit. HLS pragmas target pipeline throughput.

### Key types (all in `hngac::fpga` namespace)

- **`Bitmask256`** — 4×64-bit words; supports up to 256 nodes (subjects/objects). Helpers: `set_bit()`, `test_bit()`, `contains_all()`.
- **`StateMask`** — 32-bit bitmask for 4 named state flags: `battery_low` (0), `maintenance_mode` (1), `safety_interlock` (2), `calibration_required` (3).
- **`PolicyRule`** — subject/object/attribute bitmasks + required `StateMask` + reserved provenance field.
- **`AuthorizationRequest`** — subject ID, object ID, attribute mask, current object `StateMask`, reserved provenance.

### 4D matching logic

A rule permits a request when all four conditions hold:
1. Subject bit set in rule's subject bitmask
2. Object bit set in rule's object bitmask
3. All requested attribute bits present in rule's attribute bitmask (`contains_all`)
4. All required state bits present in request's state (`contains_all_states`)

### HLS constraints (never violate)

- No `std::vector`, no dynamic allocation, no heap use in kernel path
- No ROS2 headers in `fpga/hls/`
- Max 256 nodes (`kMaxNodes`), max 512 policy rules (`kMaxRules`)
- C++17 only; HLS-friendly subset

## Key Documentation

- `docs/implementation-plan-4d-hngac.md` — full phased roadmap
- `docs/decision-log.md` — architecture decision record (Decision / Reason / Alternatives / Impact format)

Record new architectural decisions in `docs/decision-log.md` before implementing. The 5D provenance dimension is reserved (field exists in structs) but not yet implemented.
