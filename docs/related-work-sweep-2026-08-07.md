# Related-work sweep results — 2026-08-07

Task 2 of `docs/manuscript-agent-prompt.md`. Four independent agents, no shared
context. Raw results land here as each finishes; integration into `paper/refs.bib`
and the Section VII rubric happens after all four report.

Agent instructions: 8–12 references each, verified online this session, DOIs only
when actually seen, anticipation flag for anything that got to
"dimensionality/policy is free in time in hardware" first.

---

## Sweep 3 of 4 — WCET and real-time security enforcement in CPS (COMPLETE)

12 primary + 2 spares, all verified via publisher pages, dblp, DROPS, or proceedings.

### ⚠ ANTICIPATION FLAGS — two, both 2008, both at a different abstraction level

1. **Huffmire, Brotherton, Callegari, Valamehr, White, Kastner, Sherwood,
   "Designing Secure Systems on Reconfigurable Hardware,"** ACM TODAES 13(3),
   Art. 44, 2008. DOI 10.1145/1367045.1367053. (Journal treatment of
   Moats-and-Drawbridges, IEEE S&P 2007.)
   Compiles a memory-access policy, written in a specialized language, directly
   into an FPGA reference-monitor circuit. **Ancestor of "policy compiled to
   hardware enforcement."** No closed-form latency-vs-policy-size expression, no
   WCET-in-control-loop framing, memory ranges not a policy model.

2. **Fiorin, Palermo, Lukovic, Catalano, Silvano, "Secure Memory Accesses on
   Networks-on-Chip,"** IEEE Trans. Computers 57(9), pp. 1216–1229, 2008.
   DOI 10.1109/TC.2008.69.
   Data Protection Units at NoC interfaces check access rights in hardware;
   authorized requests incur no added network latency. **Anticipates the
   zero-added-latency move at the interconnect level.** Flat rights tables, not a
   multi-dimensional policy model; no jitter/WCET argument.

