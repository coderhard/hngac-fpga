---
title: "IPCCC 2026 — Abstract, Revision 6"
subtitle: "5D H-NGAC: provenance-aware authorization on FPGA. Rewritten to the withholding register per author direction; matches the manuscript verbatim."
date: "7 August 2026"
---

# Title options

**Use option 1.** Since hardware belongs to DCAS and hypergraph belongs to BigData, dimensionality is this paper's only unclaimed contribution word, and option 1 is the only title that leads with it.

1. **Security Dimensionality at Zero Time Cost: A Provenance-Aware NGAC Authorization Primitive on FPGA** ✅
2. ~~Three Attack Classes for the Area Cost of One: Hardware-Accelerated 5D NGAC Authorization for Real-Time Robotics~~ — **do not use.** Two problems. "Three attack classes for the area of one" asserts a three-way claim that 3D synthesis does not support, since 3D was never synthesized. And "Hardware-Accelerated" is almost verbatim the DCAS title, which invites the reviewer to ask what is new.
3. **Bounded Authorization for Real-Time Robotic Systems: State- and Provenance-Aware NGAC in Reconfigurable Logic** — safe fallback. Accurate, but leads with boundedness, which DCAS already argued, rather than with the new result.

# Authors

Hassan Karim, Omar Faruque, Abdel-Hameed A. Badawy, Sai Sitharaman, Deepti Gupta

| Author | Affiliation |
|---|---|
| Hassan Karim | Stable Cyber LLC |
| Omar Faruque | Florida International University |
| Abdel-Hameed A. Badawy | Florida International University |
| Sai Sitharaman | Zetafence, Inc. |
| Deepti Gupta | Texas A&M University–Central Texas |

> Author block final 2026-08-07. Affiliations from the ac4aiagents Overleaf bios (Sitharaman: Founder and CTO, Zetafence, Inc., Dublin, CA; Gupta: Assistant Professor, Texas A&M University–Central Texas). Badawy rendering confirmed from his Google Scholar profile (verified fiu.edu email): "Abdel-Hameed A. Badawy".

# Abstract (198 words — Revision 6, matches `paper/main.tex` verbatim)

Real-time robotic systems often leave authorization out of the control path because no software policy engine guarantees a decision within a fixed cycle budget. The tail comes from operating system scheduling, not policy evaluation, so a faster engine does not tighten it. The gap leaves attack classes open that identity-based authorization cannot see, among them unsafe-state operation and command injection from compromised but credentialed nodes, both anchored to published CVEs. H-NGAC compiles NGAC policy graphs into fixed-width bitmasks so that an authorization decision reduces to a chain of bitwise AND operations. In this paper we extend H-NGAC with two further dimensions, runtime system state and command provenance, and synthesize the four- and five-dimensional kernels to a Zynq-7020 FPGA. We evaluate the synthesized kernels against six software baselines and in an adversarial ROS 2 scenario against a credentialed compromised node. The added security dimension is free in time: both kernels resolve in an identical number of clock cycles at every policy size tested, with a closed-form worst case and zero jitter by construction. Thus the authorization worst case becomes a synthesis parameter rather than a measurement, and we quantify what that determinism costs in area and in average-case speed.

**Revision 6 note (author direction, 2026-08-07 evening).** Revisions 1 through 5 enumerated the results: the closed form, the LUT delta, the block counts, the software slope. The author's corpus of pre-2023 exemplars (Hinton's "surprising results on MNIST," the TPU paper's selective headline plus determinism emphasis, the FPGA bitstream survey) withholds detail and frames the contribution. Revision 6 keeps exactly one revealed finding, zero-cycle dimensionality with a closed-form jitter-free worst case, and defers every other number to the paper. The final clause deliberately trails the honest concessions (area, average-case speed) as a reason to read Section VI rather than a spoiler of it. The reviewer-2 conflation fix survives by omission: the ROS 2 scenario is named without attributing block counts to any component.

## Superseded revision 5 text (kept for the record)

# ~~Abstract (238 words — Revision 5)~~

Real-time robotic systems often leave authorization out of the control path because no software policy engine guarantees a decision within a fixed cycle budget. The tail comes from operating system contention, not policy evaluation, so a faster engine does not tighten it.

H-NGAC compiles NGAC policy graphs into fixed-width bitmasks, so an authorization decision reduces to a chain of bitwise AND operations. We present 5D H-NGAC, which adds two dimensions and synthesizes the result to reconfigurable logic: system state, closing safety-interlock bypass, and command provenance, closing injection from nodes that hold valid DDS credentials but an unauthorized source type. Both are CVE-anchored classes that identity-based authorization admits.

Synthesized with Vitis HLS to a Zynq-7020 at 100 MHz, the four- and five-dimensional kernels resolve in an identical number of cycles at every policy size tested, with minimum, mean and maximum equal at 12 + n/2 cycles for n rules. The fifth dimension costs 524 additional LUTs and no BRAM or DSP; in software it raises per-rule cost from 0.645 to 1.220 cycles. RBAC and DAG-NGAC baselines resolve faster only by admitting every state-violating request in our corpus; the five-dimensional policy blocks 18,878 ROS 2 injection attempts in a software gatekeeper with no false positives, and the kernel verifies functionally on PYNQ-Z1 silicon.

