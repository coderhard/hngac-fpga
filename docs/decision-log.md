# Decision Log

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