**Consequence for the novelty claim (agent's analysis, verified reasoning):** do
NOT claim "first hardware access control." Cite Huffmire and Fiorin as ancestors
and differentiate on three axes nobody covers: (a) policy-model-level
(NGAC-class, multi-dimensional) authorization in fabric, (b) closed-form cycle
count as an explicit function of policy size (12 + n/2), (c) decision latency as
a WCET term inside a control-loop deadline, with zero jitter by construction
against measured software tails.

### The scheduling foil (dominant paradigm the paper argues against)

- Xie, Qin, "Improving Security for Periodic Tasks in Embedded Systems through
  Scheduling," ACM TECS 6(3), Art. 20, 2007. DOI 10.1145/1275986.1275992.
  Security as variable-quality service traded against deadlines.
- Zimmer, Bhat, Mueller, Mohan, "Time-Based Intrusion Detection in
  Cyber-Physical Systems," ICCPS 2010, pp. 109–118. DOI 10.1145/1795194.1795210.
  WCET as a detection sensor; we invert it.
- Yoon, Mohan, Choi, Kim, Sha, "SecureCore: A Multicore-based Intrusion
  Detection Architecture for Real-Time Embedded Systems," RTAS 2013, pp. 21–32.
  DOI 10.1109/RTAS.2013.6531076. Security beside the path; ours in-path.
- Mohan, Yoon, Pellizzoni, Bobba, "Real-Time Systems Security through Scheduler
  Constraints," ECRTS 2014, pp. 129–140. DOI 10.1109/ECRTS.2014.28.
- Hasan, Mohan, Pellizzoni, Bobba, "Contego: An Adaptive Framework for
  Integrating Security Tasks in Real-Time Systems," ECRTS 2017, LIPIcs 76,
  23:1–23:22. DOI 10.4230/LIPIcs.ECRTS.2017.23. Canonical
  security-fights-for-slack formulation.
- Lesi, Jovanov, Pajic, "Security-Aware Scheduling of Embedded Control Tasks,"
  ACM TECS 16(5s), Art. 188, 2017 (EMSOFT Best Paper). DOI 10.1145/3126518.
  Closest control-loop framing; enforcement still schedulable software.
- Walls, Brown, Le Baron, Shue, Okhravi, Ward, "Control-Flow Integrity for
  Real-Time Embedded Systems" (RECFISH), ECRTS 2019, LIPIcs 133, 2:1–2:24.
  DOI 10.4230/LIPIcs.ECRTS.2019.2. Measure overhead, re-run schedulability.
- Wang, Li, Li, Lu, Zhang, "RT-TEE: Real-time System Availability for
  Cyber-physical Systems using ARM TrustZone," IEEE S&P 2022, pp. 352–369.
  DOI 10.1109/SP46214.2022.9833604. Hardened scheduling; we remove the scheduler.
- Hasan, Kashinath, Chen, Mohan, "SoK: Security in Real-Time Systems," ACM
  Computing Surveys 56(9), Art. 218, 2024. DOI 10.1145/3649499. Framing citation:
  the field's dominant paradigm schedules security around tasks.

### Handle carefully

- Pinisetty, Roop, Smyth, Allen, Tripakis, von Hanxleden, "Runtime Enforcement
  of Cyber-Physical Systems," ACM TECS 16(5s), Art. 178, 2017.
  DOI 10.1145/3126500. Also synthesizes enforcers from formal policy, but for
  safety timed automata, not authorization, and with no hardware latency model.
  Borderline flag: synthesis yes, latency-as-parameter no.

### Verified spares

- Pellizzoni, Paryab, Yoon, Bak, Mohan, Bobba, "A Generalized Model for
  Preventing Information Leakage in Hard Real-Time Systems," RTAS 2015,
  pp. 271–282. DOI 10.1109/RTAS.2015.7108450.
- Hasan, Mohan, Pellizzoni, Bobba, "A Design-Space Exploration for Allocating
  Security Tasks in Multicore Real-Time Systems" (HYDRA), DATE 2018,
  pp. 225–230. DOI 10.23919/DATE.2018.8342007.

---

## Sweep 1 of 4 — FPGA access control and security primitives (COMPLETE)

12 verified + 1 reserve, via Crossref, ACM DL, IEEE Xplore, Springer.

### ⚠ Anticipation analysis — nothing states our claim; two adjacent bodies need head-on citation

- **TCAM folklore.** Song & Lockwood, "Efficient Packet Classification for Network
  Intrusion Detection Using FPGA" (BV-TCAM), FPGA '05, pp. 238–245,
  DOI 10.1145/1046192.1046223. Wide multi-field keys match in a single cycle by
  construction, paid in TCAM area/power. "Wider key, same cycle count" is folklore
  at the circuit-primitive level; never framed as access-control dimensionality.
- **StrideBV line — deterministic latency, but dimensions NOT free.** Ganegedara &
  Prasanna, StrideBV, HPSR 2012, DOI 10.1109/HPSR.2012.6260820; Ganegedara, Jiang,
  Prasanna, IEEE TPDS 25, pp. 1135–1144, 2014, DOI 10.1109/TPDS.2013.261.
  Explicitly ruleset-independent deterministic latency on FPGA (anticipates our
  zero-jitter framing), BUT latency grows linearly with match-key width, i.e.
  added dimensions cost cycles there. Exactly the axis we differ on. Cite head-on.
- Huffmire ESORICS 2006 (policy-to-circuit) and Fiorin TC 2008 re-confirmed
  independently by this agent (also flagged by sweeps 2 and 3).

### Other verified entries

- Huffmire et al., "Moats and Drawbridges," IEEE S&P 2007, pp. 281–295,
  DOI 10.1109/SP.2007.28 (isolation primitives, not decision latency).
- Coburn, Ravi, Raghunathan, Chakradhar, "SECA," CASES 2005, pp. 78–89,
  DOI 10.1145/1086297.1086308 (bus-level policy enforcement).
- Jedhe, Ramamoorthy, Varghese, "A Scalable High Throughput Firewall in FPGA,"
  FCCM 2008, pp. 43–52, DOI 10.1109/FCCM.2008.31.
- Tiwari et al., "Complete Information Flow Tracking from the Gates Up" (GLIFT),
  ASPLOS 2009, pp. 109–120, DOI 10.1145/1508244.1508258 (parallel-logic spirit,
  different problem).
- Cotret, Crenne, Gogniat, Diguet, FCCM 2012, pp. 200–207, DOI 10.1109/FCCM.2012.42
  (latency-efficient MPSoC firewalls; latency as overhead to minimize).
- Delshadtehrani et al., "PHMon," USENIX Security 2020, pp. 807–824 (no DOI,
  USENIX; instruction-stream monitor, not authorization).
- Restuccia, Meza, Kastner, "Aker," ICCAD 2021, pp. 1–9,
  DOI 10.1109/ICCAD51958.2021.9643538 (recent Zynq-class access-control wrappers).
- Reserve: Sanny, Ganegedara, Prasanna, IPDPSW 2013, pp. 124–133,
  DOI 10.1109/IPDPSW.2013.249.

**Recommended posture (agent, sound):** claim zero-cycle dimensional scaling of a
standardized AC model measured in synthesized hardware; concede single-cycle
wide-key matching (TCAM) and deterministic classification latency (BV family) as
prior art, cited head-on.

---

## Sweep 4 of 4 — ROS 2 / DDS attack literature (COMPLETE)

12 verified via Crossref/publisher (one industry white paper flagged).

### ⚠ ANTICIPATION FLAG — one partial

- **Salimi, Keramat, Peña Queralta, Westerlund, "A customizable conflict
  resolution and attribute-based access control framework for multi-robot
  systems,"** J. Systems Architecture 168, art. 103528, 2025,
  DOI 10.1016/j.sysarc.2025.103528. Per-request ABAC for ROS 2 multi-robot
  systems via a Hyperledger Fabric bridge. Closest on the attribute/context axis.
  Lacks provenance (source type) and runtime-state dimensions, and runs off-path
  on a blockchain bridge, not as an inline bounded-latency primitive.

**Headline: no located work enforces per-command application-layer authorization
conditioned on command source type. The provenance dimension is unclaimed.**

### Strongest gap evidence (transport layer is not enough)

- **Deng, Xu, Zhou, Zhang, Liu, "On the (In)Security of Secure ROS2,"** ACM CCS
  2022, pp. 739–753, DOI 10.1145/3548606.3560681. Four vulnerabilities bypass
  SROS2/DDS-Security access control; a credentialed/revoked node can keep
  publishing. Their fix stays at the crypto layer. Directly reinforces our gap.
- White, Caiazza, Jiang, Ou, Yang, Cortesi, Christensen, EuroS&PW 2019,
  pp. 57–66, DOI 10.1109/EuroSPW.2019.00013 (Secure DDS leaks capability lists).
- Wang, Li, Guan, "A Formal Analysis of Data Distribution Service Security,"
  ASIA CCS 2024, pp. 716–727, DOI 10.1145/3634737.3656288.
- Maggi et al. (Trend Micro), DDS analysis white paper, 2022 — industry, author
  list not fully verified; pair with the peer-reviewed entries.

### Ancestors and neighbors

- Dieber, Kacianka, Rass, Schartner, IROS 2016, pp. 4477–4482,
  DOI 10.1109/IROS.2016.7759659 — earliest app-layer ROS authorization,
  identity-based.
- Dieber, Breiling, Taurer, Kacianka, Rass, Schartner, Robotics and Autonomous
  Systems 98, pp. 192–203, 2017, DOI 10.1016/j.robot.2017.09.017 — journal
  version, the app-layer ancestor to cite.
- White, Christensen, Caiazza, Cortesi, "Procedurally Provisioned Access Control
  for Robotic Systems," IROS 2018, DOI 10.1109/IROS.2018.8594462 — DDS
  permission provisioning, the layer we argue is blind.
- Rivera, Lagraa, Nita-Rotaru, Becker, State, "ROS-Defender," IEEE SPW 2019,
  pp. 114–119, DOI 10.1109/SPW.2019.00030 — closest on command filtering;
  network-flow layer, identity + anomaly keyed.
- DeMarinis et al., ICRA 2019, pp. 8514–8521, DOI 10.1109/ICRA.2019.8794451 —
  internet-wide ROS exposure scan (motivation).
- Rivera, Lagraa, State, "ROSploit," IRC 2019, pp. 415–416,
  DOI 10.1109/IRC.2019.00077 — offensive tooling incl. message injection.
- Puccetti et al., "ROSPaCe" dataset, Scientific Data 11, art. 481, 2024,
  DOI 10.1038/s41597-024-03311-2 — IDS line (detect after) vs our prevention.

---

## Integration (2026-08-07)

18 of the ~46 verified references were merged into `paper/refs.bib` and the
Section VII rubric (a 6-page conference paper cannot carry all of them; the
rest are banked here for the journal version). The novelty claim was rewritten
to its bounded form: concede TCAM wide-key matching, BV-family deterministic
latency, and 2006-era policy-to-circuit compilation as ancestors; claim the
composite — a standardized relational authorization model in fabric with
measured zero-cycle cost for added dimensions — plus the unclaimed provenance
dimension. Deng et al. (CCS 2022) was added to the threat model as evidence the
transport layer's own access control is bypassable.

## Sweep 2 of 4 — NGAC/ABAC models and hardware implementations (COMPLETE)

12 peer-reviewed/standards + 1 patent, verified via Crossref, ACM DL, Springer,
NIST CSRC, Google Patents. One caveat: entry [7] JoWUA pages/DOI not verified
(journal site timed out); confirmed via NIST's publication record.

### Headline: NO prior hardware or FPGA implementation of NGAC found

Searches for NGAC+FPGA/hardware returned only SoC isolation wrappers, device
access-control patents, and one non-peer-reviewed master's thesis (Arrowhead
IoT). **The composite claim holds:** nobody compiles INCITS 565 policy graphs to
fixed-width bitmasks and demonstrates measured zero-cycle cost for added policy
dimensions in synthesized hardware. Four works each hold one piece:

