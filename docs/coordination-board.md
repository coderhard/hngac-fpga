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
- Checkpoint:
  - completed in commit `5dbd64d` (`Add five-model benchmark harness and coordination board`)

### 2026-04-13 07:59 CDT

- Agent: Codex (GPT-5.4)
- Scope: optional real SQLite-backed RBAC+state baseline
- Files:
  - `fpga/hls/bench/hngac_compare_benchmark.cpp`
  - `fpga/hls/CMakeLists.txt`
  - `docs/benchmark-gap-analysis.md`
  - `docs/local-benchmark-notes.md`
  - `docs/decision-log.md`
  - `docs/status-log.md`
  - `docs/coordination-board.md`
- Checkpoint:
  - optional SQLite-backed lookup path added alongside the modeled lookup path
  - build remains usable when SQLite is absent and gains the extra baseline when SQLite is found
  - local benchmark and single-delay sweep validated on this machine

### 2026-04-13 (Claude Code, claude-sonnet-4-6) — COMPLETED

- Agent: Claude Code (claude-sonnet-4-6)
- Scope: unit test expansion + HLS pragma pass
- Files: `fpga/hls/tb/hngac_kernel_tb.cpp`, `fpga/hls/src/hngac_kernel.cpp`
- Checkpoint: commit `38731dd` — 11 → 34 tests, HLS INTERFACE + PIPELINE pragmas added
- Note for Codex: add `-Wno-unknown-pragmas` to `hngac_kernel` target in `fpga/hls/CMakeLists.txt` to silence expected pragma warnings from g++
- CLAIM RELEASED

### 2026-04-13 (Claude Code, claude-sonnet-4-6) — IN PROGRESS

- Agent: Claude Code (claude-sonnet-4-6)
- Scope: README.md update + benchmarks/ legacy note
- Files: `README.md`, `benchmarks/README.md` (new file)

### 2026-04-13 08:27 CDT — COMPLETED

- Agent: Codex (GPT-5.4)
- Scope: standalone WSL-local Vitis setup instructions
- Files:
  - `docs/setup-vitis-wsl.md`
  - `docs/status-log.md`
  - `docs/coordination-board.md`
- Checkpoint:
  - added `docs/setup-vitis-wsl.md` with WSL-local install paths, AMD install steps, shell setup, and the first synthesis command
  - recorded the local prerequisite and memory assumptions for the first HLS run

### 2026-04-13 08:45 CDT — COMPLETED

- Agent: Codex (GPT-5.4)
- Scope: AWS FPGA Developer AMI setup + AWS CLI launch automation
- Files:
  - `docs/setup-aws-fpga-ami.md`
  - `scripts/aws/launch_fpga_dev_instance.sh`
  - `scripts/aws/fpga-dev.env.example`
  - `docs/status-log.md`
  - `docs/coordination-board.md`
- Planned checkpoint:
  - added `docs/setup-aws-fpga-ami.md` with current `us-east-1` AMI defaults, instance guidance, IAM/CLI login notes, and F1 retirement context
  - added `scripts/aws/launch_fpga_dev_instance.sh` to provision security group, key handling, and EC2 launch via AWS CLI
  - added `scripts/aws/fpga-dev.env.example` with `f2-dev`, `f2-hardware`, and disabled-by-default `f1-retired` profiles

## Outstanding Unclaimed Work

- `fpga/hls/scripts/vitis_hls.tcl`: DONE — commit `6d6b1bd`
- `benchmarks/`: legacy reconciliation — claimed above (Claude Code)
- `README.md`: update for 4D scope and five-model harness — claimed above (Claude Code)
- optional OPA baseline (out-of-process, separate latency scale)
