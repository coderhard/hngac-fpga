---
title: "IPCCC 2026 — Abstract, Revision 3"
subtitle: "H-NGAC: provenance-aware authorization on FPGA. Adds the strongest available security-effectiveness result."
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

# Abstract (227 words)

Real-time robotic systems often leave authorization out of the control path because no software policy engine guarantees a decision within a fixed cycle budget. The tail comes from operating system contention, not policy evaluation, so a faster engine does not tighten it.

We present H-NGAC, an authorization primitive that compiles NGAC policy hypergraphs into fixed-width bitmasks, reducing a decision to a chain of bitwise AND operations in reconfigurable logic. Beyond the subject-object-attribute test it enforces system state and command provenance, closing two CVE-anchored attack classes that identity-based authorization admits: safety-interlock bypass, and injection from nodes holding valid DDS credentials but an unauthorized source type.

Synthesized with Vitis HLS to a Zynq-7020 at 100 MHz, the four- and five-dimensional kernels resolve in an identical number of cycles at every policy size tested, with minimum, mean and maximum equal at 12 + n/2 cycles for n rules. The fifth dimension costs 524 LUTs and no BRAM or DSP; in software it raises per-rule cost from 0.645 to 1.220 cycles. RBAC and NGAC graph baselines resolve faster only by admitting every state-violating request in our corpus; the kernel blocks 18,878 ROS 2 injection attempts with no false positives and verifies functionally on PYNQ-Z1 silicon.

A security dimension is therefore free in time and nearly free in area, which makes the authorization worst case a synthesis parameter rather than a measurement.

**If the submission form enforces a hard 220-word cap,** delete "and verifies functionally on PYNQ-Z1 silicon" (6 words) and "tested" (1 word). That lands at exactly 220 and loses no claim, since the co-simulation result already establishes the hardware measurement and min equals mean equals max already establishes determinism.

# Keywords

Access control, NGAC, FPGA, high-level synthesis, real-time systems, robotics, ROS 2, worst-case execution time, provenance, cyber-physical security

# The threat model, and what is measured against it

This exists and is CVE-anchored. It lives in `docs/canonical-context.md` (attack class definitions) and `docs/evidence-record-2026-04-18.md` (evidence blocks 3 and 5). The abstract compresses it to one clause, per the corpus convention of omitting complete threat-model definitions, but the manuscript should carry the full table.

| Class | Dimension that closes it | What it does | CVE anchor | Measured evidence |
|---|---|---|---|---|
| Timing-window bypass | Bounded latency, not a dimension | Authorization decision arrives after DDS has already delivered the command to the actuator | CVE-2022-45789 (Schneider Modicon session hijack) | 0 slips observed in 8,733 callbacks, baseline and under `stress-ng --cpu 8`. **Null result** — see caveat below |
| Safety-interlock bypass | 4D, system state | Command accepted while the platform is in `battery_low`, `maintenance_mode` or `calibration_required` | CVE-2022-33323 (Mitsubishi MELFA unauthorized command) | RBAC, NGAC-DAG and 3D admit 200,000 of 200,000 requests, including all 100,000 state-violating ones |
| Command injection from a compromised authenticated node | 5D, command provenance | Node holds valid DDS credentials for an authorized subject but is not an entitled source type | CVE-2021-38425 (eProsima Fast DDS RTPS injection) | 18,878 injections blocked, 100%, against 17,059 legitimate commands passed, 0% false positive |

**Caveat on class 1.** The measured slip rate is zero in both conditions, so this is a null result and cannot be cited as a demonstrated exploit. `docs/evidence-record-2026-04-18.md` is explicit: do not claim a measured slip rate above zero. The argument for class 1 is architectural, that a synthesis-time latency bound closes the window by construction, and it does not belong in the abstract as an effectiveness number.

**Docs inconsistency to fix before the manuscript.** `docs/project-overview.md` line 98 labels the timing-window class "Attack Class 1 (4D defense)," but `docs/canonical-context.md` anchors CVE-2022-45789 to the 3D section, and evidence block 5 measures it against the 5D gatekeeper. The defense is really the latency bound rather than any one dimension. Pick one story before a co-author or reviewer reads both files.

# What changed in revision 3

Revision 2 carried one security-effectiveness result. The project has two more, and one of them is stronger than what was in the abstract.

**Added: the over-authorization result.** RBAC, NGAC-DAG and 3D allow 200,000 of 200,000 requests in the April 18 corpus, including all 100,000 that violate state. This is the answer to the obvious reviewer question, which is why not just use RBAC when it resolves in about 100 cycles. Because it says yes to everything. That single sentence converts the comparison table from an argument against the paper into an argument for it, and revision 2 left it out.