### ⚠ ANTICIPATION FLAGS — four, each partial

1. **Liu, Chen, Hwang, Xie, "Designing Fast and Scalable XACML Policy Evaluation
   Engines,"** IEEE Trans. Computers 60(12), pp. 1802–1817, 2011.
   DOI 10.1109/TC.2010.274. (Conf. version: XEngine, ACM SIGMETRICS 2008,
   pp. 265–276, DOI 10.1145/1375457.1375488.)
   Compiles XACML into normalized numeric structures offline. Closest software
   analogue. Software only, data-dependent tree-walk latency, XACML not NGAC.
2. **Pina Ros, Lischka, Gómez Mármol, "Graph-based XACML Evaluation,"**
   SACMAT 2012, pp. 83–92. DOI 10.1145/2295136.2295153.
   Precompiles XACML into decision diagrams. Software, variable latency.
3. **Huffmire, Prasad, Sherwood, Kastner, "Policy-Driven Memory Protection for
   Reconfigurable Hardware,"** ESORICS 2006, LNCS 4189, pp. 461–478.
   DOI 10.1007/11863908_28.
   **Single closest hardware ancestor:** formal policy compiled directly to a
   synthesized FPGA reference monitor. Policy class is DFA memory-range
   isolation, not relational multi-dimensional authorization; no
   dimensionality-cost analysis. (Same line as the TODAES 2008 flag in sweep 3.)
