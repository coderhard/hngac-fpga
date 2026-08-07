---
name: project-overview
description: Single-source-of-truth entry point for all collaborators — paper, repo, status, and canonical numbers
type: project
---

# H-NGAC IPCCC 2026 — Project Overview

**PI:** Hassan Karim, Stable Cyber LLC  
**Venue:** IEEE IPCCC 2026 (deadlines TBD — est. abstract June, final July 2026)  
**Repo:** `https://github.com/coderhard/hngac-fpga`  
**Paper draft:** `paper/main.tex` (gitignored — contact Hassan for current version)

---

## Core Claim

An FPGA-targeted H-NGAC bitmask authorization primitive delivers deterministic,
hardware-bounded WCET that software cannot guarantee regardless of optimization level.

**MEASURED 2026-08-05 on Zynq-7020 (xc7z020-clg400-1) at 100 MHz.** Adding the
provenance dimension costs **zero clock cycles** and +11.4% LUT. Latency is
closed-form (`cycles = 12 + rules/2`) with min = avg = max at every policy size.
In software the same dimension makes every policy rule 19% more expensive.

State the claim as **"free in time, nearly free in area,"** never "zero hardware
cost." Note that 3D was never synthesized, so the claim is scoped to 4D vs 5D.
Full numbers and honesty constraints: `docs/canonical-context.md`.

---

## Project Status (as of 2026-08-07)

| Item | Status | Notes |
|---|---|---|
| HLS kernel (3D/4D/5D) | **Done** | `fpga/hls/src/hngac_kernel.cpp` |
| Testbench | **Done** | 45 tests, 0 failed — `fpga/hls/tb/` |
| Local benchmark harness | **Done** | seven always-on paths plus optional SQLite; flattened 5D baseline added locally |
| April 18 canonical benchmark run | **Done** | 200k iterations, 1k warmup — see below |
| Attack Class 2 ROS2 demo | **Done** | 18,878 injections blocked, 100%, 0 FP |
| Attack Class 1 timing data | **Done** | 0 slips under WSL2 load |
| Paper skeleton | **~70% written** | All sections drafted; hardware tables can now be filled from real data |
| HLS synthesis reports (4D + 5D) | **Done 2026-08-05** | `hngac-package-from-farouq/results/cosim-opt-v1-{4d,5d}/syn_report/` |
| RTL co-simulation (4D + 5D) | **Done 2026-08-05** | Verilog Pass; per-call cycles in `cosim_report/verilog/result.transaction.rpt` |
| Board verification (5D) | **Done** | 2,307 requests PASS on PYNQ-Z1 silicon — functional only, no timing |
| Vivado place-and-route | **Partial** | Earlier 4D run only: WNS +2.170 ns, 0 failing endpoints |
| 3D synthesis | **Not done** | Blocks the original three-way equal-LUT-stage claim; one cheap csynth run |
| Fair embedded SW baseline | **Not done** | Run the benchmark on the PYNQ-Z1's own ARM Cortex-A9 |
| IPCCC abstract | **Due 2026-08-07** | |
| IPCCC manuscript | **Due week of 2026-08-14** | |

---

## Canonical Benchmark Numbers (April 18, 2026)

Platform: WSL2 x86-64, GCC -O3, 200k iterations, 1k warmup per model.  
Request mix: generated 5D-aware corpus. The April 18 canonical run predates the
flattened 5D direct-lookup baseline; run a fresh canonical benchmark before using
flattened-baseline numbers in the paper.

| Model | Mean (ns) | P99 (ns) | Correctness |
|---|---|---|---|
| RBAC hash map | 24.23 | 30 | Over-authorizes (no state/provenance) |
| NGAC-DAG traversal | 224.42 | 310 | Over-authorizes (no state) |
| H-NGAC 3D | 17.40 | 23 | Over-authorizes |
| H-NGAC 4D | 18.27 | 28 | **Correct** |
| H-NGAC 5D | 21.28 | 31 | **Correct** |
| RBAC + SQLite state | 376.83 | 674 | Correct — empirical in-process SQLite |
| RBAC + modeled state | 103,485 | — | Correct — **NOT empirical** (modeled delay) |
| Flattened 5D direct lookup | pending fresh run | pending fresh run | Correct — materialized allow-set baseline |

Key derived comparisons:
- 4D vs 3D overhead: +5.0% (within noise — sign fluctuates across runs)
- 5D vs 4D overhead: +16.5% (software path; hardware LUT overhead pending synthesis)
- NGAC-DAG vs 4D slowdown: 12.3×
- RBAC + SQLite vs 4D slowdown: 20.6×

**Do not use the April 13 smoke-run numbers (15.5 ns / 15.1 ns).** Those are superseded —
timer overhead (~25 ns/call) exceeded the signal at 20k iterations.

---

## Attack Demo Results (April 18, 2026)

