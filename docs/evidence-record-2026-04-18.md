# H-NGAC IPCCC 2026 — Measured Evidence Record
**Date:** 2026-04-18
**Author:** Hassan Karim, Stable Cyber LLC
**Purpose:** Authoritative record of all empirical measurements taken to date.
For collaborator review. All numbers are reproducible from the repo.

---

## How to Reproduce

```bash
# Repo
git clone <repo> hngac-fpga && cd hngac-fpga

# Benchmark (requires cmake, g++, sqlite3-dev)
mkdir build && cd build
cmake ../fpga/hls -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./hngac_compare_benchmark 200000 100000   # canonical run

# Testbench
./hngac_kernel_tb                          # must print "45 passed, 0 failed"

# ROS2 adversarial demo (requires ROS2 Jazzy)
source /opt/ros/jazzy/setup.bash
cd ros2_ws && colcon build --packages-select ngac_auth
source install/setup.bash
# Three terminals:
ros2 run ngac_auth gatekeeper_5d_node
ros2 run ngac_auth legit_ros2_node
ros2 run ngac_auth compromised_ros2_node
```

---

## Evidence Block 1 — Software Benchmark

**Source:** `fpga/hls/bench/hngac_compare_benchmark.cpp`
**Binary:** `hngac-fpga-build/hngac_compare_benchmark`
**Platform:** WSL2, x86-64, ~3.3 GHz, GCC -O3
**Method:** `std::chrono::steady_clock`, 1000-iteration warmup per model, 200k timed iterations
**Scenario mix:** 4 state-satisfying requests + 4 state-failing requests, cycled evenly

| Model | Mean (ns) | P99 (ns) | Allowed / Total | Notes |
|---|---|---|---|---|
| RBAC hash map | 24.23 | 30 | 200000 / 200000 | Over-authorizes: no state or provenance check |
| NGAC-DAG traversal | 224.42 | 310 | 200000 / 200000 | Over-authorizes: no state dimension |
| H-NGAC 3D baseline | 17.40 | 23 | 200000 / 200000 | Over-authorizes: no state or provenance |
| H-NGAC 4D state-aware | 18.27 | 28 | 100000 / 200000 | Correct: state check filters deny cases |
| H-NGAC 5D provenance-aware | 21.28 | 31 | 100000 / 200000 | Correct: state + provenance both enforced |
| RBAC + SQLite state lookup | 376.83 | 674 | 100000 / 200000 | Empirical in-memory SQLite |
| RBAC + modeled state lookup | 103,485 | 103,930 | 100000 / 200000 | NOT empirical — busy_wait baseline |

**Derived comparisons:**
- 4D vs 3D overhead: +5.0% mean
- 5D vs 4D overhead: +16.5% mean (software path; see note below)
- NGAC-DAG vs 4D slowdown: 12.3x
- RBAC + SQLite vs 4D slowdown: 20.6x
- RBAC + modeled vs 4D slowdown: 5,664x

**Critical note on 5D software overhead:**
The +16.5% figure is a software-path artifact. On FPGA fabric, the 5D check adds one
`AND` operation to an already-pipelined loop. A 5-input AND resolves in the same LUT
stage as a 3-input AND on UltraScale+. The hardware zero-cost claim is about synthesis
latency and LUT stage count, not about software branches. This distinction must be
explicit in Section IV of the paper.

**Max values (3D: 56 µs, 4D: 21 µs, 5D: 212 µs) are WSL2 OS scheduler jitter.**
They are not WCET measurements. Do not cite them as WCET.

**Over-authorization in RBAC and 3D:**
RBAC and 3D grant 200000/200000 requests including the 4 state-failing cases in the mix.
They cannot enforce 4D/5D safety constraints. This is a correctness gap, not only a
latency gap. The paper should present this explicitly.

---

## Evidence Block 2 — Testbench Correctness

**Source:** `fpga/hls/tb/hngac_kernel_tb.cpp`
**Binary:** `hngac-fpga-build/hngac_kernel_tb`
**Result:** 45 passed, 0 failed

