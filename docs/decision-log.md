# Decision Log

## 2026-08-07

### Decision

Hardware claims move from placeholder to measured fact, scoped to what the
evidence actually supports: 4D vs 5D only, free in time, +11.4% LUT in area.

### Reason

The Badawy/HW-team dependency that blocked the paper since April closed on
2026-08-05. `hngac-package-from-farouq/` contains Vitis HLS 2025.2 csynth reports,
passing Verilog co-simulation with per-call transaction latencies, a PYNQ-Z1 board
verification run, and perf-counter software baselines. Every derived table was
re-run from the raw reports on 2026-08-07 and reproduces exactly.

The 4D and 5D builds differ by exactly one term (`prov_ok`) on the same part, clock,
testbench and corpus, so the comparison is properly controlled.

### Alternatives considered

- Keep the placeholder framing (rejected: the evidence exists and the abstract is
  due today)
- State the original "3D/4D/5D equal LUT-stage count" claim as confirmed (rejected:
  3D was never synthesized, and 5D costs 11.4% more LUT than 4D, so the original
  wording is not supported)

### Impact on code/tests/paper

- `docs/canonical-context.md` gains an authoritative Hardware Results section and
  five new framing constraints.
- The paper must say "free in time, nearly free in area," never "zero hardware cost."
- The paper must not claim the FPGA is faster than the CPU on mean latency. It is
  not, by ~19x at 500 rules. The argument is boundedness and zero jitter.
- The board test is functional verification only and must never be cited as timing.
- 3D synthesis remains open work and is the one cheap experiment that would restore
  the original three-way claim.

## 2026-08-07

### Decision

The HW evidence package is committed as an immutable record, excluding the two
full repo mirrors it ships with.

### Reason

The standing portfolio rule requires experiment results and methodology to live in
git. The package also contains `kernel/4d/` and `kernel/5d/hngac-fpga/`, which are
copies of this repo at an older commit; committing them would duplicate the working
tree, invite edits to the wrong copy, and carry a `scripts/aws/fpga-dev.env` that
this repo deliberately gitignores.

### Alternatives considered

- Commit the package whole (rejected: 2.9 MB of duplicated working tree, and a
  co-author will eventually edit the wrong `hngac_kernel.cpp`)
- Commit only `results/` (rejected: loses the exact measured software source and
  the only Vivado place-and-route data we have)

### Impact on code/tests/paper

- Committed: `results/`, `synthesis/`, `board-test/`, plus
  `kernel/5d/hngac-fpga/fpga/hls/` (exact source measured) and
  `kernel/4d/hngac-fpga_4d_hw_results/` (Vivado place-and-route, WNS +2.170 ns).
- Excluded via `.gitignore`: the repo mirrors, 255 `*:Zone.Identifier` files, the
  duplicate zip, and `*.backup`.
- `hngac-package-from-farouq/PROVENANCE.md` records toolchain, part, dates, the
  full include/exclude rationale, and reproduction commands.
- Open: the opt-v1 two-rules-per-clock optimization and Farouq's modified
  benchmark harness still need reconciling back into `fpga/hls/`.

## 2026-04-26

### Decision

Hardware-dependent claims must remain placeholders until Badawy/HW-team synthesis
or hardware reports are available.

### Reason

The repo currently has local C++ validation and software benchmark evidence, but
does not yet have Vitis HLS synthesis reports, co-simulation logs, LUT/timing
tables, or deployed hardware measurements. Stating zero LUT-stage overhead,
fabric latency, or hardware correctness as completed evidence would overclaim
the current project state.

### Alternatives considered

- Keep aspirational hardware claims in final-result language (rejected because
  the evidence is pending)
- Remove hardware framing entirely (rejected because it remains the core IPCCC
  contribution once synthesis evidence arrives)

### Impact on code/tests/paper

- README, paper draft, and canonical docs should frame zero-cost dimensionality
  as the hardware claim to be confirmed by synthesis.