**Added: the threat model as a clause.** "Two CVE-anchored attack classes that identity-based authorization admits" costs four words and signals that a real threat model exists behind the abstract. The corpus omits full threat-model definitions, so the enumeration belongs in the manuscript, not here.

**Cost of the additions.** Revision 2 was 214 words. Revision 3 is 227, seven over the 220 guideline. The cut list above gets it back to 220 if a hard cap applies.

**Still deliberately excluded: the zero-slip result.** It is a null result. Including it would invite a reviewer to ask what the slip rate was, and the honest answer, zero, undercuts the sentence it appears in.

# What changed in revision 2, from the original draft

Five edits. The first four are corpus-conformance; the fifth is a content gap.

**1. Causality was inverted.** The draft read "software policy engines create a tail latency that introduces operating system contention." It runs the other way: OS scheduling, preemption and cache effects produce the tail. The draft's own next clause, "regardless of policy decision speed," already implied the correct direction, so the sentence argued against itself. A reviewer who works on real-time systems catches this in the first ten seconds.

**2. "Neglect" became "leave out of the control path."** Neglect reads as carelessness on the part of robotics engineers. Leaving authorization out of the control path is a deliberate and defensible engineering decision given an unbounded checker, which makes the gap real rather than a failure of diligence. It also sets up the conclusion: once the worst case is a synthesis parameter, the decision changes.

**3. "NIST NGAC-like (Next Generation Access Control)" became "NGAC."** The parenthetical expansion costs words in a tight budget, and "NGAC-like" hedges away the standard's authority in the one sentence that should be borrowing it. The hedge is worth keeping, but in the introduction where there is room to say which parts of INCITS 565 are extended.

**4. The closing sentence was replaced.** "We explore how synthesizing those decisions in reconfigurable logic impacts performance" is a characterization-paper move: it promises a study and reports no outcome. A performance-oriented defense abstract reports at least one security-effectiveness result and one cost result.

**5. The two added dimensions were missing.** The draft described 3D NGAC compiled to bitmasks, which is the DCAS 2026 contribution, already published. State and provenance are what make this a new paper.

# What this abstract deliberately does not say, and why

Each of these would be caught by a reviewer who opens the synthesis reports.

| Not claimed | Why |
|---|---|
| "Zero hardware cost" | The fifth dimension costs +11.4% LUT. It is free in time, nearly free in area. |
| 3D, 4D and 5D all resolve identically | 3D was never synthesized. Only 4D vs 5D is measured. |
| The FPGA is faster than the CPU | It is not. At 500 rules the CPU wins on mean wall clock by about 19x. The claim is boundedness. |
| A measured timing-window slip rate | Zero slips were observed. The class 1 argument is architectural, not empirical. |
| The board test is a timing result | 2,307 requests PASS on PYNQ-Z1 is functional verification only, which is why the abstract says "verifies functionally." |
| UltraScale+ | The part is a Zynq-7020. Earlier drafts said UltraScale+ aspirationally. |
| A hardware speedup number | Any speedup framing invites the wall-clock comparison. Determinism is the defensible ground. |

# Levers

**If you need words back.** In order of least damage: "and verifies functionally on PYNQ-Z1 silicon" plus "tested" (7 words, lands at 220, no claim lost); the software contrast clause 0.645 to 1.220 (14 words, but it is what makes "free" mean anything); the CVE-anchored clause (4 words, but it is the only signal in the abstract that a threat model exists).

**If you have room.** The OPA comparison from the withdrawn TS-NGAC work (OPA mean 271.491 microseconds, 13 deadline misses) preempts the "why not just use OPA" question, and a sentence placing this in the hypergraph privilege-analysis line from BigData 2025 credits where the approach originated.

**Open decision.** Whether to fold the TS-NGAC OPA and XACML baselines into IPCCC depends on the contribution split with the journal version. Worth settling before the manuscript rather than the abstract.

# The draft this revises

Kept for comparison.

> Real-time robotic systems often neglect software-based authorization controls because prevailing controls don't offer decisions within a pre-determined cycle time. software policy engines create a tail latency that introduces operating system contention, regardless of policy decision speed. In this article we present H-NGAC, an authorization method that compiles NIST NGAC-like (Next Generation Access Control) policy hypergraphs into fixed-width bitmasks. Our method enables hardware-based real-time decisions via a chain of bitwise AND operations. We explore how synthesizing those decisions in reconfigurable logic impacts performance.
