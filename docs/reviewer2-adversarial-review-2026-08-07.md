# Reviewer 2 — adversarial review of the IPCCC 2026 draft

Task 3 of `docs/manuscript-agent-prompt.md`, run against `paper/main.tex` as of
commit `a07e767` + the SOTA integration. Posture: hostile reviewer who has
opened the synthesis reports, read the DCAS paper, and wants to reject.

Attacks ranked by probability of sinking the paper. Each entry: the attack,
whether it lands after I tried to refute it, the best response, and the
recommended edit. Per the prompt's output rules I have NOT edited the
manuscript; the fix list at the end is ready to apply on your word.

---

## Attack 1 — "Your FPGA never blocked anything." (CRITICAL, lands as written)

**The attack.** The abstract says "the kernel blocks 18,878 ROS 2 injection
attempts with no false positives and verifies functionally on PYNQ-Z1 silicon."
The reviewer reads Section VI-D and discovers the ROS 2 evaluation ran the 5D
decision function in a *software gatekeeper node* on WSL2 (min callback 23 ns —
impossible for a 100 MHz fabric round trip, which the reviewer will compute in
seconds). The FPGA's only live run was functional verification with canned
requests. The abstract's subject, "the kernel," did not do the blocking. That is
the kind of conflation that flips a review from major-revise to reject, because
it looks like inflation rather than imprecision.

**Does it land?** Yes, on the abstract and on VI-D's silence. Contribution (c)
in the introduction already says "the five-dimensional policy blocks," which is
correct. The abstract and VI-D need the same discipline.

**Best response / fix.** Abstract: "the five-dimensional policy blocks 18,878
ROS 2 injection attempts in a software gatekeeper with no false positives, and
the kernel verifies functionally on PYNQ-Z1 silicon." VI-D: open with "A
software gatekeeper node executing the same five-dimensional decision function"
and add one sentence: the demo establishes what the *policy semantics* block;
the silicon run establishes that the kernel computes identical decisions on all
2,307 corpus requests. Two claims, cleanly separated, both true.

## Attack 2 — "This is not NGAC. It is a positive-authorization subset."
(CRITICAL, lands)

**The attack.** INCITS 565 defines associations, prohibitions, obligations, and
administrative commands. The paper compiles associations into bitmasks and says
"compiles NGAC policy graphs" throughout. Where are prohibitions (deny
semantics)? Obligations (state-changing side effects)? Dynamic policy update,
the standard's whole point? The reviewer says: you implemented first-match
permit over precompiled allow-sets and branded it NGAC.

**Does it land?** Yes. Nothing in the draft scopes which NGAC features are
compiled. The assumption "policy updates are outside our present scope" is
stated, but prohibitions and obligations are never mentioned.

**Best response / fix.** One scoping sentence in Section III after Definition 3:
"We compile the association subset of INCITS 565; prohibitions, obligations,
and administrative relations remain future work, and Section VIII bounds the
claim accordingly." Plus one clause in Limitations. This costs two sentences
and removes the reviewer's strongest conceptual weapon. Do not try to argue the
subset is complete; it is not, and the honest scope is defensible because the
compilation step (DCAS) was already published under the NGAC name.

## Attack 3 — "Zero-cycle dimensionality is an artifact of your slow clock."
(HIGH, partially lands)

**The attack.** The fifth dimension adds a 32-bit AND-and-compare to a
combinational stage. At 10 ns per cycle with +0.33 ns slack on both kernels,
of course it fits. Synthesize at 250 MHz and the 5D kernel may need another
pipeline stage while 4D does not; then dimensionality is NOT free in time. The
"key finding" is a property of one operating point, not of the architecture.

**Does it land?** Partially. The Hypothesis in Section III already states the
condition (free iff the dimension maps to combinational inputs within an
existing stage), and identical slack on both kernels shows the added term is
not on the critical path at this operating point. But the paper never says the
property is operating-point-conditional, and a reviewer who finds the boundary
before we admit it owns the narrative.

**Best response / fix.** One sentence in Limitations: the result is
demonstrated at 100 MHz on a Zynq-7020, where both kernels carry identical
positive slack; at aggressive clock targets the added dimension could force an
additional stage, which is exactly the boundary the Hypothesis predicts, and
characterizing that frequency ceiling is future work. This converts the attack
into evidence that the Hypothesis has predictive content.

## Attack 4 — "One measured increment, universal claim." (HIGH, partially lands)

**The attack.** "Security dimensionality is free in time" is a law induced from
exactly one dimensional increment (4D to 5D). 3D was never synthesized, by the
authors' own admission. The title generalizes ("Security Dimensionality at Zero
Time Cost"); the conclusion generalizes ("A security dimension is therefore
free in time"). n=1.

**Does it land?** Partially. The body scopes carefully (4D vs 5D everywhere,
Limitations names the missing 3D), and the Hypothesis supplies a mechanism
rather than a curve fit. But the conclusion's "therefore" earns a red pen.

**Best response / fix.** Keep the title (it names the finding, and the
mechanism supports it). Soften the conclusion's generalization by one word:
"The added security dimension is free in time and nearly free in area," and
let the Hypothesis carry the general claim as a stated, testable condition.
Response to the reviewer: the claim is mechanistic, not inductive; the
mechanism (one more AND input in an existing stage) is stated, tested at six
policy sizes, and bounded in Section VIII.

