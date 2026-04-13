# Parallel Agent Execution Brief

This brief splits the 4D H-NGAC build into disjoint ownership areas so multiple agents can work in parallel without stepping on each other.

## Shared rules

- The source of truth is `docs/implementation-plan-4d-hngac.md`.
- Every major decision must be recorded in `docs/decision-log.md`.
- Do not change the 4D scope, per-object-state model, or reserved 5D provenance hook without updating the decision log.
- Keep the HLS kernel path free of ROS2 dependencies, dynamic allocation, and `std::vector`.
- Do not rewrite imported DCAS baseline files unless the task explicitly requires it.

## Agent 1: Core kernel and types

Owns:

- `fpga/hls/include/`
- `fpga/hls/src/hngac_kernel.cpp`

Deliverables:

- 4D request and policy types
- named state bits
- reserved provenance hook in interfaces
- 4D authorize logic
- compatibility-preserving helper utilities for fixed-size bitmasks

Do not edit:

- local benchmark harnesses outside `fpga/hls/`
- docs other than brief implementation notes if strictly necessary

## Agent 2: Testbench and local validation

Owns:

- `fpga/hls/tb/`
- `fpga/hls/CMakeLists.txt`

Deliverables:

- expanded HLS testbench for 4D state logic
- regression cases for wrong subject/object/attribute
- state-specific allow/deny cases for all four named states
- combinations and edge-case coverage for out-of-range ids and empty masks

Do not edit:

- core kernel semantics unless coordinating with Agent 1

## Agent 3: Baseline comparison harnesses

Owns:

- new local benchmark artifacts for 3D vs 4D and RBAC+lookup comparisons
- any new software-only comparison code under a dedicated benchmark path

Deliverables:

- 3D baseline benchmark path
- 4D benchmark path
- RBAC plus external-state-lookup benchmark path
- output shape suitable for paper tables and quick local inspection

Do not edit:

- HLS kernel signatures without coordinating with Agent 1

## Agent 4: Docs and research traceability

Owns:

- `README.md`
- `fpga/hls/README.md`
- `docs/decision-log.md`

Deliverables:

- repo readme updates reflecting 4D scope
- HLS readme updates reflecting 4D kernel behavior
- running decision log with dated entries as implementation choices are locked

Do not edit:

- kernel or benchmark code except for doc examples and command snippets

## Recommended order

1. Agent 4 writes the initial decision-log entries for scope lock.
2. Agent 1 lands the 4D request and policy model.
3. Agent 2 updates the testbench to match the new kernel interface.
4. Agent 3 adds the local comparison baselines after the 4D kernel interface stabilizes.
5. After local tests pass, the HLS flow can be rerun and prepared for AWS.

## Merge discipline

- Merge Agent 1 before Agent 3 if benchmark code depends on the new 4D interface.
- Merge Agent 2 only after the Agent 1 interface is stable for that round.
- Agent 4 can update docs in parallel as long as it does not restate unmerged behavior as final.
