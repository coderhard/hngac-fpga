---
title: "IPCCC 2026 — Abstract, Revision 2"
subtitle: "H-NGAC: provenance-aware authorization on FPGA. Revised against the hardware-security abstract pattern."
date: "7 August 2026"
---

# Title options

Pick one, or mix. Option 1 leads with the finding, which is the strongest move given the result is genuinely surprising.

1. **Security Dimensionality at Zero Time Cost: A Provenance-Aware NGAC Authorization Primitive on FPGA**
2. **Three Attack Classes for the Area Cost of One: Hardware-Accelerated 5D NGAC Authorization for Real-Time Robotics**
3. **Bounded Authorization for Real-Time Robotic Systems: State- and Provenance-Aware NGAC in Reconfigurable Logic**

# Authors

Hassan Karim, Omar Faruque, Abdel-Hameed A. Badawy, Sai Sitharaman, Deepti Gupta

| Author | Affiliation |
|---|---|
| Hassan Karim | Stable Cyber LLC |
| Omar Faruque | Florida International University |
| Abdel-Hameed A. Badawy | Florida International University |
| Sai Sitharaman | [confirm] |
| Deepti Gupta | [confirm] |

> Check how Badawy renders his name on recent papers (Abdel-Hameed A. Badawy vs Abdel-Hameed Badawy) and confirm the two remaining affiliations before submitting.

# Abstract (214 words)

Real-time robotic systems often leave authorization out of the control path because no software policy engine guarantees a decision within a fixed cycle budget. The tail comes from operating system contention rather than policy evaluation, so a faster engine does not tighten it.

We present H-NGAC, an authorization primitive that compiles NGAC policy hypergraphs into fixed-width bitmasks, reducing a decision to a chain of bitwise AND operations in reconfigurable logic. Beyond the subject-object-attribute test it enforces system state, blocking safety-interlock bypass, and command provenance, blocking injection from authenticated but compromised nodes that DDS-Security and SROS2 admit at the transport layer.

Synthesized with Vitis HLS to a Zynq-7020 at 100 MHz, the four-dimensional and five-dimensional kernels resolve in an identical number of clock cycles at every policy size tested, at the same initiation interval, with minimum, mean and maximum latency equal at 12 + n/2 cycles for n rules. The fifth dimension costs 524 additional LUTs and no BRAM or DSP; in software it raises per-rule cost from 0.645 to 1.220 cycles. The kernel verifies functionally on PYNQ-Z1 silicon and blocks 18,878 ROS 2 injection attempts with no false positives.

A security dimension is therefore free in time and nearly free in area, which makes the authorization worst case a synthesis parameter rather than a measurement.

# Keywords

Access control, NGAC, FPGA, high-level synthesis, real-time systems, robotics, ROS 2, worst-case execution time, provenance, cyber-physical security

# How this maps to the hardware-security abstract pattern

The corpus follows a defense-paper sequence: threat and insufficiency, named defense, essential mechanism, security coverage, then performance and area cost. The word budget is allocated the same way.

| Move | Target words | Actual | Sentences |
|---|---|---|---|
| System, threat, gap | 35-45 | 43 | Both sentences of paragraph 1 |
| Proposed mechanism | 55-70 | 57 | Paragraph 2 |
| Evaluation and principal results | 60-80 | 88 | Paragraph 3 |
| Bounded implication | 15-25 | 26 | Paragraph 4 |
| **Total** | **180-220** | **214** | |

Paragraph 3 runs eight words long because the pattern requires both a security-effectiveness result and a cost result, and this paper has two of each (cycles and LUTs on the cost side, silicon verification and injection blocking on the security side). If a hard 200-word cap appears, cut "at the same initiation interval" and "tested," which costs seven words and no claim.

# What changed from the draft, and why

Five edits. The first four are corpus-conformance; the fifth is a content gap.