- Local benchmark and attack-demo results remain usable as software evidence.
- HLS result tables stay as placeholders until HW-team data arrives.

## 2026-04-26

### Decision

The reviewer-fair flattened 5D direct lookup baseline is part of the local
software comparison story.

### Reason

A fully materialized 5D allow-set answers the strongest reviewer objection:
whether H-NGAC is only faster than graph/state baselines because it avoids a
direct decision lookup formulation. The comparison must include lookup latency,
memory footprint, and build/reload cost because the flattened representation
trades compact policy representation for precomputed decisions.

### Alternatives considered

- Compare only against RBAC, NGAC-DAG, and SQLite (incomplete for reviewer fairness)
- Treat flattened lookup as implementation detail only (rejected because it is a
  meaningful baseline)

### Impact on code/tests/paper

- Benchmark docs and sweep scripts must parse/report `Flattened 5D direct lookup`.
- A fresh canonical run is needed before paper tables include flattened 5D numbers.
- Scaling experiments should report policy size, memory, build/reload cost, and
  latency distribution.

## 2026-04-20

### Decision

5D provenance-aware extension is now full paper scope. Venue changed from IEEE SOCC to IEEE IPCCC 2026.

### Reason

The Attack Class 2 ROS2 demo (April 18) produced clean empirical results: 18,878 injections blocked at 100%, 0 false positives. The zero-cost hardware claim (5D resolves in the same LUT stage count as 3D on UltraScale+) is the paper's key finding and requires 5D to be in scope, not follow-on. The IPCCC venue is a better fit for the hardware-security-for-CPS framing.

### Alternatives considered

- Keep 5D as future work / R-003 (abandoned — too strong a result to withhold)
- Submit to SOCC (venue changed due to better alignment of IPCCC scope)

### Impact on code/tests/paper

- Kernel, testbench, and benchmark already implement 5D (completed April 13–18).
- HLS synthesis must be run for both 4D and 5D kernels (Badawy lab task).
- Paper scope: 3 attack classes / 3 dimensions / 1 hardware primitive is now the central framing.
- Prior decision log entries describing 5D as "reserved hook only" are superseded by this entry.

## 2026-04-13

### Decision

SOCC implementation scope is 4D state-aware H-NGAC, not a 3D hardware-only reproduction and not a full 5D provenance implementation.

### Reason

The 3D-on-FPGA story is too thin by itself for a 6-page paper. The 4D state extension adds algorithmic novelty while still keeping the implementation tractable on the current schedule.

### Alternatives considered

- 3D hardware-baseline-only paper
- full 5D provenance-aware implementation now

### Impact on code/tests/paper

Kernel interfaces, tests, and evaluation must all include state-aware behavior. Provenance remains a reserved hook only.

## 2026-04-13

### Decision

State is modeled as per-object state in the authorization request.

### Reason

This gives the paper a stronger safety-control story than a single global state bitmask, while still staying implementable as a fixed-size HLS-friendly input.

### Alternatives considered

- global system-state bitmask
- hybrid global/object state model

### Impact on code/tests/paper

The request type must carry object-state bits, and tests must demonstrate authorization changes when object state changes under the same subject/object/attribute query.

## 2026-04-13

### Decision

The codebase will reserve a provenance field in request and policy interfaces but will not activate provenance checks in the first implementation cycle.

### Reason

This keeps the 4D scope tight for SOCC while preserving a clean migration path to a later 5D provenance-aware variant.

### Alternatives considered

- omit provenance entirely for now
- implement 5D provenance immediately

### Impact on code/tests/paper

Types and function signatures should not need reshaping later to add provenance, but tests and benchmarks in this cycle remain 4D-focused.

## 2026-04-13

### Decision

The HLS kernel interface uses a fixed-size `AuthorizationRequest` carrying subject id, object id, required attribute mask, object-state mask, and a reserved provenance field.

