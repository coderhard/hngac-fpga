# Gemini Deep Research prompt — related-work verification for IPCCC 2026

Paste everything below the line into Gemini Deep Research. It is self-contained;
Gemini has no access to our repo.

---

## PROMPT

I am finalizing an IEEE IPCCC 2026 submission and need a comprehensive,
adversarial related-work investigation. A four-way literature sweep has already
been run; your job is to go BEYOND it: find what it missed, and in particular
find anything that could sink the paper's novelty claim. Treat this as a
prior-art search a hostile reviewer would run.

### The paper under review

Title: "Security Dimensionality at Zero Time Cost: A Provenance-Aware NGAC
Authorization Primitive on FPGA." IEEE IPCCC 2026 submission.

The system, 5D H-NGAC, compiles NGAC (INCITS 565 / NIST Policy Machine) policy
graphs into fixed-width bitmasks so one authorization decision reduces to a
chain of bitwise AND operations, then synthesizes the evaluator to a Zynq-7020
FPGA with Vitis HLS 2025.2 at 100 MHz. Beyond the subject-object-attribute
identity triple it enforces two more dimensions: runtime system state (blocks
safety-interlock bypass) and command provenance, meaning source type (blocks
injection from nodes holding valid DDS credentials but not entitled to issue
that command). Measured results: the 4-dimensional and 5-dimensional kernels
resolve in an IDENTICAL number of clock cycles at every policy size tested
(4 to 500 rules), latency is closed-form at 12 + n/2 cycles for n rules with
min = mean = max (zero jitter by construction: fixed-trip-count pipelined scan,
initiation interval 1, no early exit), and the fifth dimension costs +524 LUTs
(+11.4%) with zero BRAM and zero DSP. In a live ROS 2 evaluation the
5-dimensional policy blocked 18,878 of 18,878 injection attempts from a
credentialed compromised node with 0 false positives in 17,059 legitimate
commands. In software the same added dimensions raise per-rule cost from 0.645
to 1.220 cycles, so the claim is: security dimensionality is free in time and
nearly free in area in hardware, which software cannot match.

### Our own prior work (treat as OURS, not competing prior art)

1. H. Karim, S. Sitharaman, D. Gupta, "Hardware-Accelerated NGAC Authorization
   for Real-Time Multi-Robot Systems," IEEE DCAS 2026, pp. 1-4.
   DOI 10.1109/dcas69364.2026.11544855. (The 3D H-NGAC software baseline this
   paper extends.)
2. S. Sitharaman, H. Karim, D. Gupta, M. Tyagi, "Scalable Privilege Analysis
   for Multi-Cloud Big Data Platforms: A Hypergraph Approach," IEEE BigData
   2025, pp. 6626-6633. DOI 10.1109/bigdata66926.2025.11401728.
3. H. Karim, D. Gupta, S. Sitharaman, "Securing LLM Workloads with NIST AI RMF
   in the Internet of Robotic Things," IEEE Access, 2025.
   DOI 10.1109/access.2025.3561235.
4. H. Karim, S. Sitharaman, D. Gupta, D. B. Rawat, "Securing Autonomous
   Clinical Agents: Time-Scoped Hypergraph Delegation for Controlling Patient
   Data Access," presented at IEEE ICDH 2026, Sydney.
5. Karim, Gupta, Sitharaman, "Deterministic Time-Scoped NGAC for Real-Time
   Multi-Robot Systems" (under review; do not search for this, it is
   unpublished).

