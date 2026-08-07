---
name: canonical-context
description: Authoritative numbers and framing for the IPCCC 2026 paper — do not change these without explicit author instruction
type: project
---

# H-NGAC Canonical Context — IPCCC 2026

**Owner:** Hassan Karim, Stable Cyber LLC
**Status:** Authoritative — do not override these numbers without explicit instruction

---

## The H-NGAC Bitmask System

NGAC policy graphs compiled to deterministic bitmask structures.
Authorization reduces to bitwise AND operations. Three additive,
backward-compatible variants:

### 3D (Subject AND Object AND Attribute)
```
permit(s,o,a) = S_mask[s] AND O_mask[o] AND A_mask[a]
```
Blocks: unauthorized access from unprivileged agents
CVE anchor: CVE-2022-45789 (Schneider Modicon session hijack)

### 4D (+ System State Vector Σ)
```
permit(s,o,a,σ) = 3D AND Σ_mask[σ]
```
Blocks: safety-interlock bypass, operation outside permitted state
(battery_low, maintenance_mode, calibration_required)
CVE anchor: CVE-2022-33323 (Mitsubishi MELFA unauthorized cmd)
**Measured overhead: -2.83% (noise). 4D is computationally free.**

### 5D (+ Provenance Bit Flags Π)
```
permit(s,o,a,σ,π) = 4D AND Π_mask[π]
```
Blocks: command injection from compromised authenticated nodes.
SROS2 authenticates WHO a node is at transport layer (DDS-Security PKI).
5D enforces whether that SOURCE TYPE is permitted to issue that command
to that resource — application layer, not transport layer.
Provenance bits: Bit 0=authenticated_ros2_node, Bit 1=local_terminal,
Bit 2=remote_operator, Bits 3+=reserved
CVE anchor: CVE-2021-38425 (eProsima Fast DDS RTPS injection)
**Hardware overhead: MEASURED 2026-08-05. Zero cycles, +11.4% LUT.**
Superseded the prior placeholder. See "Hardware Results" below.

### KEY FINDING (must be central to the paper)
**Security dimensionality scales at zero time cost in hardware. MEASURED, not projected.**

4D and 5D resolve in an identical number of clock cycles at every policy size
tested, with identical initiation interval (II=1) and identical timing slack.
The fifth dimension costs +524 LUT (+11.4%) and +100 FF (+3.9%), zero BRAM and
zero DSP. This property does NOT hold in software, where each added dimension
steepens the per-rule slope (3D 0.645, 4D 1.024, 5D 1.220 cycles per rule).

**Precise wording rule.** The claim is "free in time, nearly free in area," NOT
"free." Do not write "zero hardware cost" unqualified — the LUT delta is real and
a reviewer will find it in the csynth report. Do not claim equal LUT-stage count
across 3D/4D/5D: **3D was never synthesized.** Only 4D and 5D exist. Either run
the 3D csynth or scope the claim to 4D vs 5D.

Target part is **Zynq-7020 (xc7z020-clg400-1)**, not UltraScale+. Earlier drafts
said UltraScale+; that was aspirational and is now wrong.

---

## Prior Work (authoritative — do not change these numbers)

### IEEE DASC 2026 (accepted) — BEST POSTER AWARD
"Hardware-Accelerated NGAC Authorization for Real-Time Multi-Robot Systems"
Karim, H., S. Sitharaman, D. Gupta. No DOI yet.

**Venue name discrepancy — resolve before submitting.** Author-supplied record says
**DASC** (Dependable, Autonomic and Secure Computing). Earlier repo docs and the
OneDrive folder `DCAS2026_HW_NGAC_Auth/` say **DCAS**. The author list also differs
(earlier docs omitted Sitharaman). The author-supplied version above is treated as
authoritative; confirm the venue string against the acceptance email before the
bibliography is final.

- Pure bitmask AND: **34–37 ns** (two independent platforms)
- ROS2 callback: **1.05 µs** mean
- Memory footprint: **<10 KB**
- Software scheduling outlier: **157 µs** (single OS jitter spike — not a
  repeatable WCET; mean was ~1.14 µs, min ~37 ns per author clarification)