### Attack Class 2 — Command Injection via Compromised Authenticated Node (5D defense)

30-second ROS2 session, 3 nodes: gatekeeper_5d_node, legit_ros2_node, compromised_ros2_node.

| Metric | Result |
|---|---|
| Injection attempts (Subject 1, prov=remote_operator) | 18,878 |
| Blocked by 5D provenance check | **18,878 (100%)** |
| Legitimate commands passed | 17,059 (0% false positive) |
| Authorization latency min (callback) | 23 ns |

Log: `data/attack2_gatekeeper_20260418_150727.log`

### Attack Class 1 — Timing-Window Bypass (4D defense)

0 timing-window slips observed under WSL2 in both baseline and stress-ng --cpu 8 load.  
Authorization callback P99 stayed well below the 50 µs DDS threshold in both conditions.

---

## Repository Map

| Path | Purpose |
|---|---|
| `fpga/hls/src/hngac_kernel.cpp` | HLS kernel — `hngac_authorize()` with INTERFACE + PIPELINE pragmas |
| `fpga/hls/include/` | `Bitmask256`, `StateMask`, `PolicyRule`, `AuthorizationRequest` types |
| `fpga/hls/tb/hngac_kernel_tb.cpp` | 45-test correctness testbench |
| `fpga/hls/bench/hngac_compare_benchmark.cpp` | local comparison benchmark with H-NGAC, graph, RBAC, SQLite, and flattened lookup paths |
| `fpga/hls/scripts/vitis_hls.tcl` | Vitis HLS synthesis script (env-var configured) |
| `fpga/hls/scripts/run_local_compare.sh` | Single benchmark run helper |
| `paper/main.tex` | Paper draft (gitignored) |
| `paper/refs.bib` | Bibliography (gitignored) |
| `docs/canonical-context.md` | Authoritative numbers and framing rules — read before touching numbers |
| `docs/evidence-record-2026-04-18.md` | Full empirical evidence record with reproducibility instructions |
| `docs/hw-team-update-2026-04-20.md` | Updated guidance for the Badawy lab HW lead |
| `docs/coordination-board.md` | Multi-agent and human file ownership board |
| `docs/decision-log.md` | Architecture and scope decisions with rationale |

---

## Quick Start

```bash
# Build and test (keep off the Windows-backed tree if on WSL)
cmake -S fpga/hls -B /tmp/hngac-build
cmake --build /tmp/hngac-build
ctest --test-dir /tmp/hngac-build --output-on-failure
# Expected: 45 passed, 0 failed

# Run the local comparison benchmark
/tmp/hngac-build/hngac_compare_benchmark 200000 100000

# Run HLS synthesis (requires Vitis HLS + target part)
export HNGAC_HLS_PART=<your-part>
export HNGAC_HLS_CLOCK_NS=5.0
export HNGAC_HLS_WORKDIR=/tmp/hngac-hls
export HNGAC_HLS_COSIM=1
vitis_hls -f fpga/hls/scripts/vitis_hls.tcl
```

---

## Author Roles

Confirmed 2026-08-07. Order as listed.

| Author | Affiliation | Role | Primary Contribution |
|---|---|---|---|
| Hassan Karim | Stable Cyber LLC | PI / Corresponding | Research design, paper writing, benchmark harness, submission |
| Omar Faruque | Florida International University | Hardware | Vitis HLS synthesis, co-simulation, PYNQ-Z1 board bring-up and verification |
| Abdel-Hameed A. Badawy | Florida International University | Hardware Lead | Hardware direction and review |
| Sai Sitharaman | [confirm] | Co-Author | BigData 2025 hypergraph lineage, cloud/AWS resources |
| Deepti Gupta | [confirm] | Co-Author | DCAS lineage, paper review, related work |

Open: confirm Sitharaman and Gupta affiliations, and how Badawy renders his name on
recent papers (Abdel-Hameed A. Badawy vs Abdel-Hameed Badawy).

---

## Key Framing Rules (do not violate in paper text)

- Cite 157 µs as "observed scheduling outlier," not "WCET." The software authorization bound
  is non-deterministic by construction; the OS cannot guarantee even its mean.
- The RBAC 6,674× figure is modeled (busy-wait). Use 20.6× (RBAC+SQLite vs 4D) for
  empirical RBAC+state comparisons.
- BigData 2025 (0.12 s) is a batch compliance sweep, not per-decision latency. Keep it
  out of the latency comparison table; cite as complementary prior work.
- ICCCN 2026 (TS-NGAC, 0.065 µs) is a different paper — cite as complementary, do not
  reproduce its contribution as this paper's own.
- The KEY FINDING must lead Section IV and the conclusion as a hardware claim pending
  synthesis: zero-cost security dimensionality. Three attack classes for the resource
  cost of one if 4D/5D reports confirm the same LUT-stage count.
