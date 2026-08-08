# CLAUDE.md

This file provides repository-local guidance for implementation sessions working in `hngac-fpga`.

## Project Overview

**hngac-fpga** implements a 5D provenance-aware H-NGAC authorization primitive targeting FPGA via Vitis HLS. It extends the IEEE DCAS 2026 software baseline by adding **state** and **provenance** dimensions on top of subject, object, and attribute, then prepares the authorization primitive for hardware evaluation.

**Terminology (author decision 2026-08-07).** The H in H-NGAC is **Hardware**, not Hypergraph. H-NGAC was presented at DCAS 2026; this paper **extends** it, so never write "we present H-NGAC." It is a *different system* from **HyperNGAC**, the hypergraph privilege analysis of BigData 2025. Never describe H-NGAC as compiling "hypergraphs" — it compiles NGAC policy **graphs**. Full table in `docs/canonical-context.md`.

The active paper contribution is not just “NGAC on FPGA,” and it is not hardware (DCAS's claim) or hypergraphs (BigData's claim). The main claim is that **security dimensionality** scales at zero time cost in hardware while blocking the three attack classes below.

**Attack classes (canonical names — integer numbering is retired).**

| Class | Closed by | CVE anchor |
|---|---|---|
| Unauthorized access | 3D | CVE-2022-45789 (Schneider Modicon session hijack) |
| Unsafe-state operation | 4D | CVE-2022-33323 (Mitsubishi MELFA unauthorized command) |
| Command provenance abuse | 5D | CVE-2021-38425 (eProsima Fast DDS RTPS injection) |

The **timing window is not an attack class** — it is a cross-cutting delivery property closed by the synthesis-time latency bound, not by any dimension. Measured slip rate is **zero**; never claim otherwise.

**This claim is now MEASURED (2026-08-05).** On Zynq-7020 at 100 MHz, the 4D and 5D kernels resolve in an identical number of clock cycles at every policy size, with identical II=1 and identical timing slack. The fifth dimension costs +11.4% LUT and zero extra cycles. Evidence lives in `hngac-package-from-farouq/`; authoritative numbers and wording rules are in `docs/canonical-context.md`.

Two wording rules that matter: say **“free in time, nearly free in area,”** not “zero hardware cost”; and do not claim 3D/4D/5D parity, because **3D was never synthesized**.

## Source-of-Truth Files

Treat these as tracked project-state files:

- `CLAUDE.md`
- `docs/status-log.md`
- `docs/decision-log.md`
- `docs/coordination-board.md`
- `docs/implementation-plan-4d-hngac.md`
- `docs/benchmark-gap-analysis.md`
- `docs/canonical-context.md`
- `docs/project-overview.md`

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

The unified benchmark currently compares seven always-on paths, plus an optional SQLite path when SQLite is available:

1. `RBAC hash map`
2. `NGAC-DAG traversal`
3. `H-NGAC 3D`
4. `H-NGAC 4D`
5. `H-NGAC 5D`
6. `Flattened 5D direct lookup`
7. `RBAC + state lookup`
8. `RBAC + SQLite state lookup` (optional)

The request mix is a generated 5D-aware corpus. The default four-rule corpus produces eleven atomic-action requests: one 5D-satisfying request per rule plus state-failing and provenance-failing requests where applicable.

This makes the semantic difference visible: the static models over-authorize, the 4D and RBAC+state paths enforce state, and the 5D paths enforce both state and provenance.

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
| `docs/figures/` | Generated figures: presentation charts (`analysis/make_hw_sw_charts.py`) and paper-figure PNG previews (`analysis/make_paper_figures.py`, whose print PDFs land in `paper/figures/`) |
| `analysis/` | Result plotting and stats scripts |
| `hngac-package-from-farouq/` | **HW evidence, immutable.** Synthesis, co-sim, board test, SW perf. See its `PROVENANCE.md`. Not working source — the live kernel is `fpga/hls/src/`. |

### Core kernel

`hngac_authorize(rules[], rule_count, request) -> bool` in `fpga/hls/src/hngac_kernel.cpp`

Current kernel behavior:

- linear scan
- first-match-wins
- early exit on permit
- fixed-size request and policy structures

Current kernel synthesis status:

- HLS `INTERFACE` pragmas are present for return, rule count, request, and policy memory.
- HLS `PIPELINE II=1` is present inside the rule scan.
- Vitis HLS 2025.2 synthesis and Verilog co-simulation: **done 2026-08-05** for 4D and 5D
  on `xc7z020-clg400-1` at 100 MHz. Both Pass. Reports in `hngac-package-from-farouq/results/`.
- Board verification: **done** — 2,307 requests PASS on PYNQ-Z1 silicon. Functional only,
  no on-board timing was taken. Never cite it as a timing result.
- The delivered `opt-v1` kernel is **optimized beyond the version in `fpga/hls/src/`**: it
  checks two rules per clock, giving 0.5 cycles per rule. Reconciling that optimization back
  into the repo kernel is open work.
- Still not done: 3D synthesis, and a fair embedded software baseline on the PYNQ-Z1's own
  ARM Cortex-A9.

### Key types

- `Bitmask256` — 4 x 64-bit words for subject/object/attribute masks
- `StateMask` — 32-bit runtime state mask
- `ProvenanceMask` — 32-bit command-source provenance mask
- `PolicyRule` — subject/object/attribute masks plus `required_states` and `required_provenance`
- `AuthorizationRequest` — subject id, object id, required attribute mask, object-state mask, source provenance

### Named state flags

- `battery_low`
- `maintenance_mode`
- `safety_interlock`
- `calibration_required`

### Named provenance flags

- `authenticated_ros2_node`
- `local_terminal`
- `remote_operator`

### Benchmark model notes

- `RBAC hash map` is the best-case static software floor: packed `(subject, object)` key to permission bitmask
- `NGAC-DAG traversal` is a real adjacency-list BFS baseline in the unified harness
- `Flattened 5D direct lookup` is a materialized allow-set baseline validated against H-NGAC 5D on the benchmark corpus
- `RBAC + state lookup` is a **modeled external-state delay**, not an empirical database result
- `RBAC + SQLite state lookup` is an optional empirical in-process SQLite path when SQLite3 is found at configure time
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