4. **Chang, Bagepalli, Narayan, Patra, "Highly Parallel Evaluation of XACML
   Policies,"** U.S. Patent 8,677,453 B2 (Cisco), granted 2014.
   Bit-vector indexing of XACML rules for parallel search in an appliance. The
   only pre-existing bit-vector treatment of ABAC-standard policy. Patent claim,
   no constant-time result, not NGAC.

### Must-cite software performance baseline

- **Mell, Shook, Harang, Gavrila, "Linear Time Algorithms to Restrict Insider
  Access using Multi-Policy Access Control Systems,"** JoWUA 8(1), 2017.
  Pages/DOI not verified (NIST record + PDF confirm content). Best known
  software NGAC decision performance: O(n) graph traversal. Our closed form
  moves the graph cost to compile time. Directly comparable.

### Standards and formal models

- Ferraiolo, Gavrila, Jansen, "Policy Machine: Features, Architecture, and
  Specification," NIST IR 7987 r1, 2015. DOI 10.6028/NIST.IR.7987r1. The
  semantics our compiler preserves.
- Ferraiolo, Chandramouli, Kuhn, Hu, "XACML and NGAC," ABAC '16 @ CODASPY,
  pp. 13–24. DOI 10.1145/2875491.2875496. NGAC-vs-XACML positioning.
