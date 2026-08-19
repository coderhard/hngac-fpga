---
name: canonical-context
description: Authoritative numbers and framing for the IPCCC 2026 paper — do not change these without explicit author instruction
type: project
---

# H-NGAC Canonical Context — IPCCC 2026

**Owner:** Hassan Karim, Stable Cyber LLC
**Status:** Authoritative — do not override these numbers without explicit instruction

---

## TERMINOLOGY — read this before writing any prose

Author decision 2026-08-07. These terms were being used interchangeably and
are not interchangeable. Fixed as follows.

| Term | Expansion | What it is | Whose contribution |
|---|---|---|---|
| **H-NGAC** | **Hardware-NGAC** | This line. NGAC policy graphs compiled to fixed-width bitmasks so a decision is a chain of bitwise ANDs. | DCAS 2026 (published) |
| **5D H-NGAC** | — | **This paper.** H-NGAC extended with the state and provenance dimensions, synthesized and measured. | IPCCC 2026 (this submission) |
| **HyperNGAC** | Hypergraph NGAC | A **different system**: the hypergraph privilege analysis of Sitharaman et al., operating on batch compliance sweeps over multi-cloud platforms. | BigData 2025 (published) |
| **DAG-NGAC** | — | The INCITS 565 reference model, where a decision requires reachability traversal of a policy DAG. Our `NGAC-DAG traversal` benchmark row. | Prior art, not ours |

**Rules.**

1. The H in H-NGAC is **Hardware**, not Hypergraph. Expand it on first use in
   every document and in the paper.
2. **Never describe this system as compiling "hypergraphs."** It compiles NGAC
   policy **graphs**. The hypergraph framing belongs to HyperNGAC and BigData 2025.
   (A `PolicyRule` is structurally an n-ary association, so the framing is not
   *false* here, but claiming it collides with BigData's contribution and confuses
   the two lines. Do not use it.)
3. **This paper does not "present H-NGAC."** DCAS presented H-NGAC. This paper
   **extends** it. Any sentence beginning "We present H-NGAC" over-claims against
   our own published work.
4. The IPCCC contribution word is **dimensionality**, not hardware and not
   hypergraph. Hardware is DCAS's claim; hypergraph is BigData's. Ours is that the
   fourth and fifth dimensions are free in time.
5. **HyperNGAC is a proposed name for someone else's system.** Sitharaman is first
   author on BigData 2025. Confirm with him before it appears in print.

**Known label mismatch (not yet fixed).** The benchmark emits software rows labeled
`H-NGAC 3D`, `H-NGAC 4D`, `H-NGAC 5D`. Under H = Hardware those read as
"Hardware-NGAC measured on a 4.96 GHz i7." The committed CSVs and the figures in
`docs/figures/` carry those labels, as does Farouq's package, so renaming the
harness would break label continuity with committed data. Until that is decided,
write **"the H-NGAC algorithm evaluated in software"** in prose and label figure
series explicitly as software.

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

### Σ is POSITIVE and MONOTONE — this constrains how we may describe a deny

Verified in source 2026-08-20 (`fpga/hls/include/hngac_types.hpp`,
`fpga/hls/src/hngac_kernel.cpp`):

```c
contains_all_states(required, available) => (required & available) == required
```

A rule fires only when **every** bit in its `required_states` is asserted in the
request's `object_state`. `required_states == 0` is a wildcard. There is no
negative term. **Asserting a state bit can therefore only make MORE rules match,
never fewer.**

**Consequence for the prose.** An asserted `maintenance_mode` bit cannot, by
itself, deny anything. A deny arises from the **absence of a required bit**, not
the presence of a hazard bit. The benchmark encodes it exactly that way:
`make_deny_state_mask(required)` returns `required & (required - 1)`, which
clears the lowest required bit. So the measured 4D denials are all
missing-permissive-bit denials, and the numbers in "Hardware Results" stand.

The four `StateBit` names (`battery_low`, `maintenance_mode`, `safety_interlock`,
`calibration_required`) are named as hazards, which invites the opposite reading.
A rule carrying `required_states = {maintenance_mode}` is a rule that fires **only
during** maintenance, e.g. "a technician may run diagnostics in maintenance mode."
`hngac_kernel_tb.cpp:54,86` asserts precisely that and expects PERMIT.

**Wording rule.** Never write that the model denies because `maintenance_mode` is
set. Write that motion rules require a permissive bit that maintenance clears.
Definition 1 and Definition 3 in the manuscript are already correct and precise
("the state bits that must be asserted for the rule to apply", `σ_i ⊆ σ_q`); it
is the threat-model narrative and the Layla framing that must match them.