The added security dimension is therefore free in time and nearly free in area, which makes the authorization worst case a synthesis parameter rather than a measurement.

**Portal update note (2026-08-07 evening).** Revision 4 (229 words, "the kernel blocks") was the version at abstract submission this morning. Revision 5 applies the reviewer-2 fixes: the blocking is credited to the five-dimensional policy in a software gatekeeper, the kernel to silicon functional verification, and the closing generalization is scoped to "the added security dimension." Paste-ready plain text: `docs/ipccc-2026-abstract-portal-text.txt`.

**If the submission form enforces a hard 220-word cap,** delete "and the kernel verifies functionally on PYNQ-Z1 silicon" (8 words), "tested" (1 word), and "Both are CVE-anchored classes that identity-based authorization admits" (9 words, but it is the only signal that a threat model exists). All three land exactly at 220. Do not cut "in a software gatekeeper"; that phrase is the reviewer-2 fix.

## Terminology corrections in this revision

Two errors in revision 3, both introduced by carrying the original draft's wording forward without checking it against `docs/canonical-context.md`.

**"Compiles NGAC policy hypergraphs" was wrong.** `canonical-context.md` says H-NGAC compiles NGAC policy **graphs**. "Hypergraph" is HyperNGAC's framing, from BigData 2025, which is a different system solving a different problem (batch privilege analysis, not per-decision authorization). Using it here merged two research lines that need to stay distinct. Now reads "policy graphs."

**"We present H-NGAC" over-claimed against our own published work.** The H is **Hardware**, and H-NGAC was presented at DCAS 2026. This paper does not present it, it extends it. The abstract now presents **5D H-NGAC**, which names the delta rather than reasserting the base. This matters more than it looks: IPCCC reviewers will have the DCAS paper in front of them because we cite it, and "we present" against a published primitive is the kind of thing that reads as self-plagiarism even when it is only loose wording.

**Consequence for the framing.** Since hardware is DCAS's contribution and hypergraph is BigData's, the contribution word for this paper is **dimensionality**. Title option 1 already leads with it. Options 2 and 3 do not, and option 2 additionally asserts a three-way claim that 3D synthesis does not yet support.

**Also changed:** "NGAC graph baselines" became "DAG-NGAC baselines," giving the INCITS 565 reference model a name so the contrast with H-NGAC is explicit rather than implied.

# Keywords

Access control, NGAC, FPGA, high-level synthesis, real-time systems, robotics, ROS 2, worst-case execution time, provenance, cyber-physical security

# The threat model, and what is measured against it

This exists and is CVE-anchored. It lives in `docs/canonical-context.md` (attack class definitions) and `docs/evidence-record-2026-04-18.md` (evidence blocks 3 and 5). The abstract compresses it to one clause, per the corpus convention of omitting complete threat-model definitions, but the manuscript should carry the full table.

Integer numbering is retired as of 2026-08-07. Use these names.

| Attack class | Closed by | What the attacker does | CVE anchor | Measured evidence |
|---|---|---|---|---|
| **Unauthorized access** | 3D, subject/object/attribute | Acts as an unprivileged agent or a hijacked session | CVE-2022-45789 (Schneider Modicon session hijack) | Covered by the 45-test kernel testbench; no adversarial demo run |
| **Unsafe-state operation** | 4D, system state | Issues a legitimate command while the platform is in `battery_low`, `maintenance_mode` or `calibration_required` | CVE-2022-33323 (Mitsubishi MELFA unauthorized command) | RBAC, DAG-NGAC and 3D admit 200,000 of 200,000 requests, including all 100,000 state-violating ones |
| **Command provenance abuse** | 5D, command provenance | Holds valid DDS credentials for an authorized subject but is not an entitled source type | CVE-2021-38425 (eProsima Fast DDS RTPS injection) | 18,878 injections blocked, 100%, against 17,059 legitimate commands passed, 0% false positive |

**The timing window is not a fourth class.** It is a cross-cutting delivery property: any correct decision is useless if it arrives after the actuator moved, which applies to all three classes above. It is closed by the synthesis-time latency bound, not by a dimension. Measured slip rate is **0 in 8,733 callbacks**, at baseline and under `stress-ng --cpu 8`. That is a null result, so the argument is architectural and it stays out of the abstract as an effectiveness number.

**Docs inconsistency, resolved 2026-08-07.** `project-overview.md` had labelled the timing window "Attack Class 1 (4D defense)," which was wrong twice: it is not a peer class, and the state dimension has nothing to do with it. Separately, CVE-2022-45789 was doing double duty, anchoring unauthorized access in `canonical-context.md` and the timing window in `evidence-record-2026-04-18.md`. It is a session hijack, so it belongs to unauthorized access only; the timing window has no CVE anchor and needs none. All three files now agree.

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
