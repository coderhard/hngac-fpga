# Manuscript agent prompt — IPCCC 2026 H-NGAC

Paste the block below into a fresh agent session rooted at `~/projects/hngac-fpga`.
It is written to be model-agnostic.

**Use it for the manuscript, not the abstract.** The abstract is a single judgment call
on framing and is faster to write by hand.

**What this deliberately does not do:** it does not ask agents to draft body sections in
parallel. This paper is one argument chain (dimensionality is free in hardware, so three
attack classes cost the area of one, so the worst case is bounded in a way software
cannot match). Split that across agents and it breaks at the section seams. Drafting
stays with one author and one voice. Agents do the work that genuinely parallelizes:
verification, literature, and adversarial review.

---

## THE PROMPT

You are helping finish an IEEE IPCCC 2026 submission in `~/projects/hngac-fpga`.
The manuscript is due this week. Accuracy matters more than speed.

### Read these first, in this order

1. `docs/canonical-context.md` — **authoritative.** Every number and framing rule.
   If anything you produce conflicts with this file, this file wins.
2. `hngac-package-from-farouq/PROVENANCE.md` — what the hardware evidence is and is not.
3. `docs/hngac-fpga-status-review-2026-08-07.md` — current state, results, known gaps.
4. `CLAUDE.md` — repo conventions and the multi-agent coordination protocol.

### Non-negotiable accuracy rules

These exist because the project has already had claims drift from its evidence. Any
output violating one is wrong regardless of how well written it is.

1. Never write "zero hardware cost." The fifth dimension costs +524 LUT (+11.4%).
   The correct phrasing is **free in time, nearly free in area.**
2. Never claim 3D, 4D and 5D resolve in the same LUT-stage count. **3D was never
   synthesized, and will not be.** Omar Faruque confirmed 2026-08-07 that he does not
   have the 3D kernel code. The gap is permanent for IPCCC. Only 4D versus 5D is
   supported; do not infer 3D and present it as measured.
3. The target part is **Zynq-7020 (xc7z020-clg400-1)**. Not UltraScale+.
4. Never state or imply the FPGA is faster than the CPU on mean latency. It is not:
   at 500 rules the CPU wins by roughly 19x on wall clock. The claim is **boundedness
   and zero jitter**, never mean speed.
5. The board test is **functional verification only**. No on-board timing exists.
   Never cite 2,307 PASS as a timing result.
6. The RBAC 6,674x figure is a **modeled busy-wait**, never empirical. Use the
   SQLite-backed 20.6x for empirical RBAC-plus-state claims.
7. BigData 2025's 0.12 s is a **batch compliance sweep over n=4000**, not a
   per-decision latency. Cite the paper prominently, but keep that number out of the
   per-decision latency table.
8. TS-NGAC is **withdrawn and unpublished**, retargeted to a journal. Cite it as
   **"under review"** — never as an ICCCN paper. Its OPA and XACML baselines may be
   reused in the manuscript, but its time-scoping contribution belongs to the journal
   version. OPA numbers were deliberately kept out of the abstract; adding them to the
   manuscript is an open author decision, not a default.
9. Software cycle counts are **derived**, not per-decision counter reads. **Resolved
   2026-08-07 by Omar Faruque:** use the perf-based numbers (`extract_sw_cycles.py` /
   `sw_cycles.csv`) and **discard the benchmark's `CYCLES|` lines**, which are stale
   rdtsc artifacts. That settles the 70-versus-82.29 disagreement. But the figures are
   still mean_ns times a perf-*measured* clock (4.96 GHz), not per-decision counter
   reads. Write "derived from a perf-measured clock." Never "hardware counter reads."
10. Cite every source file for every hardware number. If you cannot point to the file
    in `hngac-package-from-farouq/`, do not state the number.

### Mandatory citations

All four must appear. This is the group's own research line and omitting any of them
misrepresents the lineage. BigData is the **intellectual origin** and belongs in the
Introduction, not only Related Work.