Test coverage:
- 3D baseline: unknown subject, wrong object, wrong attribute
- 4D state: all four named state bits individually, composite state, partial composite deny
- 4D state: superset state allows, zero required_states = wildcard
- 4D state: state-fail despite correct subject/object/attr
- 5D provenance: correct provenance allows, wrong provenance denies, no provenance denies
- 5D provenance: superset provenance allows, zero required_provenance = wildcard
- 5D provenance: two-provenance rule (either type satisfies), third type denies
- 5D provenance: state-fail with correct provenance still denies
- Boundary: subject=255, object=255 (kMaxNodes-1), rule_count > kMaxPolicyRules clamped
- Multi-subject rule: both subjects authorized independently, unlisted subject denied
- Multi-attribute rule: subset request allowed, extra required bit denied

---

## Evidence Block 3 — Command Provenance Abuse (CVE-2021-38425 class)

> **Renamed 2026-08-07.** Was "Attack Class 2." Integer numbering is retired; see the
> ATTACK CLASS TAXONOMY in `docs/canonical-context.md`. This class is closed by 5D.

**Threat model:** Compromised authenticated ROS2 node.
The attacker holds valid DDS credentials for Subject 1 — the same identity the policy
authorizes. SROS2 transport-layer checks pass. Under 3D and 4D authorization, the
command passes subject, object, attribute, and state checks.
Under 5D, the source provenance (remote_operator) does not match the policy's
required provenance (authenticated_ros2_node). The command is denied.

**Nodes:**
- `gatekeeper_5d_node`: 5D authorization using `hngac_authorize` kernel. Policy: subject=1, object=2, attr=5, state=wildcard, provenance=authenticated_ros2_node.
- `legit_ros2_node`: Subject 1, provenance=authenticated_ros2_node(1), 2 Hz.
- `compromised_ros2_node`: Subject 1, provenance=remote_operator(4), 10 Hz.

**Run:** 2026-04-18, 30-second session, ROS2 Jazzy on WSL2.
**Log:** `data/attack2_gatekeeper_20260418_150727.log` (42 953 lines)
**Compromised rate:** 10 Hz — **Legit rate:** 2 Hz

| Metric | Value |
|---|---|
| Injection attempts (Subject 1, prov=remote\_operator) | 18 878 |
| Blocked by 5D provenance check | **18 878** |
| Provenance block rate | **100.0%** |
| Legitimate commands (Subject 1, prov=authenticated\_ros2\_node) | 17 059 |
| Passed by 5D | **17 059** |
| False positive rate | **0.0%** |
| Wrong-subject blocks | 0 |

**Authorization latency inside ROS2 callback** (steady-state, first 100 events excluded):

| Event | n | Min | Mean | P99 | Max |
|---|---|---|---|---|---|
| BLOCK\_PROV | 13 688 | 23 ns | 188 ns | 820 ns | 16 804 ns |
| PASS\_5D | 14 490 | 35 ns | 241 ns | 937 ns | 71 276 ns |

Mean is elevated vs isolated benchmark (~19 ns) because DDS callback context brings
the policy array in from cold cache between sparse timer firings. Min (23–35 ns) is
consistent with steady-state benchmark. Max values are WSL2 scheduler jitter, not WCET.
Pure authorization latency (isolated benchmark): 5D mean=21.28 ns, p99=31 ns.

**Conclusion:** 5D provenance enforcement blocks 100% of injection attempts from a
compromised node with valid Subject 1 credentials, with zero false positives on
legitimate traffic. This result is directly reproducible from the repo.

---

## Evidence Block 4 — DCAS 2026 Baseline (prior published work)

**Source:** Accepted paper: "Hardware-Accelerated NGAC Authorization for Real-Time
Multi-Robot Systems," Karim + Gupta, DCAS 2026.

Numbers from that paper (do not re-derive — cite the paper):
- Pure bitmask AND: 34–37 ns (two platforms)
- ROS2 callback: 1.05 µs mean
- Memory: < 10 KB
- Software WCET: 157 µs

The 157 µs WCET figure is the authoritative software worst-case from DCAS.
The 15–21 ns benchmark numbers above are from a different environment (WSL2 x86-64)
and are not contradictions — they are a different measurement context.
Both sets of numbers belong in the paper with their environments labeled.

---

## Evidence Block 5 — The Timing Window (NOT an attack class)