- OPA edge deployment: **1–5 ms** (measured)
- XACML OpenPDP: **~50 ms** (measured)

### TS-NGAC — WITHDRAWN from ICCCN 2026, being retargeted to a journal
"Deterministic Time-Scoped NGAC for Real-Time Multi-Robot Systems"
Karim, Gupta, Sitharaman

**Status: withdrawn, unpublished.** Cite as "under review" or "in preparation" —
never as an ICCCN publication.

- H-NGAC mean: **0.065 µs** / P99: **0.214 µs**
- TS-NGAC mean: **0.105 µs** / P99: **0.249 µs**
- OPA mean: **271.491 µs**, 13 deadline misses
- Speedup vs OPA: **2,585×** mean

**Reuse policy (revised 2026-08-07, supersedes the earlier do-not-duplicate rule).**
The withdrawal removes the double-submission concern that motivated the old rule.

- **The OPA and XACML baseline data from this run MAY be reused in IPCCC.** IPCCC has
  no OPA implementation of its own, and "why not just use OPA" is the first reviewer
  question. OPA mean 271.491 us with 13 deadline misses is the answer.
- **The time-scoping contribution itself stays with the journal paper.** IPCCC's
  contribution is the hardware primitive and the dimensionality result. Do not present
  time-scoped delegation as an IPCCC contribution.
- Author decision still required: if the journal version is not yet submitted,
  publishing overlapping material at IPCCC first may affect its novelty claim. Confirm
  the contribution split before the manuscript is final.

### REQUIRED CITATIONS — the hypergraph/NGAC lineage

Author instruction 2026-08-07: **every** paper in this project must cite all four.
This is the group's own research line; omitting any of them misrepresents the lineage.

1. Karim, H., S. Sitharaman, D. Gupta (accepted 2026). "Hardware-Accelerated NGAC
   Authorization for Real-Time Multi-Robot Systems." IEEE DASC 2026.
   **BEST POSTER AWARD.** No DOI yet.
2. S. Sitharaman, H. Karim, D. Gupta, and M. Tyagi, "Scalable Privilege Analysis for
   Multi-Cloud Big Data Platforms: A Hypergraph Approach," 2025 IEEE International
   Conference on Big Data (BigData), pp. 6626-6633, Dec. 2025.
   https://doi.org/10.1109/bigdata66926.2025.11401728
3. H. Karim, D. Gupta, and S. Sitharaman, "Securing LLM Workloads with NIST AI RMF in
   the Internet of Robotic Things," IEEE Access, pp. 1-1, Jan. 2025.
   https://doi.org/10.1109/access.2025.3561235
4. H. Karim, S. Sitharaman, D. Gupta, D. B. Rawat, "Securing Autonomous Clinical
   Agents: Time-Scoped Hypergraph Delegation for Controlling Patient Data Access."
   Presented at IEEE ICDH 2026, Sydney, Australia.

Item 2 (BigData) is the **intellectual origin** of this work and should appear in the
Introduction, not only in Related Work.

### BigData 2025 (published)
"Scalable Privilege Analysis for Multi-Cloud Big Data Platforms"
Sitharaman, Karim, Gupta, Tyagi

- NGAC-Hypergraph detection at n=4000: 0.12 seconds
- **THIS IS BATCH SWEEP, NOT PER-DECISION LATENCY.**
- Do not mix into comparison table. State distinction explicitly.

---

## Local Benchmark Results

### April 13, 2026 smoke run (20k iterations, no warmup)
Command: `hngac_compare_benchmark 20000 100000`

| Model | Mean | P99 | Max | Notes |
|---|---|---|---|---|
| H-NGAC 3D | 15.526 ns | 18 ns | 134 ns | |
| H-NGAC 4D | 15.087 ns | 17 ns | 722 ns | See cold-start note |
| RBAC + ext. state | 100,688 ns | — | — | **MODELED — NOT empirical** |

4D vs 3D overhead: **-2.83%** (noise; 4D effectively free)
RBAC vs 4D slowdown: **6,674×** (modeled busy-wait, not empirical — do not cite as measurement)

