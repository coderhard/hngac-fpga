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
**Hardware overhead: zero additional LUT stages on UltraScale+.**
5-input AND tree resolves in same stage count as 3-input.

### KEY FINDING (must be central to the paper)
Security dimensionality scales at zero hardware cost.
Three attack classes blocked for the resource cost of one.
This property does NOT hold in software.

---

## Prior Work (authoritative — do not change these numbers)

### DCAS 2026 (accepted, camera-ready submitted)
"Hardware-Accelerated NGAC Authorization for Real-Time Multi-Robot Systems"
Karim, Gupta

- Pure bitmask AND: **34–37 ns** (two independent platforms)
- ROS2 callback: **1.05 µs** mean
- Memory footprint: **<10 KB**
- Software scheduling outlier: **157 µs** (single OS jitter spike — not a
  repeatable WCET; mean was ~1.14 µs, min ~37 ns per author clarification)
- OPA edge deployment: **1–5 ms** (measured)
- XACML OpenPDP: **~50 ms** (measured)

### ICCCN 2026 (submitted March 2026)
"Deterministic Time-Scoped NGAC for Real-Time Multi-Robot Systems"
Karim, Gupta, Sitharaman

- H-NGAC mean: **0.065 µs** / P99: **0.214 µs**
- TS-NGAC mean: **0.105 µs** / P99: **0.249 µs**
- OPA mean: **271.491 µs**, 13 deadline misses
- Speedup vs OPA: **2,585×** mean

**IMPORTANT:** IPCCC must NOT duplicate ICCCN content.
ICCCN = time-scoped software extension.
IPCCC = hardware synthesis of base primitive + 4D + 5D + adversarial demo.
Complementary, not overlapping.

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
5D vs 4D: +16.5% (software path only; hardware overhead is zero LUT stages)
NGAC-DAG vs 4D: 12.3×
RBAC+SQLite vs 4D: 20.6×

Note on 15 ns vs 17–18 ns: Different compiler flags and WSL2 state between runs.
Both valid. DCAS used separate hardware (34–37 ns). All three environments documented.
Not contradictions — different measurement contexts.

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
- BigData 2025 (0.12 s) is a batch sweep result. Do not put it in the latency table.
- ICCCN 2026 (TS-NGAC, 0.065 µs) is a different paper. Do not reproduce its
  contribution as IPCCC's own. Cite it as complementary prior work.
- The KEY FINDING must lead Section IV and the conclusion: zero-cost security
  dimensionality. Three attack classes for the resource cost of one.
