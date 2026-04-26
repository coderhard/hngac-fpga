# Resume Plan — 2026-04-26

Purpose: preserve the next work plan before shutting down. This is a restart checklist for experiments, research, and manuscript edits that do not depend on Badawy/HW-team validation.

## Current State

- Local build and CTest pass: `45 passed, 0 failed`.
- The paper and README are only partially aligned.
- Hardware validation remains pending: HLS synthesis, co-sim, LUT/timing reports, and any real FPGA/AWS measurement.
- `CLAUDE.md` is stale relative to current 5D/IPCCC repo state.
- `docs/coordination-board.md` has a stale active Codex claim for benchmark/doc alignment and a separate standing lock on `fpga/hls/src/hngac_kernel.cpp` while HW synthesis is pending.
- Current uncommitted work includes README and benchmark harness edits related to the flattened 5D baseline.

## Priority 1 — Manuscript Claim Cleanup

Goal: make the paper defensible before hardware numbers arrive.

- Rewrite the contribution list to match current scope: 3D/4D/5D, SQLite empirical baseline, modeled lookup sensitivity analysis, flattened 5D direct lookup, attack demo, and pending HLS validation.
- Remove or clearly mark unsupported hardware claims as pending: fabric latency, LUT percentage, AWS F1/F2 hardware validation, co-sim pass, and deployed FPGA correctness.
- Replace the scalar 4D code listing with the actual `hngac_authorize()` policy-array kernel, or label the listing as simplified pseudocode.
- Fix terminology and typos: `Policy Decision Point (PDK)` should be `PDP`.
- Clarify that local benchmark numbers are software-only measurements, not FPGA data.
- Keep the key framing: zero-cost security dimensionality is the hypothesis/claim to be confirmed by synthesis, not a measured hardware result until reports exist.

## Priority 2 — README and Repo Doc Alignment

Goal: make repo entry points match the paper and current code.

- Resolve benchmark-count inconsistencies: README says seven models, current benchmark can run eight paths when SQLite is available because it includes `Flattened 5D direct lookup`.
- Add explicit 5D semantics to README: provenance bits, `required_provenance == 0` wildcard, and any-match provenance behavior.
- Update stale 4D/SOCC-era language in `CLAUDE.md`, `fpga/hls/README.md`, and benchmark docs.
- Preserve the HW-team lock on `fpga/hls/src/hngac_kernel.cpp` unless explicitly deciding the synthesis handoff is no longer active.
- Mark the stale April 20 Codex coordination claim as released/stale if no agent is actually running.

## Priority 3 — Fresh Canonical Benchmark Evidence

Goal: produce current, reproducible numbers after the flattened 5D baseline changes settle.

Recommended commands:

```bash
cmake -S fpga/hls -B /tmp/hngac-fpga-build
cmake --build /tmp/hngac-fpga-build
ctest --test-dir /tmp/hngac-fpga-build --output-on-failure
/tmp/hngac-fpga-build/hngac_compare_benchmark 200000 100000
```

Run at least one longer pass if time allows:

```bash
/tmp/hngac-fpga-build/hngac_compare_benchmark 1000000 100000
```

Capture and report:

- mean, p95, p99, max, stddev
- allowed/total counts
- build/reload cost
- memory footprint
- flattened 5D vs H-NGAC 5D latency and memory ratios
- SQLite vs H-NGAC 5D slowdown
- modeled lookup sensitivity separately from SQLite empirical data

## Priority 4 — Scaling Experiments

Goal: answer reviewer fairness questions about whether H-NGAC only wins at tiny policy sizes.

Add or run a policy-size sweep over rule counts such as:

- 4
- 16
- 64
- 128
- 256
- 512

For each size, report:

- H-NGAC 5D latency distribution
- flattened 5D direct lookup latency distribution
- NGAC-DAG latency distribution
- RBAC + SQLite latency distribution
- memory footprint
- policy build/reload/update cost

The most important comparison is not only lookup latency. The paper needs to show the tradeoff between compact policy representation and fully materialized lookup tables.

## Priority 5 — Jitter and Load Experiments

Goal: strengthen the determinism argument without overstating software WCET.

- Run local software benchmarks under idle and CPU-loaded conditions.
- Report tail latency and max separately from mean.
- Phrase any software max as an observed outlier, not a guaranteed WCET.
- If using `stress-ng`, record exact command, CPU count, duration, and machine details.

## Priority 6 — Research and Related Work

Goal: reduce reviewer objections around novelty and baselines.

Research/fill citations for:

- FPGA policy evaluation and access-control acceleration
- real-time authorization in cyber-physical systems and robotics
- DDS/SROS2 security limitations
- WCET and safety-certification framing for IEC 61508 / ISO 26262
- hardware security primitives that are adjacent but not policy evaluation

Check every CVE anchor:

- Use "CVE-class threat model" unless the experiment directly reproduces the specific vulnerability.
- Avoid implying the demo is an exploit reproduction if it is only an authorization-pattern demonstration.

Clarify novelty boundaries:

- DCAS: prior ROS2/software H-NGAC baseline.
- ICCCN: time-scoped software extension; cite as complementary, do not duplicate.
- BigData: batch compliance sweep; do not mix into per-decision latency tables.
- IPCCC: hardware synthesis path plus 4D/5D security-dimensionality claim.

## Hardware-Pending Items

Do not claim these as completed until evidence exists:

- Vitis HLS synthesis reports for 4D and 5D.
- Co-simulation pass/fail and number of matched test cases.
- LUT, FF, BRAM, DSP, latency cycles, initiation interval, and max frequency.
- Any FPGA fabric latency.
- Any host-to-FPGA round-trip latency.
- Any AWS F1/F2 deployed hardware measurement.

If hardware reports arrive, update:

- `paper/main.tex`
- `docs/evidence-record-2026-04-18.md` or a new dated evidence record
- `docs/project-overview.md`
- `docs/status-log.md`
- `README.md`

## Suggested Next Session Start

1. Inspect worktree: `git status --short`.
2. Read `docs/coordination-board.md` and decide whether to mark the stale active claim released.
3. Finish or revert the uncommitted README/benchmark alignment work deliberately.
4. Run build, CTest, and a fresh canonical benchmark.
5. Update paper claims before adding new results tables.
