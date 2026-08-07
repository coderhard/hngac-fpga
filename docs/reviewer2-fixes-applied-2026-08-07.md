# Reviewer-2 fixes as applied — before and after

All six fixes from `docs/reviewer2-adversarial-review-2026-08-07.md`, applied to
`paper/main.tex` in HSK v1.2 academic voice. This file is the audit trail: every
change verbatim, with the voice rule it obeys. Compile state after application:
7 pages, zero overfull boxes, abstract 238 words, no new semicolons, no
em-dashes, no forbidden vocabulary.

---

## Fix 1 — policy-vs-kernel separation (Attack 1, critical)

**Abstract, before:**
> ...the kernel blocks 18,878 ROS 2 injection attempts with no false positives
> and verifies functionally on PYNQ-Z1 silicon.

**Abstract, after:**
> ...the five-dimensional policy blocks 18,878 ROS 2 injection attempts in a
> software gatekeeper with no false positives, and the kernel verifies
> functionally on PYNQ-Z1 silicon.

**VI-D opening, before:** "A gatekeeper node enforcing the five-dimensional
policy fronted an actuator topic..."

**VI-D opening, after:** "A **software** gatekeeper node **executing the same
five-dimensional decision function** fronted an actuator topic..."

**VI-D, added after the false-positive sentence:**
> This demo and the board run carry two separate claims. The demo establishes
> what the five-dimensional policy semantics block under sustained adversarial
> load. The silicon run establishes that the synthesized kernel computes
> identical decisions on all 2,307 corpus requests. Neither claim substitutes
> for the other.

Voice note: declarative claim, two explanatory sentences, short closer — the
profile's signature rhythm. The staccato closer is deliberate; do not smooth it.

**Divergence flag:** the abstract submitted to the portal earlier today still
reads "the kernel blocks." If the portal allows abstract edits before the
manuscript deadline, paste the corrected sentence there too.

## Fix 2 — NGAC subset scoping (Attack 2, critical)

**Section III, added after "They add no traversal, no lookup and no branch.":**
> One scope statement belongs here. We compile the association subset of
> INCITS 565, that is, the rules that grant. Prohibitions, obligations and
> administrative relations are not compiled, and Section VIII bounds our
> claims accordingly.

**Limitations, new item:**
> Seventh, the kernel compiles only the association subset of INCITS 565.
> Prohibitions, obligations and administrative relations are not implemented,
> so no claim here extends beyond rules that grant.

Voice note: "that is," is the profile's primary restatement clarifier, used in
its natural position.

## Fix 3 — operating-point boundary (Attack 3, high)

**Limitations, new item:**
> Eighth, the zero-cycle result is demonstrated at one operating point,
> 100 MHz on a Zynq-7020, where both kernels close timing with identical
> +0.33 ns slack. At an aggressive clock target the added dimension could
> force an additional pipeline stage, which is exactly the boundary the
> hypothesis of Section III predicts. Characterizing that frequency ceiling is
> future work.

This converts the reviewer's strongest technical attack into evidence that the
Section III hypothesis has predictive content.

## Fix 4 — generalization softened (Attack 4, high)

**Abstract final line:** "A security dimension is therefore free in time..."
became "**The added** security dimension is therefore free in time..."

**Introduction ¶5:** "Security dimensionality is free in time and nearly free
in area, which converts..." became "They are free in time and nearly free in
area, which converts..." (the antecedent is "the added dimensions" in the
previous sentence).

The title stays. It names the finding, and the hypothesis supplies the
mechanism that licenses it. The conclusion already said "The added dimension"
and needed no change.

## Fix 5 — published numbers carry the motivation (Attack 5, medium-high)

**Introduction ¶2, before:**
> In our own prior measurements, Open Policy Agent averaged 271.491 us per
> decision and missed 13 real-time deadlines in a single run [TS-NGAC, under
> review], and a software authorization path that averaged 1.14 us produced a
> single 157 us scheduling outlier [DCAS].

**After:**
> In our own published measurements, an OPA edge deployment took 1 to 5 ms per
> decision and an XACML decision point near 50 ms, while a software
> authorization path averaging 1.14 us still produced a single 157 us
> scheduling outlier [DCAS]. A further evaluation, now under review, measured
> OPA at a 271.491 us mean with 13 missed real-time deadlines [TS-NGAC].

The load-bearing motivation now rests on the published DCAS paper; the
under-review numbers corroborate instead of carrying.

## Fix 6 — role-explosion concession (Attack 6, medium)

**VI-C, added before "It might be tempting...":**
> However, an RBAC deployment could encode state into its role set, a
> maintenance operator role distinct from a normal operator role. Sixteen
> state bits and three source types multiply that role count combinatorially,
> and the honest alternative, RBAC consulting an external state store, cost
> 20.6 times the four-dimensional path when we measured it against an
> in-process SQLite store.

Numbers check: 20.6x is RBAC+SQLite (376.83 ns) over 4D (18.27 ns), the
April 18 canonical run, per `docs/canonical-context.md`. It is the empirical
figure; the 6,674x modeled figure stays banned.

---

## Post-application audit

| Check | Result |
|---|---|
| Compile | 7 pages, 0 overfull, 0 undefined citations |
| Abstract | 238 words (220-250 band for attack+defense papers) |
| Em-dashes | 0 |
| Semicolons | 8, unchanged; none added by the fixes |
| Forbidden vocabulary | none |
| Staccato preserved | yes ("Neither claim substitutes for the other.") |
| Accuracy rules | no 3D hardware claim, no speedup framing, board = functional only, "free in time, nearly free in area" intact |

Attacks 7 through 10 required no edits; the responses are recorded in the
reviewer-2 report for the rebuttal file.