- Hu et al., "Guide to ABAC Definition and Considerations," NIST SP 800-162,
  2014/2019. DOI 10.6028/NIST.SP.800-162. State/provenance are environment-class
  attributes made first-class.
- Jin, Krishnan, Sandhu, "A Unified Attribute-Based Access Control Model"
  (ABAC-alpha), DBSec 2012, LNCS 7371, pp. 41–55. DOI 10.1007/978-3-642-31540-4_4.
- Servos, Osborn, "HGABAC," FPS 2014, LNCS 8930, pp. 187–204.
  DOI 10.1007/978-3-319-17040-4_12. Hierarchies we flatten at compile time.
- Servos, Osborn, "Current Research and Open Problems in ABAC," ACM CSUR 49(4),
  Art. 65, 2017. DOI 10.1145/3007204. Neutral citation that ABAC performance is
  an open problem.
- Turkmen, Crispo, "Performance Evaluation of XACML PDP Implementations,"
  SWS '08, pp. 37–44. DOI 10.1145/1456492.1456499. Software PDP latency grows
  with policy size.
- Hategekimana, Mandebi Mbongue, Pantho, Bobda, "Inheriting Software Security
  Policies within Hardware IP Components," FCCM 2018, pp. 53–56.
  DOI 10.1109/FCCM.2018.00017. Label-check wrappers on FPGA IP, not policy-model
  evaluation.


---

## Gemini Deep Research cross-check (received 2026-08-07 ~17:30)

