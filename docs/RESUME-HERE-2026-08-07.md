# Where we left off — 7 August 2026, late morning

Read this first when you come back. Written at the point Hassan had to leave.

## Deadline state

- **IPCCC abstract: due today.** The abstract is finished and ready to submit.
  `docs/ipccc-2026-abstract-rev4.docx` — that is the file to work from.
- **Manuscript: due next week.** Not started. `docs/manuscript-agent-prompt.md` is
  ready to paste into a fresh agent session when you want to begin.

## The one thing to do next

**Submit the abstract.** Use title option 1, "Security Dimensionality at Zero Time
Cost: A Provenance-Aware NGAC Authorization Primitive on FPGA." It is 229 words; if
the form caps at 220, the exact cut is written into the doc.

The author block is final. Affiliations resolved (Sitharaman: Zetafence, Inc.;
Gupta: Texas A&M University–Central Texas) and Badawy's rendering confirmed from
his Scholar profile: "Abdel-Hameed A. Badawy".

## What changed today, in order

1. Verified Omar's hardware package end to end. Every derived table reproduces from
   the raw reports. Committed it with a `PROVENANCE.md`.
2. Resolved the DCAS citation via Crossref (IEEE Xplore blocks automated fetches).
   It is DCAS, not DASC: `10.1109/dcas69364.2026.11544855`.
3. Rewrote the abstract to the hardware-security defense pattern, four revisions.
4. Added the over-authorization result to the abstract, which was the strongest
   security number in the project and had been sitting unused.
5. **Fixed the attack-class taxonomy across all three docs.** Integer numbering is
   retired. See below.
6. **Split H-NGAC from HyperNGAC.** See below.
7. Recorded Omar's answers to the email asks. **He answered before the email was
   sent** — the draft in `docs/email-to-farouq-2026-08-07.md` is now partly obsolete.

## Decision 1 — attack classes are named, not numbered

Three files disagreed. Two root causes, both fixed:

- **CVE-2022-45789 was doing double duty**, anchoring unauthorized access in one file
  and the timing window in another. It is a session hijack, so it belongs to
  unauthorized access only.
- **The timing window was numbered as a peer class.** It is not one. It is a
  cross-cutting delivery property closed by the latency bound, not by any dimension.
  `project-overview.md` had attributed it to 4D, which was wrong on every reading.

Canonical names now: **unauthorized access** (3D), **unsafe-state operation** (4D),
**command provenance abuse** (5D). Full table in `docs/canonical-context.md`.

`attack2_*` filename tokens are preserved on purpose — they are committed evidence
identifiers and mean command provenance abuse.

## Decision 2 — H-NGAC is Hardware-NGAC, and it is not HyperNGAC

**"H-NGAC" was never expanded anywhere in this repo**, in 30+ uses. That is why the
terms kept mixing. Now fixed:

| Term | Means | Whose |
|---|---|---|
| H-NGAC | **Hardware**-NGAC | DCAS 2026 |
| 5D H-NGAC | this paper | IPCCC 2026 |
| HyperNGAC | hypergraph privilege analysis | BigData 2025 |
| DAG-NGAC | INCITS 565 reference model | prior art |

Three consequences that bite:

1. **This paper extends H-NGAC; it does not present it.** "We present H-NGAC"
   over-claims against our own published DCAS paper, which we cite.
2. **Never say H-NGAC compiles hypergraphs.** It compiles policy graphs. The
   hypergraph framing is BigData's and was ceded deliberately.
3. **The contribution word is dimensionality.** Hardware is DCAS's, hypergraph is
   BigData's. Neither is available to this paper.

**HW-HyperNGAC was considered and rejected** — it fragments a published, award-winning
brand and puts both predecessors' contributions in the name while the actual new result
appears nowhere in it.

**Open:** HyperNGAC names Sai's system. **Confirm with Sitharaman before it goes to
print.**

## Omar's answers, received 10:26 today

Source: `hngac-package-from-farouq/response_from_farouq.txt`.

- **3D synthesis: will not happen.** He does not have the 3D kernel code. The gap is
  permanent for IPCCC. Every claim stays scoped to 4D versus 5D. This retroactively
  confirms the abstract's scoping was right and keeps title option 2 retired.
- **Cortex-A9 baseline: possible, not committed.** He notes the board CPU is weaker
  than the i7. Framing risk: it can read as choosing a weak CPU. Boundedness versus
  the i7 stays the primary argument.
- **Cycle method: resolved.** Use the perf numbers; the `CYCLES|` lines are stale
  rdtsc artifacts. **Caveat:** the figures are still mean_ns times a perf-measured
  clock, not per-decision counter reads. Say "derived from a perf-measured clock."
- **No board timing, and there will not be any.** Co-simulation is the authoritative
  hardware timing source. The board run stays functional verification only.

## Open items, ranked

1. Submit the abstract. The author block is final; nothing blocks submission.
2. Confirm HyperNGAC with Sai before it appears in print.
3. Rewrite `docs/email-to-farouq-2026-08-07.md` — Omar already answered. What is left
   is thanking him, confirming the perf-versus-rdtsc decision, and deciding whether
   the A9 run is worth his week.
4. Decide the OPA/XACML reuse from the withdrawn TS-NGAC work. Open author decision,
   not a default.
5. Reconcile the `opt-v1` two-rules-per-clock optimization back into
   `fpga/hls/src/hngac_kernel.cpp`. The repo kernel is not the measured kernel.
6. Decide the benchmark label mismatch: software rows say `H-NGAC 3D/4D/5D`, which
   under H = Hardware reads oddly. Renaming breaks continuity with committed CSVs and
   figures. Deferred deliberately.

## Things that will bite if forgotten

- The FPGA is **not faster** than the CPU on mean wall clock. At 500 rules the i7 wins
  by roughly 19x. The claim is boundedness and zero jitter, never speed.
- The board test is **functional only**. 2,307 PASS is not a timing result.
- Say **"free in time, nearly free in area,"** never "zero hardware cost."
- The measured timing-window slip rate is **zero**. Never claim otherwise.
- `hngac-package-from-farouq/` is immutable evidence. Never edit it.