**1. Causality was inverted.** The draft read "software policy engines create a tail latency that introduces operating system contention." It runs the other way: OS scheduling, preemption and cache effects produce the tail. The draft's own next clause, "regardless of policy decision speed," already implies the correct direction, so the sentence argued against itself. A reviewer who works on real-time systems catches this in the first ten seconds. Now: "The tail comes from operating system contention rather than policy evaluation, so a faster engine does not tighten it."

**2. "Neglect" became "leave out of the control path."** Neglect reads as carelessness on the part of robotics engineers. Leaving authorization out of the control path is a deliberate and defensible engineering decision given an unbounded checker, which makes the gap real rather than a failure of diligence. It also sets up the conclusion: once the worst case is a synthesis parameter, the decision changes.

**3. "NIST NGAC-like (Next Generation Access Control)" became "NGAC."** Two reasons. The parenthetical expansion costs words in a 200-word budget, and IPCCC reviewers know the term. More importantly, "NGAC-like" hedges away the standard's authority in the one sentence that should be borrowing it. The hedge is worth keeping, but it belongs in the introduction where there is room to say precisely which parts of INCITS 565 are extended and which are not.

**4. The closing sentence was replaced.** "We explore how synthesizing those decisions in reconfigurable logic impacts performance" is a characterization-paper move: it promises a study and reports no outcome. In this corpus a performance-oriented defense paper reports at least one security-effectiveness result and one cost result. The replacement paragraph supplies both, and the final sentence gives the bounded implication the pattern ends on.

**5. The two added dimensions were missing.** The draft describes 3D NGAC compiled to bitmasks, which is the DCAS 2026 contribution, already published. State and provenance are what make this a new paper, and provenance is the dimension nothing else in the ROS 2 stack covers. Without them a reviewer who knows the DCAS poster sees a resubmission.

# What this abstract deliberately does not say, and why

Each of these would be caught by a reviewer who opens the synthesis reports.

| Not claimed | Why |
|---|---|
| "Zero hardware cost" | The fifth dimension costs +11.4% LUT. It is free in time, nearly free in area. |
| 3D, 4D and 5D all resolve identically | 3D was never synthesized. Only 4D vs 5D is measured. |
| The FPGA is faster than the CPU | It is not. At 500 rules the CPU wins on mean wall clock by about 19x. The claim is boundedness. |
| The board test is a timing result | 2,307 requests PASS on PYNQ-Z1 is functional verification only. The abstract says "verifies functionally" for exactly this reason. |
| UltraScale+ | The part is a Zynq-7020. Earlier drafts said UltraScale+ aspirationally. |
| A hardware speedup number | Any speedup framing invites the wall-clock comparison. Determinism is the defensible ground. |

# Levers

**If you need words back.** In order of least damage: "tested" and "at the same initiation interval" (7 words, no claim lost, since min equals mean equals max already establishes pipelined determinism); the software contrast clause (0.645 to 1.220), which costs 14 words but is what makes "free" mean anything; the silicon verification clause, which costs 9 words but is the only real-hardware credibility in the abstract.

**If you have room.** Two additions would strengthen it. First, the OPA comparison from the withdrawn TS-NGAC work (OPA mean 271.491 microseconds, 13 deadline misses) preempts the "why not just use OPA" question. Second, a sentence placing this in the hypergraph privilege-analysis line that began with the BigData 2025 paper, which is where the approach originated.

**Open decision.** Whether to fold the TS-NGAC OPA and XACML baselines into IPCCC depends on the contribution split with the journal version. Worth settling before the manuscript rather than the abstract.

# The draft this revises

Kept for comparison.

> Real-time robotic systems often neglect software-based authorization controls because prevailing controls don't offer decisions within a pre-determined cycle time. software policy engines create a tail latency that introduces operating system contention, regardless of policy decision speed. In this article we present H-NGAC, an authorization method that compiles NIST NGAC-like (Next Generation Access Control) policy hypergraphs into fixed-width bitmasks. Our method enables hardware-based real-time decisions via a chain of bitwise AND operations. We explore how synthesizing those decisions in reconfigurable logic impacts performance.