Also part of the team: Omar Faruque (Florida International University), the
hardware lead who performed the Vitis HLS synthesis, RTL co-simulation, and
PYNQ-Z1 board verification in this paper. His Google Scholar profile is
https://scholar.google.com/citations?hl=en&user=Da0o4CIAAAAJ — review his FPGA
and hardware publications, include the ones relevant to hardware
acceleration/security as part of the team's prior line, and check whether any
of his published work should be cited in this paper's related work or
methodology sections. Likewise Abdel-Hameed A. Badawy (FIU,
https://scholar.google.com/citations?hl=en&user=YfUn3GEAAAAJ), whose
architecture/performance-modeling work may bear on the hardware evaluation
methodology.

### What the existing sweep already found (do NOT just re-report these)

Closest hardware ancestors: Huffmire et al., "Policy-Driven Memory Protection
for Reconfigurable Hardware" (ESORICS 2006) and "Designing Secure Systems on
Reconfigurable Hardware" (TODAES 2008); Fiorin et al., "Secure Memory Accesses
on Networks-on-Chip" (IEEE TC 2008); Coburn et al. SECA (CASES 2005); Cotret
et al. (FCCM 2012); Restuccia et al. Aker (ICCAD 2021); CHERI (S&P 2015);
GLIFT (ASPLOS 2009); PHMon (USENIX Sec 2020). Packet classification:
Song & Lockwood BV-TCAM (FPGA '05); Ganegedara/Prasanna StrideBV line
(HPSR 2012, TPDS 2014, IPDPSW 2013); Jedhe et al. FPGA firewall (FCCM 2008).
Policy compilation in software: Liu et al. XEngine (SIGMETRICS 2008 / TC 2011);
Pina Ros et al. graph-based XACML (SACMAT 2012); Cisco patent US 8,677,453
(bit-vector XACML). NGAC/ABAC: NIST IR 7987r1; SP 800-162; Ferraiolo et al.
XACML-vs-NGAC (ABAC '16); ABAC-alpha (DBSec 2012); HGABAC (FPS 2014);
Servos/Osborn survey (CSUR 2017); Mell et al. linear-time NGAC (JoWUA 2017).
Real-time security: Xie/Qin (TECS 2007); Zimmer et al. (ICCPS 2010);
SecureCore (RTAS 2013); Mohan et al. (ECRTS 2014); Contego (ECRTS 2017);
Lesi et al. (TECS/EMSOFT 2017); Pinisetty et al. runtime enforcement
(TECS 2017); RECFISH (ECRTS 2019); RT-TEE (S&P 2022); Hasan et al. SoK
(CSUR 2024). ROS/DDS security: Dieber et al. (IROS 2016, RAS 2017); White
et al. (IROS 2018, EuroS&PW 2019); DeMarinis et al. (ICRA 2019); ROSploit and
ROS-Defender (2019); Deng et al. "On the (In)Security of Secure ROS2"
(CCS 2022); Wang et al. formal DDS analysis (ASIA CCS 2024); ROSPaCe dataset
(Sci. Data 2024); Salimi et al. ABAC for multi-robot systems (JSA 2025);
Trend Micro DDS white paper (2022).

### The kill-shot questions (highest priority, answer each explicitly)

1. Does ANY published work implement NGAC (Policy Machine, INCITS 565) in
   hardware, FPGA, ASIC, SmartNIC, or programmable dataplane? Our sweep found
   none; verify or refute.
2. Does any work demonstrate that ADDING policy dimensions/attributes to a
   hardware policy evaluator costs zero additional clock cycles, or make any
   equivalent "richer policy is free in time" claim with measurements?
3. Does any work give a closed-form cycle count for authorization latency as a
   function of policy size on hardware?
4. Does any robot/CPS system enforce per-command application-layer
   authorization conditioned on command SOURCE TYPE (provenance), or on
   runtime platform safety state? Check industrial robot controllers, safety
   PLCs (IEC 61508/ISO 10218 interlock implementations), automotive gateway
   firewalls (AUTOSAR SecOC), avionics MILS/ARINC 653 partitioning, and
   medical device standards, not just academic robotics.
5. Is there hardware ABAC/RBAC/policy-engine work in venues our sweep
   underweighted: FPL, FPT, ReConFig, DATE, ASP-DAC, HOST, CHES, TrustED,
   IEEE TIFS, IEEE TDSC, ACM TECS, Chinese and European venues, arXiv
   2023-2026, patents beyond US 8,677,453, PhD dissertations, and vendor
   material (AMD/Xilinx app notes, Intel FPGA security IP, Arm TrustZone
   documentation)?
6. Any TCAM-based ACCESS CONTROL (not packet classification) engines, P4 or
   SmartNIC policy offload work, or zero-trust hardware enforcement points
   that evaluate multi-attribute policy at line rate?
7. Recent 2024-2026 work on hardware acceleration of authorization for IoT,
   edge, or robotic systems that our sweep may have missed due to recency.

### Output format

- Organize by the seven questions above, then a category for anything else
  significant.
- For every reference: authors, exact title, venue, year, pages, DOI (state
  explicitly if you could not verify the DOI), one line on what it does, one
  line on how it relates to our claim, and a THREAT LEVEL to our novelty claim:
  NONE / ADJACENT / DIRECT.
- End with (a) a ranked list of the five closest prior works overall,
  (b) an explicit verdict on each of the seven kill-shot questions, and
  (c) BibTeX entries for everything rated ADJACENT or DIRECT.
- Do not fabricate references. If a question comes up empty after genuine
  search effort, say so explicitly; a verified empty result is valuable to us.