## Attack 5 — "Your headline motivation number is unverifiable." (MEDIUM-HIGH,
partially lands)

**The attack.** The introduction's strongest motivation numbers (OPA 271.491 us
mean, 13 deadline misses) cite an unpublished manuscript "under review." A
reviewer cannot check them, and citing your own unpublished work for the load-
bearing motivation looks circular.

**Does it land?** Partially. The published DCAS paper carries OPA edge
deployment at 1 to 5 ms and XACML at roughly 50 ms, which are worse for the
engines and fully citable.

**Best response / fix.** Reorder the introduction sentence to lead with the
published numbers: OPA 1 to 5 ms at the edge and XACML near 50 ms (DCAS,
published), then the 271.491 us / 13 misses as corroborating detail from work
under review. Same argument, review-proof foundation.

## Attack 6 — "The over-authorization table is circular." (MEDIUM, mostly
refuted)

**The attack.** You built a corpus containing state- and provenance-violating
requests, then report that models without state and provenance dimensions admit
them. Tautology. A competent RBAC deployment would encode state into roles
(maintenance_operator vs normal_operator) and deny them.

**Does it land?** Mostly no, but the role-encoding alternative deserves a
sentence. Encoding 16 state bits and 3 provenance types into RBAC roles
multiplies the role set combinatorially (the classic role-explosion argument),
and the corpus is not hidden: Section V states its construction. The RBAC+state
external-lookup baseline (SQLite, 20.6x over 4D) already represents the honest
"RBAC checks state somewhere else" design.

**Best response / fix.** Half a sentence in VI-C acknowledging the role-
explosion alternative and pointing at the SQLite baseline as its measured cost.

## Attack 7 — "Zero jitter in a vacuum." (MEDIUM, conceded by design)

**The attack.** The bound covers the kernel between AXI handshakes in
co-simulation. A real SoC adds PS-PL crossing, AXI arbitration, and DMA
contention; no on-board timing exists. "Zero jitter" as deployed is unproven.

**Does it land?** As stated in the draft, no: Limitations already concedes no
on-board timing, and the claim is scoped to the kernel. The response to keep
ready: the kernel bound is the component-level term a system integrator sums;
bus-level determinism is a known, separately-solvable problem (the SoK's
architecture-level category), and the co-simulated bound is what synthesis
guarantees.

## Attack 8 — "Cycles are not a cross-platform unit." (MEDIUM, handled)

**The attack.** Comparing derived "cycles" on a 4.96 GHz out-of-order i7
against measured cycles on a 100 MHz in-order fabric treats incommensurable
units as commensurable. The honest units are wall clock (CPU wins 19x) or
boundedness (FPGA wins). The cycle framing flatters the FPGA.

**Does it land?** Mostly no: VI-E preempts the wall-clock inference explicitly,
the method section labels SW cycles as derived, and the dimensionality claim
(the paper's center) is within-platform, not cross-platform. Keep the SW-vs-HW
cycle ratios out of the paper (they live in the results README; the draft uses
one absolute pair, 685 vs 262, with labels). No edit needed; have the response
ready: cycles normalize away clock choice for embedded targets, and both other
units are reported.

## Attack 9 — "512 rules is a toy." (LOW-MEDIUM, handled)

**The attack.** Enterprise NGAC graphs hold thousands of nodes; 256 policy
nodes and 512 rules is embedded-scale.

**Response ready.** The primitive authorizes one platform's command surface,
not an enterprise directory; 256 subjects/objects per platform is realistic for
a robot, and scaling is an area trade (wider masks, banked scan) with latency
still closed-form. Limitations already names the caps. Optional: one clause on
the scaling path.

## Attack 10 — "Wilson bounds on deterministic logic are theater." (LOW)

**The attack.** The gatekeeper is deterministic policy evaluation, not a
stochastic detector; a 99.99% lower bound implies statistical uncertainty that
does not exist. Either it implements the policy or it does not.

**Response ready.** Correct, and that is why the paper frames the run as
implementation correctness under sustained adversarial load, with exact
denominators. The bounds cost nothing and preempt the opposite complaint (a
bare 100%). No edit.

---

## What survives untouched

The novelty claim (two independent search engines, bounded form, explicit
concessions), the honesty labels, the worst-case framing with the wall-clock
concession, determinism-by-construction, the closed form at six points, and
the scored rubric. Reviewer 2 does not get to say the numbers are wrong; every
attack above is about scope and wording.

## Recommended pre-submission fixes, in order

1. **Abstract + VI-D: policy-vs-kernel separation** (Attack 1). Two sentences.
   Non-negotiable.
2. **NGAC subset scoping sentence in III + Limitations clause** (Attack 2).
   Two sentences. Non-negotiable.
3. **Operating-point boundary sentence in Limitations** (Attack 3). One
   sentence.
4. **Conclusion: "The added security dimension"** (Attack 4). One word class.
5. **Introduction: lead motivation with published DCAS OPA/XACML numbers**
   (Attack 5). Reorder one sentence.
6. **VI-C: role-explosion half-sentence** (Attack 6). Optional but cheap.

Items 1 and 2 are the difference between major-revise and reject with a
hostile reviewer. Items 3 through 5 are the difference between major and minor.
