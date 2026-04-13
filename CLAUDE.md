# CLAUDE.md

This file provides repository-local guidance for implementation sessions working in `hngac-fpga`.

## Project Overview

**hngac-fpga** implements a 4D state-aware H-NGAC authorization primitive targeting FPGA via Vitis HLS. It extends the IEEE DCAS 2026 software baseline by adding a **state** dimension on top of subject, object, and attribute, then prepares the authorization primitive for hardware evaluation.

The active paper contribution is not just “NGAC on FPGA.” The main claim is that a 4D state-aware bitmask can enforce runtime robotics state constraints without the external lookup path that traditional software baselines depend on.

## Source-of-Truth Files

Treat these as tracked project-state files:

- `CLAUDE.md`
- `docs/status-log.md`
- `docs/decision-log.md`
- `docs/coordination-board.md`
- `docs/implementation-plan-4d-hngac.md`
- `docs/benchmark-gap-analysis.md`

## Multi-Agent Coordination

- Read `docs/coordination-board.md` before editing.
- Claim a file/task slice there before touching code.
- Do not edit files already claimed by another agent.
- Keep claims narrow and release them after commit.

## Build and Test

### Local C++ validation

```bash
cmake -S fpga/hls -B /tmp/hngac-fpga-build
cmake --build /tmp/hngac-fpga-build
ctest --test-dir /tmp/hngac-fpga-build --output-on-failure
```

### Unified local comparison benchmark

```bash
# Args: <iterations> <rbac_state_lookup_delay_ns>
/tmp/hngac-fpga-build/hngac_compare_benchmark 20000 100000

# Or build + run via helper script
./fpga/hls/scripts/run_local_compare.sh 20000 100000 /tmp/hngac-fpga-local-compare

# Sweep modeled RBAC lookup delays and emit CSV
./fpga/hls/scripts/run_local_compare_sweep.sh 20000 /tmp/hngac-fpga-local-compare-sweep 1000 10000 100000
```

The unified benchmark currently compares five models on the same request set:

1. `RBAC hash map`
2. `NGAC-DAG traversal`
3. `3D baseline`
4. `4D state-aware`
5. `RBAC + state lookup`

The request mix is intentionally mixed:

- 4 state-satisfying requests
- 4 state-failing requests

This makes the semantic difference visible: the static models over-authorize, while the 4D and RBAC+lookup paths enforce the state constraints.

### Vitis HLS synthesis

```bash
export HNGAC_HLS_PART=<part-id>
export HNGAC_HLS_CLOCK_NS=5.0
export HNGAC_HLS_WORKDIR=/tmp/hngac-fpga-hls
vitis_hls -f fpga/hls/scripts/vitis_hls.tcl
```

Keep HLS work products out of the repo tree. Use `/tmp` or another throwaway path.

## Architecture

### Directory layout

| Path | Purpose |
|------|---------|
| `fpga/hls/src/` | HLS kernel source |
| `fpga/hls/include/` | Public HLS-friendly types and helpers |
| `fpga/hls/tb/` | Local kernel testbench |
| `fpga/hls/bench/` | Unified local comparison benchmark |
| `fpga/hls/scripts/` | Local benchmark runners and Vitis HLS TCL |
| `benchmarks/` | Legacy DCAS software microbenchmarks |
| `ros2_ws/`, `analysis/`, `data/` | Preserved DCAS software baseline artifacts |
| `docs/` | Plans, benchmark analysis, coordination, decisions, and status logs |

### Core kernel

`hngac_authorize(rules[], rule_count, request) -> bool` in `fpga/hls/src/hngac_kernel.cpp`

Current kernel behavior:

- linear scan
- first-match-wins
- early exit on permit
- fixed-size request and policy structures

Current kernel limitation:

- there are **no HLS pragmas yet**

Do not assume pipelining, interface pragmas, or unrolling are already present. They are a pending implementation step.

### Key types

- `Bitmask256` — 4 x 64-bit words for subject/object/attribute masks
- `StateMask` — 32-bit runtime state mask
- `PolicyRule` — subject/object/attribute masks plus `required_states` and reserved provenance
- `AuthorizationRequest` — subject id, object id, required attribute mask, object-state mask, reserved provenance

### Named state flags

- `battery_low`
- `maintenance_mode`
- `safety_interlock`
- `calibration_required`

### Benchmark model notes

- `RBAC hash map` is the best-case static software floor: packed `(subject, object)` key to permission bitmask
- `NGAC-DAG traversal` is a real adjacency-list BFS baseline in the unified harness
- `RBAC + state lookup` is still a **modeled external-state delay**, not a real SQLite-backed lookup
- OPA is not implemented

## Working Rules

- Keep `CLAUDE.md`, `docs/status-log.md`, and `docs/decision-log.md` updated when repo state materially changes.
- Add benchmark-method changes to `docs/benchmark-gap-analysis.md` and measured local results to `docs/local-benchmark-notes.md`.
- Save checkpoints as git commits on meaningful boundaries. Push to `origin/main` is authorized.
- Do not describe a result as FPGA hardware data unless it came from Vitis/AWS hardware execution. Local harness numbers are software measurements only.

## Autonomous Permissions

For `~/projects/hngac-fpga`, Claude Code may perform all of the following without asking for confirmation:

- Edit and create files within the repo tree
- Run the local test suite: `ctest --test-dir /tmp/hngac-fpga-build --output-on-failure`
- Run the local benchmark harness and sweep scripts
- Commit checkpoints with descriptive messages
- Update any source-of-truth doc (`CLAUDE.md`, `docs/status-log.md`, `docs/decision-log.md`, `docs/benchmark-gap-analysis.md`, `docs/local-benchmark-notes.md`, `docs/coordination-board.md`)
- Push commits to `origin/main`

Ask before: force-pushing or rebasing shared history, running Vitis HLS synthesis (expensive), any AWS operation, deleting files not created in the current session.