### Reason

This keeps the top-level interface narrow, HLS-friendly, and aligned with the 4D paper framing while preserving the future 5D extension point.

### Alternatives considered

- separate scalar arguments for every request field
- no provenance field until a later refactor
- global state input instead of per-request object state

### Impact on code/tests/paper

The testbench and future benchmarks should treat request construction as the stable interface boundary. Vitis HLS work can now target a single request-plus-policy-array kernel shape.

## 2026-04-13

### Decision

Local pre-AWS evaluation uses a single comparison harness that measures 3D baseline, 4D state-aware kernel, and RBAC plus external-state-lookup on the same request set.

### Reason

This keeps the paper’s two baseline comparisons aligned to one benchmark method and avoids incomparable measurements from different harnesses.

### Alternatives considered

- separate microbenchmarks for each comparison path
- only 3D vs 4D before AWS
- only RBAC vs 4D before AWS

### Impact on code/tests/paper

The repo now has one local comparison executable that can produce mean, p99, and max latency for all three paths before any FPGA deployment work. Reported smoke numbers must be labeled as local software measurements, not hardware results.

## 2026-04-13

### Decision

The Vitis HLS script uses a configurable work directory and defaults to `/tmp/hngac-fpga-hls` instead of creating synthesis projects inside the repo checkout.

### Reason

The repo lives on a Windows-backed filesystem in this environment, and local tool output should not depend on write access or pollute the tracked tree.

### Alternatives considered

- keep HLS work products under `fpga/hls/work`
- require a manually supplied output path every time

### Impact on code/tests/paper

Future synthesis runs can execute without mutating the repo tree. This makes the HLS path more portable across local environments and reduces noise in the working copy.

## 2026-04-13 07:52 CDT

### Agent

Codex (GPT-5.4)

### Decision

The canonical local SOCC benchmark is a single five-model harness covering RBAC hash map, NGAC-DAG traversal, 3D bitmask, 4D state-aware bitmask, and RBAC plus modeled external-state lookup.

### Reason

The benchmark-gap analysis should be backed by executable code, not a placeholder list. Keeping the local comparison in one harness also reduces the risk of incomparable measurement methods.

### Alternatives considered

- leave RBAC hash map and NGAC-DAG as unimplemented paper-only baselines
- split every baseline into a separate executable

### Impact on code/tests/paper

The paper can now report a five-model local software comparison from one executable with one request mix. The remaining benchmark-design question is whether the RBAC+state path stays modeled or becomes a real SQLite-backed lookup.

## 2026-04-13 07:52 CDT

### Agent

Codex (GPT-5.4)

### Decision

`CLAUDE.md` remains a stable shared-context file, while `docs/coordination-board.md` and `docs/status-log.md` carry live ownership and execution state.

### Reason

Putting volatile task ownership into `CLAUDE.md` would cause confusion across concurrent agents. Stable repo facts and live coordination need separate homes.

### Alternatives considered

- put both durable repo context and live task claims into `CLAUDE.md`
- keep all coordination implicit in commit history only

### Impact on code/tests/paper

Future concurrent sessions should update `docs/coordination-board.md` before editing claimed files and keep `CLAUDE.md` limited to durable repo context.

## 2026-04-13 08:04 CDT

### Agent

Codex (GPT-5.4)

### Decision

The unified benchmark includes both a modeled RBAC+state lookup path and an optional SQLite-backed empirical lookup path when `SQLite3` is available at configure time.

### Reason

This keeps the benchmark usable on minimal machines while still allowing a real in-process external-state baseline on systems that already have SQLite3 installed.

### Alternatives considered

- replace the modeled path entirely with SQLite
- make SQLite mandatory for the benchmark build

### Impact on code/tests/paper

The paper can compare 4D H-NGAC against both a controllable modeled lookup delay and a real local SQLite-backed lookup. Benchmark framing must state clearly which one is being reported in each figure or table.