Raw evidence: `docs/sota-garbeled.txt` (report) and `docs/related_work.tex`
(Gemini's BibTeX export). **Neither is citable as delivered.** Every entry was
checked against our agent-verified data or re-verified online before use.

### Verdicts — the analysis confirms the sweep on every kill-shot question

| Q | Gemini verdict | Agrees with our sweep? |
|---|---|---|
| 1. Hardware NGAC anywhere | "REFUTED — NO PRIOR ART EXISTS. 5D H-NGAC is the first hardware implementation of NIST NGAC" | Yes — **two independent engines now confirm** |
| 2. Zero-cycle dimensionality | Adjacent patterns only (StrideBV, Aker); nobody proves it for policy models | Yes |
| 3. Closed-form latency vs policy size | No direct prior art | Yes |
| 4. Provenance/state per-command authorization | Silos only (SecOC origin auth, ISO 10218 interlocks, DDS-Security topic ACLs); nothing unified | Yes, and adds the industrial-standards framing |
| 5. Underweighted venues/patents | No direct conflicts | Yes |
| 6. TCAM/P4/SmartNIC | Adjacent only | Yes |
| 7. 2024–2026 recency | No direct conflicts | Yes |

### Verification table — Gemini's bibliographic data is unreliable

| Gemini entry | Verdict | Evidence |
|---|---|---|
| Aker DOI `10.1109/ICCAD51581.2021.9643534` | **WRONG** | Crossref: DOI does not exist. Ours (`…51958…9643538`) resolves to the correct paper |
| "m-NGAC Patent US 11,909,768 B2" (ranked #2 closest prior art) | **WRONG NUMBER** | That patent is "in-home network security using AI," Erik J. Barnett, Securetelligence LLC. The real NIST line exists (below) but not under this number |
| "VLSI Access Control Engine, SCCTS Research Team, Energies 2025" (ranked #3 closest) | **FABRICATED** | DOI `10.3390/en18081973` resolves to "Blockchain-Based, Dynamic Attribute-Based Access Control for Smart Home Energy Systems" — a real but unrelated software paper. The 65nm CMOS FSM description is confabulated |
| Deng CCS 2022 authors "Deng, Aoyagi, Zhao, Kim", pp. 1205–1219, DOI `…3560662` | **WRONG** | Actual (Crossref-verified): Gelei Deng, Guowen Xu, Yuan Zhou, Tianwei Zhang, Yang Liu, pp. 739–753, DOI `…3560681` |
| SECA pages 129–138, DOI `10.1145/1086228.1086246` | **CONFLICTS** with agent-verified pp. 78–89, DOI `10.1145/1086297.1086308` — ours from Crossref/ACM DL |
| StrideBV TPDS "Single Bit-Vector Lookup," 25(8) 1967–1976, DOI `…2013.216` | **CONFLICTS** with agent-verified title/pages/DOI (`…2013.261`, pp. 1135–1144) |
| Huffmire TODAES DOI `10.1145/1365890.1365896` | **CONFLICTS** with agent-verified `10.1145/1367045.1367053` |
| `ferraiolo2013ngac` "NIST SP 800-162 / INCITS 565" as one 2013 techreport | **GARBLED** — conflates the 2014 ABAC guide, the 2020 standard, and the Policy Machine manual |
| Honeybee/NEMO "NSF Research Team" | **CONFLATED** — probably a real vector-DB RBAC paper, but attribution is garbage; low relevance; not integrated |
| Huffmire ESORICS 2006, AUTOSAR SecOC R20-11, Fugkeaw IEEE Access 2024 | Consistent / plausible |

### Genuinely new, verified, and integrated

- **Ferraiolo, Gavrila, Katwala, Roberts, "Imposing Fine-grain Next Generation
  Access Control over Database Queries," 2nd ACM Workshop on ABAC (ABAC '17),
  2017. DOI 10.1145/3041048.3041050.** Verified via NIST publication record.
  This is the real, citable core of Gemini's garbled patent entry: NIST's own
  NGAC enforcement line is database/software. Added to refs.bib and Section VII
  as direct support for the "no hardware NGAC" posture. (NIST also holds
  related patents licensed to Tetrate.io and the NSA, per NIST TPO records;
  patent numbers unverified, so we cite the paper, not a patent.)

### Recorded, not cited (team context from Gemini)

- Faruque: Gate-Breaker (netlist-to-RTL reverse engineering, 2025);
  TrojanWhisper (arXiv:2412.07636, 2024); GHOST (2024); WBAN security. Hardware
  trojan/verification line; supports his hardware-lead role, not related work
  for this paper.
- Badawy: compiler vectorization across x86/ARM (MWSCAS 2025, unverified) —
  potentially relevant if the Cortex-A9 baseline ever lands.

**Net effect:** the novelty claim now stands on two independent engines, and the
Section VII sentence "no prior hardware implementation of NGAC exists at all"
survives an adversarial deep-research pass. The cost of skipping verification
would have been three garbage citations in a submitted paper, including two of
Gemini's own top-five "closest prior works."
