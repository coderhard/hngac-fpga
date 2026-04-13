# Local Benchmark Notes

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
