# Coordination Board

Use this file to coordinate multiple coding agents working in the same repo.

## Rules

1. Claim files and task scope before editing.
2. Do not edit files claimed by another agent unless the claim is released here first.
3. Keep claims narrow and file-specific when possible.
4. Add a timestamp, agent name, and intended checkpoint for each claim.
5. Release or update the claim after commit.

## Active Claims

### 2026-04-13 07:50 CDT

- Agent: Codex (GPT-5.4)
- Scope: unified benchmark expansion and repo-state docs
- Files:
  - `fpga/hls/bench/hngac_compare_benchmark.cpp`
  - `fpga/hls/scripts/run_local_compare_sweep.sh`
  - `CLAUDE.md`
  - `docs/benchmark-gap-analysis.md`
  - `docs/local-benchmark-notes.md`
  - `docs/decision-log.md`
  - `docs/status-log.md`
  - `docs/coordination-board.md`
- Planned checkpoint:
  - commit the five-model comparison harness
  - track repo-state files in git
  - refresh benchmark and agent guidance docs

### 2026-04-13 (Claude Code, claude-sonnet-4-6)

- Agent: Claude Code (claude-sonnet-4-6)
- Scope: unit test expansion + HLS pragma pass
- Files claimed (do not edit while this claim is active):
  - `fpga/hls/tb/hngac_kernel_tb.cpp`
  - `fpga/hls/src/hngac_kernel.cpp`
- Planned checkpoint:
  - expand testbench: calibration_required isolation, object out-of-range, empty policy, superset-state semantics, first-match-wins, zero-required-states rule
  - add `#pragma HLS PIPELINE` to the rule-scan loop in hngac_kernel.cpp
  - commit and push
- Release condition: claim released after both checkpoints are committed

## Outstanding Unclaimed Work

- `fpga/hls/scripts/vitis_hls.tcl`: first synthesis/report capture workflow
- `benchmarks/`: legacy benchmark reconciliation or deprecation notes
- optional real SQLite-backed RBAC+state baseline (replaces simulated delay in compare benchmark)
- optional OPA baseline (out-of-process, separate latency scale)
- CSV output for all five models via sweep script (currently sweep covers three models)