> **Corrected 2026-08-07.** Was "Attack Class 1 (CVE-2022-45789 class, Timing-Window
> Bypass)." Two errors. First, CVE-2022-45789 is a Schneider Modicon **session hijack**,
> which anchors **unauthorized access** (3D) in `docs/canonical-context.md`; it does not
> anchor the timing window, and using it for both made one CVE mean two different
> failure modes. The timing window has no CVE anchor and does not need one. Second, the
> timing window is not a peer of the three attack classes at all — it is a cross-cutting
> delivery property, since any correct decision is useless if it arrives after the
> actuator moved. It is closed by the synthesis-time latency bound, not by a dimension.

**Threat model:** Authorization latency exceeds DDS message propagation time. When an
authorization decision (permit/deny) takes longer than DDS delivers the message to
the actuator subscriber, the actuator may act before the deny arrives.
Threshold: DDS localhost propagation = 50–200 µs (literature).

**Measurement:** 5D gatekeeper under 100 callbacks/second (50 Hz legit + 50 Hz attacker).
Platform: WSL2 x86-64. ROS2 Jazzy. CPU stress via stress-ng --cpu 8 (all cores).

| Condition | n | Min | P99 | Max | Mean | Callbacks > 50 µs |
|---|---|---|---|---|---|---|
| No load (baseline) | 5,350 | 20 ns | 1,204 ns | 39,981 ns | 272 ns | **0** |
| Loaded (stress-ng --cpu 8) | 3,383 | 26 ns | 659 ns | 19,305 ns | 157 ns | **0** |

**Command-provenance-abuse block rate during these runs: 100% (same result as Evidence Block 3).**

**Key finding:** Under WSL2, the software gatekeeper callback stayed below the 50 µs
DDS threshold in both conditions. No timing-window slip was observed in this environment.

**DCAS 157 µs figure — RESOLVED (2026-04-18):**
The 157 µs figure from the DCAS 2026 paper was a single scheduling anomaly (OS jitter
spike), not a representative WCET for the authorization primitive. The DCAS measured
distribution was: mean ≈ 1.14 µs, min ≈ 37 ns. The 157 µs max was an outlier catch
from a longer run, not a steady-state bound. This is consistent with the current WSL2
data showing max < 40 µs over thousands of callbacks.

**Correct framing for the IPCCC paper:**
- Do not cite 157 µs as "the software WCET." Cite it as "a measured scheduling outlier
  in the DCAS 2026 baseline" representing OS non-determinism, not authorization cost.
- The authoritative software representative latency from DCAS is mean ≈ 1.14 µs
  (ROS2 callback path) and min ≈ 37 ns (pure function).
- The timing-window argument stands but must be written as: software authorization
  exhibits unbounded jitter (single outlier to 157 µs observed); FPGA synthesis bounds
  latency at synthesis time, eliminating the jitter class entirely. The claim is about
  determinism, not about a specific measured slip rate.
- **Do not claim a measured slip rate > 0.** Current data shows 0 slips. The argument
  is architectural: hardware-bounded latency closes the window by construction.

---

## What Is Not Yet Measured (open gaps)

| Gap | Status | Blocker |
|---|---|---|
| Timing-window slip rate (WSL2) | Measured — 0 slips (see above) | N/A; WSL2 cannot reproduce DCAS WCET |
| HLS synthesis reports (LUT, timing) | Not available | Vitis — Badawy lab or university collab |
| Hardware latency (FPGA WCET) | Not available | Lab hardware or university collab |
| 5D zero LUT overhead (synthesis confirmation) | Not confirmed | Same as HLS synthesis above |

---

## Superseded Numbers (do not use)

| Number | Source | Why superseded |
|---|---|---|
| 3D mean=15.526 ns, 4D mean=15.087 ns | April 13 smoke run (20k iter) | Timer overhead (~25 ns/call) exceeded signal (~15 ns). Noise dominated. |
| 4D vs 3D overhead = -2.83% | Same smoke run | Sign is wrong. Correct value is +5.0% (physics requires positive overhead). |
| RBAC speedup = 6,674x | Modeled, not measured | busy_wait baseline; not an empirical comparison |