**Cold-start note:** 4D max=722 ns vs 3D max=134 ns was suspected cold-start artifact.
CONFIRMED RESOLVED: 200k-iteration run with 1k warmup (April 18, 2026) shows no spike.
The 722 ns was a one-time cold-start; steady-state max is OS scheduler jitter, not logic cost.

### April 18, 2026 canonical run (200k iterations, 1k warmup per model)
Command: `hngac_compare_benchmark 200000 100000`

| Model | Mean | P99 | Allowed/Total | Notes |
|---|---|---|---|---|
| RBAC hash map | 24.23 ns | 30 ns | 200k/200k | Over-authorizes (no state/provenance) |
| NGAC-DAG traversal | 224.42 ns | 310 ns | 200k/200k | Over-authorizes (no state) |
| H-NGAC 3D | 17.40 ns | 23 ns | 200k/200k | Over-authorizes |
| H-NGAC 4D | 18.27 ns | 28 ns | 100k/200k | Correct |
| H-NGAC 5D | 21.28 ns | 31 ns | 100k/200k | Correct |
| RBAC + SQLite state | 376.83 ns | 674 ns | 100k/200k | Empirical in-process |
| RBAC + modeled state | 103,485 ns | — | 100k/200k | NOT empirical |

4D vs 3D: +5.0% (noise — same conclusion as smoke run; sign fluctuates)
5D vs 4D: +16.5% (software path only; hardware overhead pending synthesis)
NGAC-DAG vs 4D: 12.3×
RBAC+SQLite vs 4D: 20.6×

Note on 15 ns vs 17–18 ns: Different compiler flags and WSL2 state between runs.
Both valid. DCAS used separate hardware (34–37 ns). All three environments documented.
Not contradictions — different measurement contexts.

---

## Hardware Results (August 5, 2026) — AUTHORITATIVE

Source: `hngac-package-from-farouq/` (see its `PROVENANCE.md`).
Toolchain Vitis HLS 2025.2. Part `xc7z020-clg400-1` (Zynq-7020 / PYNQ-Z1).
Clock 10 ns (100 MHz). All tables below re-derived from raw reports and verified
to reproduce exactly on 2026-08-07.

### Kernel cycles per decision (co-simulation, Verilog, Pass)

4D and 5D are identical at every point. min = avg = max at every point.

| Rules | 4D | 5D |
|---|---|---|
| 4 | 14 | 14 |
| 10 | 17 | 17 |
| 50 | 37 | 37 |
| 100 | 62 | 62 |
| 200 | 112 | 112 |
| 500 | 262 | 262 |

**Closed-form latency: cycles = 12 + rules/2.** Exact at every measured point.
The optimized kernel checks two rules per clock, hence 0.5 cycles per rule.

### Resource utilization (csynth)

| Metric | 4D | 5D | Delta |
|---|---|---|---|
| II | 1 | 1 | 0 |
| Iteration latency | 3 | 3 | 0 |
| LUT | 4580 (8%) | 5104 (9%) | +524 (+11.4%) |
| FF | 2579 (2%) | 2679 (2%) | +100 (+3.9%) |
| BRAM | 0 | 0 | 0 |
| DSP | 0 | 0 | 0 |
| Timing slack | 0.33 ns | 0.33 ns | 0 |

### Vivado place-and-route (earlier 4D run)

Source: `hngac-package-from-farouq/kernel/4d/hngac-fpga_4d_hw_results/vivado_pr_report/`
WNS +2.170 ns, TNS 0.000, **0 failing endpoints of 1987**. WHS +0.044 ns, 0 failing.
This is the only post-route data we have. It is from the earlier 4D kernel, not opt-v1.

### Software per-decision cycles (perf, i7-12800H at 4.96 GHz)

Derived as mean ns x measured clock. 200k iterations, 1k warmup.

