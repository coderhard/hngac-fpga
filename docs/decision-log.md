# Decision Log

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