Adding a genuine negative condition (`forbidden_states`) would be a kernel change
plus re-synthesis. Out of scope for IPCCC 2026.

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
**Hardware overhead: MEASURED. Zero cycles across 3D/4D/5D; +11.4% LUT for 5D over 4D,
+35.5% across the full 3D→5D span.** 4D/5D 2026-08-05, 3D 2026-08-13.
Superseded the prior placeholder. See "Hardware Results" below.

---

## ATTACK CLASS TAXONOMY — canonical, supersedes all integer numbering

Author decision 2026-08-07. **Integer numbering ("Attack Class 1", "Attack Class 2")
is retired.** It was never assigned in dimension order, only ever named two of the
classes, and caused `docs/project-overview.md` to attribute the timing window to 4D,
which is wrong on every reading. Use the names below. They are the same three names
already used at the top of `CLAUDE.md`.

| Attack class | Closed by | What the attacker does | CVE anchor |
|---|---|---|---|
| **Unauthorized access** | 3D (subject, object, attribute) | Acts as an unprivileged agent or a hijacked session | CVE-2022-45789 (Schneider Modicon session hijack) |
| **Unsafe-state operation** | 4D (+ system state) | Issues a legitimate command while the platform is in `battery_low`, `maintenance_mode` or `calibration_required` | CVE-2022-33323 (Mitsubishi MELFA unauthorized command) |
| **Command provenance abuse** | 5D (+ command provenance) | Holds valid DDS credentials for an authorized subject but is not an entitled source type | CVE-2021-38425 (eProsima Fast DDS RTPS injection) |

### The timing window is NOT an attack class

It is a cross-cutting delivery property that applies to all three classes above: any
correct decision is useless if it arrives after the actuator has already moved. It is
closed by the synthesis-time latency bound, **not by any dimension**. This is the
single biggest source of the old confusion — the timing window was numbered as if it
were a peer of the other classes, then attributed to 4D, which has nothing to do with it.

**Do not claim a measured slip rate above zero.** Evidence block 5 measured 0 slips in
8,733 callbacks, both at baseline and under `stress-ng --cpu 8`. It is a null result.
The argument for the timing window is architectural: a synthesis-time bound closes the
window by construction. It has no place in an abstract as an effectiveness number.

### CVE-2022-45789 was doing double duty — resolved

It anchored 3D (unauthorized access) here while `docs/evidence-record-2026-04-18.md`
used it to anchor "Attack Class 1, Timing-Window Bypass." Those are different failure
modes. The CVE is a session-hijack, so it belongs to **unauthorized access** only. The
timing window has no CVE anchor and does not need one.

### Legacy identifiers preserved on purpose

`data/attack2_gatekeeper_20260418_150727.log` and the `attack2_*` filename tokens in
`scripts/ros2_demo/*.sh` keep the old numbering. They are **committed evidence
identifiers**; renaming them would break the reproduction path against committed data.
`attack2_*` means **command provenance abuse**.

---

### KEY FINDING (must be central to the paper)
**Security dimensionality scales at zero time cost in hardware. MEASURED, not projected.**

3D, 4D and 5D resolve in an identical number of clock cycles at every policy size
tested, with identical initiation interval (II=1), identical 6.965 ns estimated
clock, and identical 0.33 ns timing slack. Carrying both added dimensions costs
+1,336 LUT (+35.5%) and +331 FF (+14.1%) over 3D, zero BRAM and zero DSP. This
property does NOT hold in software, where each added dimension steepens the
per-rule slope (3D 0.645, 4D 1.024, 5D 1.220 cycles per rule).

**Precise wording rule.** The claim is "free in time, nearly free in area," NOT
"free." Do not write "zero hardware cost" unqualified — the LUT delta is real and
a reviewer will find it in the csynth report.

**The three-way claim is now supported (3D synthesized 2026-08-13, package v2).**
The former prohibition on claiming 3D/4D/5D parity is RETIRED. Two qualifications
travel with it:

- **Iteration latency is not identical: 2 (3D), 3 (4D), 3 (5D).** Pipeline depth
  grows one stage when state arrives, then stops. II=1 amortizes it, so the
  per-decision cycle count does not move. State this in the paper; do not let a
  reviewer discover it in the csynth report.
- **+11.4% LUT is the 5D-vs-4D delta only.** The 3D→4D delta is +812 LUT (+21.6%)
  and the full 3D→5D span is +1,336 LUT (+35.5%). Label which span any percentage
  refers to.

**Second key finding, also measured: identical cycles buy different security.**
The 3D and 4D co-simulations ran the same 2,307-request corpus as 5D. At 500
rules 3D permits all 1,334 requests, 4D permits 917, 5D permits 500 — at
identical cycle cost. Over-authorization is now an RTL result, not a
software-only argument.

