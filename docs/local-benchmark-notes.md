# Local Benchmark Notes

## 2026-04-13 five-model validation run

Command:

```bash
/tmp/hngac-fpga-build/hngac_compare_benchmark 2000 10000
```

Modeled configuration:

- iterations: 2,000
- RBAC external-state lookup delay: 10,000 ns
- warmup: 1,000 iterations per model
- scenario mix: 4 state-satisfying requests and 4 state-failing requests

Observed output:

- RBAC hash map mean: 22.248 ns
- RBAC hash map allowed count: 2,000 / 2,000
- NGAC-DAG traversal mean: 178.75 ns
- NGAC-DAG traversal allowed count: 2,000 / 2,000
- 3D baseline mean: 16.6525 ns
- 3D baseline allowed count: 2,000 / 2,000
- 4D state-aware mean: 22.2875 ns
- 4D state-aware allowed count: 1,000 / 2,000
- RBAC plus state lookup mean: 10,185.8 ns
- RBAC plus state lookup allowed count: 1,000 / 2,000
- computed 4D vs 3D mean overhead: 33.8388%
- computed 4D vs RBAC hash-map mean overhead: 0.177544%
- computed NGAC-DAG vs 4D mean slowdown: 8.02017x
- computed RBAC plus lookup vs 4D mean slowdown: 457.018x

Interpretation:

- The five-model harness is now executable from one benchmark binary.
- The static software baselines still over-authorize the mixed request set because they do not encode runtime state.
- The 4D path stays near the best-case RBAC hash-map floor in this local software run.
- The modeled external-state path remains far slower than the in-process 4D path.

## 2026-04-13 SQLite validation run

Command:

```bash
/tmp/hngac-fpga-build-sqlite/hngac_compare_benchmark 2000 10000
```

Modeled configuration:

- iterations: 2,000
- RBAC external-state lookup delay: 10,000 ns
- warmup: 1,000 iterations per model
- SQLite3 found by CMake and linked into `hngac_compare_benchmark`

Observed output:

- RBAC hash map mean: 23.9025 ns
- NGAC-DAG traversal mean: 167.672 ns
- 3D baseline mean: 15.9185 ns
- 4D state-aware mean: 24.4805 ns
- RBAC plus state lookup mean: 10,280.8 ns
- RBAC plus SQLite state lookup mean: 463.377 ns
- RBAC plus SQLite state lookup allowed count: 1,000 / 2,000
- computed RBAC plus SQLite vs 4D mean slowdown: 18.9284x

Interpretation:

- The SQLite-backed lookup is much cheaper than the intentionally delayed modeled lookup path, but still materially slower than the in-process 4D bitmask path.
- The SQLite-backed path preserves the same allow/deny split as the 4D and modeled RBAC+lookup paths on the mixed scenario set.

## 2026-04-13 smoke run

Command:

```bash
/tmp/hngac-fpga-hls-build-compare/hngac_compare_benchmark 20000 100000
```

Modeled configuration:

- iterations: 20,000
- RBAC external-state lookup delay: 100,000 ns

Observed output:

- 3D baseline mean: 15.526 ns
- 3D baseline p99: 18 ns
- 3D baseline max: 134 ns
- 4D state-aware mean: 15.0865 ns
- 4D state-aware p99: 17 ns
- 4D state-aware max: 722 ns
- RBAC plus state lookup mean: 100,688 ns
- RBAC plus state lookup p99: 102,665 ns
- RBAC plus state lookup max: 696,110 ns
- computed 4D vs 3D mean overhead: -2.83074%
- computed RBAC plus lookup vs 4D mean slowdown: 6674.06x

Interpretation:

- The 3D and 4D local software means are effectively tied at this run size.
- The slight negative 4D overhead is measurement noise, not evidence that 4D is intrinsically faster.
- The RBAC plus external-state-lookup path is dramatically slower once the external lookup delay is modeled.
- These numbers are local pre-AWS software results only. They are useful for shaping the paper and guiding HLS work, but they are not FPGA measurements.

## 2026-04-13 mixed allow/deny scenario

Command:

```bash
/tmp/hngac-fpga-hls-build-compare/hngac_compare_benchmark 20000 100000
```

Modeled configuration:

- iterations: 20,000
- RBAC external-state lookup delay: 100,000 ns
- scenario mix: 4 state-satisfying requests and 4 state-failing requests

Observed output:

- 3D baseline mean: 14.9253 ns
- 3D baseline p99: 17 ns
- 3D baseline max: 133 ns
- 3D baseline allowed count: 20,000 / 20,000
- 4D state-aware mean: 16.2374 ns
- 4D state-aware p99: 18 ns
- 4D state-aware max: 12,103 ns
- 4D state-aware allowed count: 10,000 / 20,000
- RBAC plus state lookup mean: 100,929 ns
- RBAC plus state lookup p99: 107,352 ns
- RBAC plus state lookup max: 1,003,759 ns
- RBAC plus state lookup allowed count: 10,000 / 20,000
- computed 4D vs 3D mean overhead: 8.79148%
- computed RBAC plus lookup vs 4D mean slowdown: 6215.85x

Interpretation:

- The mixed scenario now exposes the semantic gap clearly: the 3D baseline authorizes every request because it cannot enforce state constraints.
- The 4D and RBAC plus state-lookup paths both block the state-failing half of the request set.
- The measured 4D mean overhead over 3D in this run is still small relative to the modeled RBAC plus state-lookup cost.
- These remain local pre-AWS software results only and should not be presented as FPGA measurements.