| Model | 4 | 10 | 50 | 100 | 200 | 500 | Correct? |
|---|---|---|---|---|---|---|---|
| H-NGAC 3D | 70 | 75 | 115 | 151 | 211 | 390 | No, over-authorizes |
| H-NGAC 4D | 74 | 78 | 120 | 194 | 292 | 582 | No, over-authorizes |
| H-NGAC 5D | 80 | 82 | 130 | 221 | 360 | 685 | **Yes** |
| RBAC hash map | 97 | 87 | 93 | 91 | 89 | 100 | No, over-authorizes |
| NGAC-DAG traversal | 608 | 638 | 636 | 629 | 636 | 1032 | No, over-authorizes |
| Flattened 5D lookup | 990 | 1016 | 970 | 1003 | 1003 | 984 | **Yes**, 19.7x memory |

### Marginal cost per policy rule (cycles per rule)

| | 3D | 4D | 5D |
|---|---|---|---|
| Software | 0.645 | 1.024 | 1.220 |
| Hardware | not synthesized | 0.500 | 0.500 |

### Board verification, PYNQ-Z1 silicon

2,307 requests across 6 rule counts, **all PASS**, allow/deny counts match csim
and cosim exactly. **Functional verification only — no on-board timing was taken.**

### Honesty constraints on the hardware numbers

- Wall clock: at 500 rules HW is 2.62 us (262 cycles at 100 MHz), SW is 138 ns
  (685 cycles at 4.96 GHz). **The CPU beats the FPGA by ~19x on mean wall clock.**
  Never imply otherwise. The hardware argument is boundedness, not mean speed.
- The defensible comparison is worst case. SW 5D worst observed vs HW worst
  (= HW best, since variance is zero): 4 rules 298.8 us vs 0.14 us; 200 rules
  355.4 us vs 1.12 us; 500 rules 17.2 us vs 2.62 us.
- SW cycles are **derived** (mean ns x 4.96 GHz), not per-decision counter reads.
  The benchmark also emits a `CYCLES|` line that disagrees (82.29 vs 70 for 3D at
  4 rules). State which method the paper uses.
- The SW baseline is a 4.96 GHz laptop CPU; the fabric is a 100 MHz Zynq. An
  ARM Cortex-A9 run on the PYNQ-Z1 itself would make this comparison fair.

---

## Attack Class 2 Demo Results (April 18, 2026)

30-second session, ROS2 Jazzy, WSL2. Log: `data/attack2_gatekeeper_20260418_150727.log`

| Metric | Result |
|---|---|
| Injection attempts (subject=1, prov=remote_operator) | 18,878 |
| Blocked by 5D | **18,878 (100%)** |
| Legitimate commands (subject=1, prov=authenticated_ros2_node) | 17,059 |
| False positives | **0 (0%)** |
| Authorization latency min (callback context) | 23 ns |

---

## Paper Framing Rules

- Cite 157 µs as "observed scheduling outlier" not "WCET." Framing: software
  authorization is unbounded by construction; the OS cannot guarantee even its mean.
- The RBAC 6,674× figure is from a modeled busy-wait baseline. It must never appear
  as an empirical measurement. Use the SQLite-backed comparison (20.6× over 4D) for
  empirical RBAC+state claims.
- BigData 2025 is the **intellectual origin of this work** (the hypergraph approach to
  privilege analysis) and must be cited prominently in the Introduction and Related Work.
  The narrow rule is only about one number: 0.12 s is a batch compliance sweep over
  n=4000, not a per-decision latency, so it must never appear in the per-decision latency
  table. Cite the paper freely; keep that one figure out of that one table.
- TS-NGAC is withdrawn and unpublished. Never cite it as an ICCCN paper. Its OPA and
  XACML baselines may be reused in IPCCC; its time-scoping contribution may not.
- The KEY FINDING must lead Section IV and the conclusion as a **measured** result:
  adding the provenance dimension costs zero clock cycles and +11.4% LUT, while in
  software the same dimension makes every policy rule 19% more expensive. Phrase it
  as "free in time, nearly free in area," never as "zero hardware cost."
- **3D was never synthesized.** Do not write that 3D, 4D and 5D resolve in the same
  LUT-stage count. Only 4D vs 5D is supported by evidence.
- The target part is Zynq-7020, not UltraScale+. Correct any surviving draft text.
- Never present the board test as a timing result. It is functional PASS only.
- Never present the FPGA as faster than the CPU on mean latency. It is not. The
  claim is a bounded, jitter-free, closed-form worst case.
