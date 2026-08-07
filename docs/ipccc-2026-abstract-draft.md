---
title: "IPCCC 2026 — Abstract Draft"
subtitle: "H-NGAC: provenance-aware authorization on FPGA. For author review and revision."
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

# Abstract (254 words)

Real-time robotic systems need authorization decisions with a bounded worst case, but software policy engines cannot offer one: their tail latency is governed by the operating system, not by the policy. We present H-NGAC, an authorization primitive that compiles NGAC policy hypergraphs into fixed-width bitmasks so that a decision reduces to a chain of bitwise AND operations, and we synthesize it to reconfigurable logic. Beyond the conventional subject-object-attribute test we add two dimensions: system state, which blocks safety-interlock bypass, and command provenance, which blocks injection from compromised but authenticated nodes. That last class is invisible to DDS-Security and SROS2, which authenticate node identity at the transport layer but never ask whether a given source type is entitled to issue a given command.

Our central result is that security dimensionality is free in time on an FPGA. Synthesized with Vitis HLS to a Zynq-7020 at 100 MHz, the four- and five-dimensional kernels resolve in an identical number of clock cycles at every policy size measured, with identical initiation interval and timing slack, and with minimum, mean and maximum latency exactly equal. Latency is closed-form at 12 + n/2 cycles for n rules, with zero jitter. The fifth dimension costs 524 additional LUTs and no BRAM or DSP, leaving the primitive at 9% of a low-cost part. In software the same dimension is not free: per-rule cost rises from 0.645 to 1.220 cycles as dimensions are added. We verify 2,307 decisions on PYNQ-Z1 silicon and block 18,878 injection attempts in ROS 2 with no false positives.

# Keywords

Access control, NGAC, FPGA, high-level synthesis, real-time systems, robotics, ROS 2, worst-case execution time, provenance, cyber-physical security

# Notes for your revision

**What this abstract deliberately does not say, and why.** Each of these would be caught by a reviewer who opens the synthesis reports.

| Not claimed | Why |
|---|---|
| "Zero hardware cost" | The fifth dimension costs +11.4% LUT. It is free in time, nearly free in area. |
| 3D, 4D and 5D all resolve identically | 3D was never synthesized. Only 4D vs 5D is measured. |
| The FPGA is faster than the CPU | It is not. At 500 rules the CPU wins on mean wall clock by about 19x. The claim is boundedness. |
| UltraScale+ | The part is a Zynq-7020. Earlier drafts said UltraScale+ aspirationally. |
| A hardware speedup number | Any speedup framing invites the wall-clock comparison. Determinism is the defensible ground. |

**Levers if you need to cut words.** The DDS-Security and SROS2 sentence is the most compressible; it can drop to a clause. The software comparison sentence (0.645 to 1.220) can go if you are tight, though it is what makes "free" meaningful by contrast.

**Levers if you have room.** Two additions would strengthen it. First, the OPA comparison from the withdrawn TS-NGAC work (OPA mean 271.491 microseconds, 13 deadline misses) preempts the "why not just use OPA" question. Second, a sentence placing this in the hypergraph privilege-analysis line that began with the BigData 2025 paper, which is where the approach originated.

**Open decision.** Whether to fold the TS-NGAC OPA and XACML baselines into IPCCC depends on the contribution split with the journal version. Worth settling before the manuscript rather than the abstract.
