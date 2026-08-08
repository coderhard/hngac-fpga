# Email to Omar Faruque (cc Hameed Badawy) — v2, 2026-08-07 evening

Supersedes the morning draft, which was written before Omar's 10:26 answers and
was never sent. His answers are in `hngac-package-from-farouq/response_from_farouq.txt`
and resolved in `docs/canonical-context.md`.

**Attach:** `paper/main.pdf`

**Subject:** Perf numbers confirmed in the paper — draft attached, comments by Tuesday

---

Omar,

Thank you for the fast answers this morning. They settled every open question,
and the paper moved a long way today because of it. The abstract went in, and
the full manuscript draft is attached.

**The perf decision is confirmed, and it is in the paper.** Every software
cycle figure comes from your perf pipeline: `extract_sw_cycles.py` against
`perf_all_models_scaling.log`, exactly as you delivered it. The `CYCLES|` lines
are discarded as the rdtsc artifacts you flagged. That resolves the 70-versus-82.29
disagreement in perf's favor.

One nuance in how the paper words it, so we are saying the same thing if a
reviewer asks. The per-decision figures are mean nanoseconds multiplied by the
perf-measured 4.96 GHz clock. So the paper says "derived from a perf-measured
clock" and never "per-decision counter reads." Better than rdtsc, better than a
nominal clock, and honest about what it is. That framing keeps anyone from
demanding per-decision counter dumps we do not have.

**On your other answers, three decisions.**

1. **3D: closed.** No 3D kernel code means no 3D synthesis, and we do not infer
   it. Every hardware claim in the paper is scoped to 4D versus 5D. Nothing
   needed from you.
2. **The Cortex-A9 run: skip it.** I thought about this one. Beating the
   board's own weaker CPU on wall clock invites a reviewer to say we picked a
   soft baseline. Losing the mean to a 4.96 GHz i7 while winning the worst case
   is the stronger story, and it is the honest one. If it ever becomes cheap to
   run, it goes in the journal version, not this paper.
3. **Board timing: agreed.** Co-simulation is the timing authority. The board
   run appears strictly as functional verification, 2,307 requests PASS, and
   the paper says so in exactly those terms.

**What I need from you on the draft, in priority order.**

1. **Section IV.** It describes your opt-v1 kernel: two rules per clock, II=1,
   no early exit, fixed trip count, thus 12 + n/2 cycles with zero jitter by
   construction. Confirm that description matches the kernel you synthesized.
   The no-early-exit point carries the determinism argument, so it has to be
   right.
2. **Tables 1 and 2.** Every hardware number traces to your package. Spot-check
   them against your own reports.
3. **Related work.** Two independent literature sweeps found no prior hardware
   implementation of NGAC anywhere. We cite Huffmire, Fiorin, and the
   bit-vector classification line as the honest ancestors. If you know FPGA
   prior art we missed, this week is the time to say so.

**Authorship is final:** Karim, Faruque, Badawy, Sitharaman, Gupta. You are
second author; you produced the synthesis, co-simulation, and board data. Two
confirmations with your reply. First, I have you as New Mexico State
University, per your LinkedIn and ResearchGate; tell me if that is wrong or if
you want it rendered differently. Second, confirm Hameed is happy with the
ordering and with Florida International University as his affiliation.

The manuscript is due next week. Comments by Tuesday give me time to fold them
in without rushing the final pass.

Thanks again. Your package and your answers are the reason this paper exists in
its current shape.

Hassan

---

## Notes before sending

- Attach the current `paper/main.pdf` (post reviewer-2 fixes, revision 7
  abstract).
- cc Hameed so the ordering confirmation happens in one thread.
- If Omar pushes back on skipping the A9 run, the fallback position is
  recorded in `docs/canonical-context.md`: secondary embedded-realism data
  point, never the primary comparison.
- The opt-v1 reconciliation into `fpga/hls/src/` stays our task, not his; the
  measured source is preserved in the package, so nothing blocks on it.
