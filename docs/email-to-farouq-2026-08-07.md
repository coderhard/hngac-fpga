# Email draft to Farouq — 2026-08-07

**Subject:** H-NGAC package verified and committed — two asks before the manuscript

---

Farouq,

The evidence package arrived and it is excellent work. I have gone through it end to
end and committed it to the repo. Two notes before the asks.

**It verifies.** I re-ran your own extraction scripts against the raw reports and every
derived table in `results/README.md` reproduces exactly. I also diffed the 4D and 5D
synthesis sources: they differ by exactly one term, `prov_ok`, on the same part, clock,
testbench and corpus. That makes it a properly controlled comparison, which is what
makes the central claim defensible. Reviewers will look for exactly that.

**The result is the paper.** 4D and 5D resolving in identical cycles at every policy
size, with min equal to avg equal to max, is the finding. The closed form falling out
at `12 + n/2` cycles is better than I expected: we can state the worst case
analytically rather than empirically. Your call to measure in cycles rather than
wall-clock time was the right one, and it is what makes the comparison meaningful
against an embedded target.

The abstract goes in today. The manuscript is due next week. Two asks, in priority
order.

**Ask 1, high priority: a 3D synthesis run.**

Our headline claim has been that 3D, 4D and 5D all resolve at the same cost. We can
only support 4D versus 5D right now, because 3D was never synthesized. This is the one
gap that materially weakens the paper, and it is cheap: strip the `state_ok` and
`prov_ok` terms from `check_rule` in the opt-v1 kernel, run `hls_csynth.tcl`, send me
the `csynth.rpt`. No co-simulation and no board work needed. If the cycle count and II
match 4D and 5D, we get the full three-way claim back. If it does not match, we need to
know that before a reviewer tells us.

**Ask 2, lower priority but high value: a software baseline on the board's own ARM core.**

Right now our software numbers come from a 4.96 GHz i7-12800H while the fabric runs at
100 MHz. On cycles we win. On wall-clock mean the i7 beats the fabric by about 19x at
500 rules, and a reviewer will do that arithmetic immediately. Our answer is that the
FPGA gives a bounded worst case rather than a faster average, and that argument holds.
But it would be much stronger if we also ran the same benchmark on the PYNQ-Z1's own
Cortex-A9, which is the CPU an actual embedded deployment would use. The board is
already up and the benchmark builds with plain g++. If you can run
`run_perf_benchmark.sh` there, we get an apples-to-apples comparison where the fabric
wins on both cycles and wall clock.

**Three smaller things.**

1. Your benchmark harness takes rule count and model name as arguments and emits a
   `CYCLES|` line. The version in our repo does neither. Can you send me that diff, or
   push it, so the harness in the repo is the one that produced the numbers?
2. The log contains two disagreeing software cycle figures: `extract_sw_cycles.py`
   derives 70 cycles for 3D at 4 rules from nanoseconds times clock, while the
   benchmark's own `CYCLES|` line says 82.29 for the same case. Which do you consider
   the right method? I need to state one in the paper.
3. Is on-board timing feasible at all, even roughly? The board run proves functional
   correctness, which is valuable, but we currently cannot cite any measured hardware
   latency from silicon, only from co-simulation. Not a blocker if it is not practical.

**Authorship.** You produced all the synthesis, co-simulation and board data, so you
belong on the author list. Please send me your full name as you want it to appear and
your affiliation, and let me know your preference on author order.

Thanks again. This closed a dependency that had the paper parked since April.

Hassan

---

## Notes before sending

- Confirm Farouq's preferred name spelling and whether the Badawy lab affiliation is
  the right one to list.
- If the 3D run is not possible before the manuscript deadline, the fallback is to
  scope the claim to 4D versus 5D throughout. That is already the wording in
  `docs/canonical-context.md`, so nothing breaks; the paper is just slightly less
  striking.
- Ask 2 is genuinely optional. Decide whether to spend his week on it or on the 3D run
  if he only has time for one. The 3D run is worth more.