1. H. Karim, S. Sitharaman, D. Gupta, "Hardware-Accelerated NGAC Authorization for
   Real-Time Multi-Robot Systems," 2026 IEEE 19th Dallas Circuits and Systems
   Conference (DCAS), pp. 1-4, 2026. Best Poster Award.
   https://doi.org/10.1109/dcas69364.2026.11544855
2. S. Sitharaman, H. Karim, D. Gupta, M. Tyagi, "Scalable Privilege Analysis for
   Multi-Cloud Big Data Platforms: A Hypergraph Approach," IEEE BigData 2025,
   pp. 6626-6633. https://doi.org/10.1109/bigdata66926.2025.11401728
3. H. Karim, D. Gupta, S. Sitharaman, "Securing LLM Workloads with NIST AI RMF in the
   Internet of Robotic Things," IEEE Access, Jan. 2025.
   https://doi.org/10.1109/access.2025.3561235
4. H. Karim, S. Sitharaman, D. Gupta, D. B. Rawat, "Securing Autonomous Clinical
   Agents: Time-Scoped Hypergraph Delegation for Controlling Patient Data Access,"
   presented at IEEE ICDH 2026, Sydney, Australia.

### Tasks

Run task 1 first and alone. It is the highest-value task and the others depend on a
draft that is already numerically sound. Tasks 2 and 3 can run in parallel.

**Task 1 — numbers-integrity audit (highest priority).**

Walk the manuscript claim by claim. For each factual claim:

- locate the source file in `hngac-package-from-farouq/` and quote the line
- mark it CONFIRMED, WRONG, or UNSUPPORTED
- check it against all ten accuracy rules above
- for any hardware number, verify it against the raw report, not against another
  prose summary of the report

Report as a table: claim, manuscript location, source file and line, verdict, and the
corrected wording where needed. Do not rewrite the manuscript. Report only.

Flag specifically: any surviving "zero cost" phrasing, any 3D hardware claim, any
UltraScale+ mention, any speedup framing that invites a wall-clock comparison, and any
ICCCN citation.

**Task 2 — related work sweep.**

Four independent searches, one agent each, no shared context:

- FPGA-accelerated access control and security primitives in reconfigurable logic
- NGAC and ABAC formal models, and any prior hardware implementations
- WCET and real-time guarantees for security enforcement in cyber-physical systems
- ROS 2, DDS-Security and SROS2 attack literature, especially compromised-node and
  command-injection work

For each: 8 to 12 strong references with full bibliographic data and DOIs, a one-line
statement of what it does, and one line on how it relates to or differs from this work.
Flag anything that anticipates the dimensionality-is-free result, since that is our
novelty claim and we need to know early if someone got there first.

**Task 3 — adversarial review.**

Read the manuscript as a hostile IPCCC reviewer who has opened the synthesis reports.
Produce the strongest attacks you can, ranked by how likely each is to sink the paper.
For each: the attack, whether it lands, and the best available response.

Start from these known weak points, then find more:

- the fabric runs at 100 MHz while the software baseline runs at 4.96 GHz
- 3D was never synthesized, so the three-way claim is incomplete
- the board test proves function, not timing
- co-simulation is not silicon measurement
- 500 rules may be small relative to a real deployment policy
- the software baseline is a laptop CPU, not an embedded target

Then verify each finding independently before reporting: try to refute your own attack
and discard the ones that do not survive.

### Output rules

- Report findings; do not edit the manuscript unless explicitly asked.
- Quote source files by path and line for every claim you evaluate.
- If a number cannot be traced to a file, say UNSUPPORTED rather than guessing.
- Claim files on `docs/coordination-board.md` before editing anything, per `CLAUDE.md`.
- `hngac-package-from-farouq/` is immutable evidence. Never edit it.

---

## Sequencing note

Run task 1 before tasks 2 and 3. A related-work sweep against a draft with wrong
numbers wastes the sweep, and an adversarial reviewer will spend its effort on errors
you already know about instead of the ones you do not.
