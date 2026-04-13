# 4D State-Aware H-NGAC Plan

## Summary

Pivot `hngac-fpga` from a 3D HLS baseline to a 4D state-aware H-NGAC implementation, while preserving a clean extension point for later 5D provenance. The first implementation will model state as per-object state and will evaluate both comparisons as first-class paper results:

- 3D H-NGAC vs 4D H-NGAC overhead
- 4D H-NGAC vs a role-table-plus-external-state-lookup baseline

All major technical choices will be recorded in a single running decision log in the repo. No AWS work happens until the 4D local kernel, software tests, and local synthesis path are stable.

## Implementation Changes

### 1. Decision logging and repo conventions

- Add a repo-tracked running log at `docs/decision-log.md`.
- Use one dated entry per meaningful choice with this fixed format: `Decision`, `Reason`, `Alternatives considered`, `Impact on code/tests/paper`.
- Log at minimum: 4D scope lock, per-object state choice, 5D hook design, baseline selection, HLS top-function shape, state semantics, and any evaluation simplifications.
- Update the root `README.md` so it explicitly says this repo is now the 4D SOCC work repo, with `fpga/hls/` as the active implementation path and `docs/decision-log.md` as the running research log.

### 2. Core model and interfaces

- Replace the current 3D conceptual kernel in `fpga/hls/` with a 4D model:
  - `subject`
  - `object`
  - `attribute`
  - `state`
- Keep the 5D provenance hook in types/interfaces, but do not activate provenance logic in this cycle.
- Implement fixed-size HLS-friendly types only: no `std::vector`, no dynamic allocation, no ROS dependencies in the kernel path.
- Model state as a bitmask attached to the object context of the request, not as a global system-state singleton.
- Define the first four state bits explicitly and freeze them as named constants:
  - `battery_low`
  - `maintenance_mode`
  - `safety_interlock`
  - `calibration_required`
- Keep policy records fixed-size and HLS-friendly. Each rule should include required state constraints in the same style as subject/object/attribute matching.
- Reserve but do not yet use a provenance field in the policy/request types. The field should exist so 5D can be added later without reshaping the core API.

### 3. Kernel behavior

- The active HLS top function should authorize based on one request plus one bounded policy array.
- Request shape should include:
  - subject id
  - object id
  - required attribute mask
  - object state mask
  - reserved provenance field
- Policy rule shape should include:
  - subject mask
  - object mask
  - attribute mask
  - required state mask
  - reserved provenance mask or field
- Matching rule for 4D:
  - subject must match
  - object must match
  - requested attributes must be contained
  - request object-state must satisfy the rule’s required state mask
- Preserve the current empty-required-mask-passes behavior unless a test demonstrates that this breaks the intended semantics for state. If it does, log and change it intentionally.

### 4. Baseline implementations for evaluation

- Keep a 3D baseline kernel path in the repo for direct apples-to-apples overhead measurement.
- Add a software-only comparison baseline representing role-based authorization plus external state lookup.
- Do not claim absolute impossibility of RBAC expression. The implemented comparison should support the narrower paper claim:
  - 4D H-NGAC avoids role explosion and external state-query overhead for these safety constraints.
- The RBAC/state-lookup baseline should be intentionally simple and defensible:
  - role/object/permission table lookup
  - separate state fetch/check step
  - timing measured as combined authorization plus state-query path
- Keep the comparison baseline outside the HLS kernel path; it is an evaluation artifact, not the FPGA design.

### 5. Local evaluation before AWS

- Add local software tests for 4D state transitions:
  - same subject/object/attribute, different object-state bitmasks produce different permit/deny outcomes
  - each of the four state bits has at least one allow and one deny case
  - mixed-state cases behave deterministically
- Add local benchmark paths for:
  - 3D baseline
  - 4D kernel
  - RBAC plus state-lookup baseline
- Measure locally before AWS:
  - mean latency
  - p99 latency
  - max latency
  - relative overhead of 4D vs 3D
  - relative latency of 4D vs RBAC plus state lookup
- Do not promise a less-than-5-percent overhead result ahead of measurement. Treat it as a hypothesis to test.

### 6. HLS and synthesis preparation

- Update `fpga/hls/README.md` to describe the 4D kernel and the reserved provenance hook.
- Update the testbench so it validates:
  - baseline allow
  - wrong subject
  - wrong object
  - wrong attribute
  - failing state constraint
  - passing state constraint
  - combinations of the four named states
- Keep the Vitis HLS script parameterized by environment variables for part and clock.
- Local success gate before AWS:
  - 4D kernel compiles and passes its testbench
  - 3D and 4D local measurements are collected
  - RBAC plus state-lookup baseline measurements are collected
  - decision log is up to date through kernel-shape lock and benchmark-method lock

## Test Plan

- Unit tests for bitmask helpers and state-mask containment logic.
- 3D regression test to prove the old baseline behavior is unchanged when state is not involved.
- 4D authorization tests covering all four named states individually.
- Cross-product tests where one state bit changes and the authorization result changes as expected.
- Negative tests for out-of-range subject/object ids and invalid masks.
- Benchmark runs for 3D vs 4D overhead using identical policy sizes and request counts.
- Benchmark runs for 4D vs RBAC plus state-lookup using identical scenario sets.
- Local HLS testbench pass before any Vitis synthesis run.
- First synthesis acceptance criteria:
  - builds successfully
  - fixed top-function interface
  - no dynamic structures in kernel path
  - latency/resource report generated for the 4D kernel

## Assumptions and defaults

- SOCC scope is 4D now, not full 5D, but the code will reserve a provenance field so 5D can be added without refactoring core interfaces.
- State is per-object state in the request, not a global singleton.
- The four named state bits are the only state conditions in this cycle.
- The paper evaluation will prioritize both comparisons: 3D vs 4D and 4D vs RBAC plus lookup.
- AWS is for final hardware evidence only; local kernel/test/benchmark stabilization comes first.
- The decision log is a single running markdown file, not per-decision ADR files.
