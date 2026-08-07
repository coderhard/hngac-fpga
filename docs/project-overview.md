---
name: project-overview
description: Single-source-of-truth entry point for all collaborators — paper, repo, status, and canonical numbers
type: project
---

# H-NGAC IPCCC 2026 — Project Overview

**PI:** Hassan Karim, Stable Cyber LLC  
**Venue:** IEEE IPCCC 2026 (deadlines TBD — est. abstract June, final July 2026)  
**Repo:** `https://github.com/coderhard/hngac-fpga`  
**Paper draft:** `paper/main.tex` (tracked in git since 2026-08-07; full v1 draft, 6 pages)

---

## Core Claim

A 5D H-NGAC (Hardware-NGAC) bitmask authorization primitive delivers deterministic,
hardware-bounded WCET that software cannot guarantee regardless of optimization level.

**Terminology.** The H is **Hardware**. H-NGAC was presented at DCAS 2026; this paper
**extends** it with the state and provenance dimensions. It is a different system from
**HyperNGAC**, the hypergraph privilege analysis of BigData 2025. See the TERMINOLOGY
table at the top of `docs/canonical-context.md` before writing prose.

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
| Command-provenance-abuse ROS2 demo | **Done** | 18,878 injections blocked, 100%, 0 FP |
| Timing-window data (not an attack class) | **Done** | 0 slips under WSL2 load — null result |
| Manuscript draft | **v1 complete 2026-08-07** | `paper/main.tex`, 6 pages, all real data; the April skeleton was lost (gitignored, local-only) and the draft was rebuilt from zero |
| HLS synthesis reports (4D + 5D) | **Done 2026-08-05** | `hngac-package-from-farouq/results/cosim-opt-v1-{4d,5d}/syn_report/` |
| RTL co-simulation (4D + 5D) | **Done 2026-08-05** | Verilog Pass; per-call cycles in `cosim_report/verilog/result.transaction.rpt` |
| Board verification (5D) | **Done** | 2,307 requests PASS on PYNQ-Z1 silicon — functional only, no timing |
| Vivado place-and-route | **Partial** | Earlier 4D run only: WNS +2.170 ns, 0 failing endpoints |
| 3D synthesis | **Permanently out for IPCCC** | Omar has no 3D kernel code (2026-08-07); every claim scoped to 4D vs 5D |
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

The canonical attack-class taxonomy is in `docs/canonical-context.md`. Integer
numbering is retired; the classes are **unauthorized access** (3D), **unsafe-state
operation** (4D) and **command provenance abuse** (5D).

### Command Provenance Abuse — closed by 5D

30-second ROS2 session, 3 nodes: gatekeeper_5d_node, legit_ros2_node, compromised_ros2_node.
The attacker holds valid DDS credentials for Subject 1 but is not an entitled source type.

| Metric | Result |
|---|---|
| Injection attempts (Subject 1, prov=remote_operator) | 18,878 |
| Blocked by 5D provenance check | **18,878 (100%)** |
| Legitimate commands passed | 17,059 (0% false positive) |
| Authorization latency min (callback) | 23 ns |

Log: `data/attack2_gatekeeper_20260418_150727.log` — the `attack2_` token is the old
numbering, preserved because it is a committed evidence identifier.

### The timing window — NOT an attack class, and a null result

0 slips observed under WSL2 in both baseline and `stress-ng --cpu 8` load.
Authorization callback P99 stayed well below the 50 µs DDS threshold in both conditions.

This was previously labelled "Attack Class 1 (4D defense)", which was wrong twice. It
is not a peer of the three classes — it is a cross-cutting delivery property that
applies to all of them, since any correct decision is useless if it arrives after the
actuator moved. And it is closed by the synthesis-time latency bound, **not by the
state dimension**. Do not claim a measured slip rate above zero; the argument is
architectural.

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
| `paper/main.tex` | Paper draft (tracked since 2026-08-07) |
| `paper/refs.bib` | Bibliography (tracked since 2026-08-07) |
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
| Sai Sitharaman | Zetafence, Inc. | Co-Author | BigData 2025 hypergraph lineage, cloud/AWS resources |
| Deepti Gupta | Texas A&M University–Central Texas | Co-Author | DCAS lineage, paper review, related work |

Open: confirm Sitharaman and Gupta affiliations, and how Badawy renders his name on
recent papers (Abdel-Hameed A. Badawy vs Abdel-Hameed Badawy).

---

## Key Framing Rules (do not violate in paper text)

- Cite 157 µs as "observed scheduling outlier," not "WCET." The software authorization bound
  is non-deterministic by construction; the OS cannot guarantee even its mean.
- The RBAC 6,674× figure is modeled (busy-wait). Use 20.6× (RBAC+SQLite vs 4D) for
  empirical RBAC+state comparisons.
- BigData 2025 (0.12 s) is **HyperNGAC**, a different system: a batch compliance sweep,
  not per-decision latency. Keep that number out of the latency comparison table. Cite
  the paper prominently in the Introduction as the intellectual origin of the line.
- TS-NGAC (0.065 µs) is **withdrawn and unpublished**, retargeted to a journal. Cite it
  as "under review." Never as an ICCCN paper. Its OPA and XACML baselines may be reused;
  its time-scoping contribution may not.
- The KEY FINDING leads Section IV and the conclusion as a **measured** result
  (2026-08-05, Zynq-7020): security dimensionality is free in time. Say "free in time,
  nearly free in area," never "zero hardware cost" — the fifth dimension costs +524 LUT
  (+11.4%). Scope the claim to **4D versus 5D**; 3D was never synthesized, so "three
  attack classes for the area of one" is not yet supported by synthesis data.
- Terminology: H-NGAC is **Hardware-NGAC** and this paper **extends** it rather than
  presenting it. See TERMINOLOGY in `docs/canonical-context.md`.
