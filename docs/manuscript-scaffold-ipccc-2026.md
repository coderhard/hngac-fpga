# IPCCC 2026 Manuscript Scaffold — 5D H-NGAC

Assembled 2026-08-07. This is the consolidated story, the asset assessment, and the
section-by-section build plan. The live draft is `paper/main.tex`. Authoritative
numbers: `docs/canonical-context.md`. Do not restate numbers here; point at them.

---

## 1. The one-line story (defense-paper causal chain)

> Robots leave authorization out of the control loop because software cannot bound
> the decision → identity-only authorization admits two CVE-anchored attack classes
> (unsafe-state operation, command provenance abuse) → 5D H-NGAC extends the DCAS
> bitmask primitive with state σ and provenance π and synthesizes to fabric → the
> added dimensions cost zero clock cycles and +11.4% LUT, with latency closed-form
> at L(n) = 12 + n/2 and zero jitter **by construction** (fixed trip count, no early
> exit) → authorization worst case becomes a synthesis parameter, so it fits inside
> a control-loop budget.

Contribution word: **dimensionality**. Not hardware (DCAS's), not hypergraph
(BigData's). The paper *extends* H-NGAC; it never "presents" it.

## 2. Asset assessment (what exists, what is missing)

### Exists and verified

| Asset | Where | State |
|---|---|---|
| HW cycles, 4D+5D, 6 policy sizes | `hngac-package-from-farouq/results/README.md` + raw cosim rpt | Reproduces exactly; min=avg=max at every point |
| Closed form 12 + n/2 | derived | Exact at all 6 points (deterministic fit, not statistical) |
| Resources 4D vs 5D | csynth.rpt | +524 LUT, +100 FF, 0 BRAM/DSP, same II/slack |
| Vivado post-route (4D, older kernel) | `kernel/4d/hngac-fpga_4d_hw_results/` | WNS +2.170 ns, 0/1987 failing endpoints |
| SW cycles, 6 models × 6 sizes | `results/sw/sw_cycles.csv` | perf-measured clock; **derived**, per Omar 2026-08-07 |
| Board functional run | `board-test/` | 2,307 PASS; **no timing** |
| ROS 2 adversarial demo | `data/attack2_gatekeeper_20260418_150727.log` | 18,878/18,878 blocked, 0/17,059 FP |
| Over-authorization corpus result | evidence-record block 1 | RBAC/DAG/3D admit 200k/200k incl. 100k state-violating |
| OPA/XACML baselines | TS-NGAC (under review) + DCAS | OPA 271.491 µs mean, 13 deadline misses; XACML ~50 ms |
| Attack taxonomy + terminology | canonical-context | Named classes; H-NGAC=Hardware, HyperNGAC=BigData |
| Two paper figures | `docs/figures/*.png` | dataviz-validated palette |
| Abstract rev4, 229 words | `docs/ipccc-2026-abstract-draft.md` | Ready |

### Missing or dead

| Gap | Status | Manuscript consequence |
|---|---|---|
| **`paper/` skeleton ("~70% written", April)** | **Gone.** Was gitignored + local only; not on this disk | Draft rebuilt from zero 2026-08-07. `.gitignore` fixed so this cannot recur |
| 3D synthesis | Permanent (Omar: no 3D kernel code) | Scope everything 4D vs 5D |
| On-board timing | Will not exist for IPCCC | Co-sim is the timing authority; board = functional only |
| Cortex-A9 baseline | Possible, not committed | Optional; framing risk noted in canonical-context |
| Related-work sweep | Not run | Task 2 of `docs/manuscript-agent-prompt.md`; rubric slots marked TODO in draft |
| Sitharaman/Gupta affiliations | **Resolved 2026-08-07** | Zetafence, Inc.; Texas A&M University–Central Texas (ac4aiagents Overleaf bios) |
| HyperNGAC name sign-off | Pending Sai | Needed before print |
| opt-v1 reconciliation into `fpga/hls/src/` | Open | Not needed for the paper; PROVENANCE.md covers the delta |

## 3. Statistics and honesty ledger (stats-advisor gate)

Every number in the draft carries one of these labels. The draft's method section
states the ledger explicitly.

| Class | Numbers | Treatment |
|---|---|---|
| **Measured, deterministic** | HW cycles (min=avg=max), resources, closed form | No CI applies; variance is zero by construction (fixed trip count). Say "by construction," never "observed zero jitter" |
| **Measured, empirical** | SQLite path, ROS 2 demo counts, board PASS counts | Report with denominators. Block rate 100% (18,878/18,878; one-sided 95% Wilson lower bound 99.99%). FP 0/17,059 (95% upper bound 0.02%) |
| **Derived** | SW per-decision cycles = mean ns × perf-measured 4.96 GHz | Say "derived from a perf-measured clock," never "counter reads" |
| **Modeled** | RBAC busy-wait (6,674×) | Excluded from empirical claims; use SQLite 20.6× |
| **Null result** | Timing-window slips: 0 in 8,733 callbacks | Architectural argument only; never an effectiveness number |
| **Prior measured** | OPA 271.491 µs / 13 misses; XACML ~50 ms; DCAS 34–37 ns | Cite to source paper; OPA/XACML reuse is an OPEN AUTHOR DECISION, marked in the tex |

Distributional claims about SW latency: WSL2 scheduler jitter contaminates max; cite
min/P99, never call any SW max a WCET (157 µs = "observed scheduling outlier").

## 4. Section map (what goes where, which HSK move carries it)

| § | Content | HSK move | Evidence source |
|---|---|---|---|
| Abstract | rev4 verbatim, minor tense fit | defense-pattern 43/62/96/26 | abstract draft doc |
| I. Introduction | Named-actor operational hook (technician + interlocked robot + remote command); capability-to-gap pivot off SROS2/DDS-Security; BigData as intellectual origin; contributions (a)(b)(c); roadmap | Hook + gap engine + front-loaded contributions + chained-verb roadmap | canonical-context, CVEs |
| II. Threat model & background | DAG-NGAC vs H-NGAC; three named attack classes w/ CVE anchors; timing window as cross-cutting property; explicit assumptions | Scope-bounding, explicit assumptions | taxonomy in canonical-context |
| III. 5D model | Masks, domains in set notation; Hypothesis (dimension free in time iff combinational, not sequential); Definitions; permit equation; limit behavior | Greek factors + Hypothesis/Definition ladder + limit-behavior prose | kernel source semantics |
| IV. Hardware architecture | Two-rule/clock pipeline, II=1, **no early exit → determinism by construction**; interfaces; no BRAM/DSP | Render structure formally | opt-v1 source + csynth |
| V. Method | Platforms, corpus (11 requests per 4 rules, 2.67×n scaling), measurement classes, honesty ledger | Practitioner quantification | PROVENANCE.md, results README |
| VI. Results | KEY FINDING leads; cycles table; resources; SW slope 0.645/1.024/1.220; over-authorization table; ROS 2 demo w/ Wilson bounds; worst-case table; **pre-empt the wrong inference** (CPU wins mean wall clock by ~19× at 500 rules, claim is boundedness) | Headline first, defend, pre-empt | canonical-context §HW results |
| VII. Related work | **Scored rubric**: axes pdl, wcb, dim, hwv ∈ Z, 0–5, one-line justification each, ours max under stated definitions; TODO rows await sweep | Signature rubric move | verified cites + sweep |
| VIII. Limitations | 3D never synthesized (code lost — say so); no on-board timing; WSL2 jitter; i7-vs-fabric asymmetry; ≤512 rules; modeled path excluded | Honest incl. what failed | canonical-context honesty rules |
| IX. Future work + conclusion | Numbered open questions; call to community for cycle-level authorization data; "In conclusion," hand to robotics platform + safety engineers | Numbered questions + community call + named audience | — |

## 5. Citation ledger

**Verified real (safe to cite now):** DCAS 2026 (10.1109/dcas69364.2026.11544855);
BigData 2025 (10.1109/bigdata66926.2025.11401728); IEEE Access 2025
(10.1109/access.2025.3561235); ICDH 2026 (presented, Sydney); TS-NGAC (under
review — status string fixed); INCITS 565-2020 NGAC standard; Ferraiolo et al.
Policy Machine (JSA 2011); SROS2 (Mayoral-Vilches et al., IROS 2022); ROS 2
(Macenski et al., Science Robotics 2022); OMG DDS-Security 1.1; OASIS XACML 3.0;
Open Policy Agent (software); CVE-2021-38425, CVE-2022-33323, CVE-2022-45789 (NVD);
CHERI (Watson et al., IEEE S&P 2015).

**TODO from the related-work sweep (do not fabricate):** FPGA access-control /
security-primitive prior art; NGAC/ABAC hardware implementations; WCET-bounded
security enforcement in CPS; ROS 2 / DDS attack literature beyond SROS2. Slots are
marked `% TODO-SWEEP` in `refs.bib` and rubric rows.

## 6. Voice contract for every editing session

HSK v1.2 academic, **authentic mode** (calibrated hedges stay, peer review).
Non-negotiables: "we" active; no em-dashes; minimal semicolons; staccato preserved;
"Further," not "Furthermore,"; define before use; every Greek symbol gets a domain;
related work is scored, never narrated; results lead with the headline number and
pre-empt the wrong inference. Forbidden list per profile. Abstract obeys the
hardware-security abstract skill; body obeys this profile.

## 7. Open author decisions carried into the draft

1. **OPA/XACML reuse** — numbers are IN the draft, flagged `% AUTHOR DECISION` in
   the tex. Cut them if the journal split argues otherwise.
2. **HyperNGAC name** — used once, flagged, pending Sai's sign-off.
3. ~~Sitharaman/Gupta affiliations~~ resolved 2026-08-07 (Zetafence, Inc.; Texas A&M University–Central Texas, from the ac4aiagents Overleaf bios). Badawy name rendering still open.
4. Cortex-A9 run — draft assumes it does not arrive; §VIII notes it as future work.