Target part is **Zynq-7020 (xc7z020-clg400-1)**, not UltraScale+. Earlier drafts
said UltraScale+; that was aspirational and is now wrong.

---

## Prior Work (authoritative — do not change these numbers)

### IEEE DCAS 2026 (published) — BEST POSTER AWARD
"Hardware-Accelerated NGAC Authorization for Real-Time Multi-Robot Systems"
H. Karim, S. Sitharaman, D. Gupta, *2026 IEEE 19th Dallas Circuits and Systems
Conference (DCAS)*, pp. 1-4. https://doi.org/10.1109/dcas69364.2026.11544855

**Venue resolved 2026-08-07 via Crossref.** It is **DCAS** (Dallas Circuits and
Systems), not DASC. Author list confirmed as Karim, Sitharaman, Gupta — earlier repo
docs omitted Sitharaman. Do not reintroduce "DASC" or "no DOI yet".

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
- **Author decision 2026-08-07: cite TS-NGAC as "under review."** That is the agreed
  status string.
- **The OPA/XACML reuse is deferred past the abstract.** Do not add OPA numbers to the
  abstract. Revisit for the manuscript only.

### REQUIRED CITATIONS — the hypergraph/NGAC lineage

Author instruction 2026-08-07: **every** paper in this project must cite all four.
This is the group's own research line; omitting any of them misrepresents the lineage.

1. H. Karim, S. Sitharaman, D. Gupta, "Hardware-Accelerated NGAC Authorization for
   Real-Time Multi-Robot Systems," *2026 IEEE 19th Dallas Circuits and Systems
   Conference (DCAS)*, pp. 1-4, 2026. **BEST POSTER AWARD.**
   https://doi.org/10.1109/dcas69364.2026.11544855
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

- HyperNGAC detection at n=4000: 0.12 seconds
- **THIS IS BATCH SWEEP, NOT PER-DECISION LATENCY.**
- Do not mix into comparison table. State distinction explicitly.
- HyperNGAC is a **different system** from H-NGAC. See TERMINOLOGY at the top.

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

3D, 4D and 5D are identical at every point. min = avg = max at every point.
All three co-simulations Pass.

| Rules | 3D | 4D | 5D |
|---|---|---|---|
| 4 | 14 | 14 | 14 |
| 10 | 17 | 17 | 17 |
| 50 | 37 | 37 | 37 |
| 100 | 62 | 62 | 62 |
| 200 | 112 | 112 | 112 |
| 500 | 262 | 262 | 262 |

### Requests permitted per variant (same corpus, same co-simulation)

Source: `hngac-package-v2-from-farouq/results/cosim-opt-v1-*/cosim_report/verilog/tmp.log`

| Rules | Requests | 3D permits | 4D permits | 5D permits |
|---|---|---|---|---|
| 4 | 11 | 11 | 8 | 4 |
| 10 | 27 | 27 | 19 | 10 |
| 50 | 134 | 134 | 92 | 50 |
| 100 | 267 | 267 | 184 | 100 |
| 200 | 534 | 534 | 367 | 200 |
| 500 | 1334 | 1334 | 917 | 500 |
| **Total** | **2307** | **2307** | **1587** | **864** |

3D permits the entire corpus because every request carries a valid identity
triple and the failing variants differ only in state and provenance.

### On-board AXI Timer capture (PYNQ-Z1, hardware-latched)

Source: `hngac-package-v2-from-farouq/board-test/.../board-scripts/bare_metal_latency.log`
min = avg = max at every policy size. Constant +25-cycle AXI-Lite offset over co-sim.

| Rules | Co-sim | Board | Offset |
|---|---|---|---|
| 4 | 14 | 39 | 25 |
| 10 | 17 | 42 | 25 |
| 50 | 37 | 62 | 25 |
| 100 | 62 | 87 | 25 |
| 200 | 112 | 137 | 25 |
| 500 | 262 | 287 | 25 |

**Board round-trip (CLOCK_MONOTONIC) is a different statistic and is NOT a kernel
result.** Mean rises 2090 → 4765 ns across the sweep, but max reaches 5,920,702 ns
at 500 rules. That tail is PS-side Linux userspace jitter. Never cite it without
attributing it to the software stack rather than the fabric.

**Closed-form latency: cycles = 12 + rules/2.** Exact at every measured point.
The optimized kernel checks two rules per clock, hence 0.5 cycles per rule.

### Resource utilization (csynth)

| Metric | 3D | 4D | 5D | 3D→5D |
|---|---|---|---|---|
| II | 1 | 1 | 1 | 0 |
| Iteration latency | 2 | 3 | 3 | +1 |
| LUT | 3768 (7%) | 4580 (8%) | 5104 (9%) | +1336 (+35.5%) |
| FF | 2348 (2%) | 2579 (2%) | 2679 (2%) | +331 (+14.1%) |
| BRAM | 0 | 0 | 0 | 0 |
| DSP | 0 | 0 | 0 | 0 |
| Est. clock | 6.965 ns | 6.965 ns | 6.965 ns | 0 |
| Timing slack | 0.33 ns | 0.33 ns | 0.33 ns | 0 |

4D→5D alone: +524 LUT (+11.4%), +100 FF (+3.9%).
3D→4D alone: +812 LUT (+21.6%), +231 FF (+9.8%).

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
| Hardware | 0.500 | 0.500 | 0.500 |

### Board verification, PYNQ-Z1 silicon

2,307 requests across 6 rule counts, **all PASS**, allow/deny counts match csim
and cosim exactly. Since 2026-08-13 the board run **also carries hardware-latched
timing** from an AXI Timer (see the capture table above). Round-trip latency from
the PS is a separate statistic and is not a kernel result.

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

## Command Provenance Abuse — Demo Results (April 18, 2026)

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
  adding state and provenance costs zero clock cycles across 3D, 4D and 5D, while in
  software each added dimension makes every policy rule more expensive (the fifth by
  19%). Phrase it as "free in time, nearly free in area," never as "zero hardware cost."
- **The three-way 3D/4D/5D claim is supported as of 2026-08-13.** Report the
  iteration-latency step (2/3/3) alongside it, and label which span any LUT
  percentage refers to.
- The target part is Zynq-7020, not UltraScale+. Correct any surviving draft text.
- Board **AXI Timer cycle counts are a timing result** and may be cited as one:
  hardware-latched, min=avg=max, constant +25-cycle offset over co-sim. Board
  **round-trip** latency may not be cited as a kernel property; its millisecond tail
  is PS-side Linux jitter.
- Never present the FPGA as faster than the CPU on mean latency. It is not. The
  claim is a bounded, jitter-free, closed-form worst case.

---

## HW-team answers from Omar Faruque, 2026-08-07 10:26–10:33

Source: `hngac-package-from-farouq/response_from_farouq.txt`.

### 3D synthesis will NOT happen — the gap is permanent for IPCCC

> "I don't have the 3D kernel code. But 4D, 5D should be enough to infer the trend."

**Consequences, binding.** Scope every claim to **4D versus 5D**. Do not infer 3D and
present it as measured. "Three attack classes for the area of one" is unsupported and the
title using it is retired. The abstract already scopes correctly.

### Board-CPU baseline is possible but the comparison cuts against us

> "We can run on the FPGA boards processor alone but that processor is less capable than
> the 12th gen i7 cpu. Currently we are comparing with a more capable consumer grade CPU."

Not committed to. Note the framing risk: a Cortex-A9 baseline makes the fabric win on
wall clock, but a reviewer can read it as choosing a weak CPU. The stronger and more
honest framing stays **boundedness and zero jitter versus the i7**, with the A9 as a
secondary embedded-realism data point if it arrives.

### Which software cycle method — RESOLVED, with a caveat we must state correctly

> "I reported the sw cycle counts from PERF tool. Cycle lines are from rdtsc tool which I
> experimented with when I didn't have access to a linux machine. PERF counts actual core
> cycles regardless of frequency. So more accurate counts... old test artifacts like rdtsc
> counts remained."

**Use the perf-based numbers** (`extract_sw_cycles.py` / `sw_cycles.csv`). **Discard the
benchmark's `CYCLES|` lines** — they are stale rdtsc artifacts, which resolves the
70-versus-82.29 disagreement.

**Caveat that must not be lost.** The per-decision cycle figures are still **derived**,
not direct per-decision counter reads. `extract_sw_cycles.py`'s own docstring: it pairs
SUMMARY per-decision mean nanoseconds with `cpu_core/cycles` and elapsed time "to derive
the effective clock rate, then converts nanoseconds to cycles." So perf supplies a
*measured* clock (4.96 GHz, not nominal), and the cycle count is mean_ns × that clock.
That is better than rdtsc and better than a nominal clock, but the paper must say
**derived from a perf-measured clock**, never "per-decision hardware counter reads."
Accuracy rule 9 in `docs/manuscript-agent-prompt.md` stands.

### Co-simulation cycles are sound; no board timing

> "Cosim cycle numbers should be fine, they are cycle accurate, board cycle counts would
> be nice, but I didn't have enough time to figure out how to measure that."

Co-simulation remains the authoritative per-decision timing source. **Superseded in part
on 2026-08-13:** the board run now carries an AXI Timer capture, hardware-latched, with
min=avg=max and a constant +25-cycle AXI-Lite offset over cosim, so on-board cycle counts
do exist for IPCCC and corroborate cosim. Board *round-trip* latency still may not be
cited as a kernel property.
