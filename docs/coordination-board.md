# Coordination Board

Use this file to coordinate multiple coding agents working in the same repo.

## Rules

1. Claim files and task scope before editing.
2. Do not edit files claimed by another agent unless the claim is released here first.
3. Keep claims narrow and file-specific when possible.
4. Add a timestamp, agent name, and intended checkpoint for each claim.
5. Release or update the claim after commit.

## Active Claims

No active agent claims.

## ⚠️ STANDING WARNING — `paper/main.tex` is NOT authoritative right now

**As of 2026-08-19 the author is editing the manuscript in Overleaf, not in this
repo.** The Overleaf copy is the live one. The author will save a final copy back
here later.

Until that copy lands:

- **Do not edit `paper/main.tex`.** Any edit here will be silently overwritten by
  the Overleaf save, or worse, will tempt someone to merge two diverged versions
  three days before a deadline.
- **Do not treat the committed `paper/main.pdf` as current.** It reflects repo
  state at commit `b8d22c9`, not the author's Overleaf edits.
- The author is trimming 9 pages back to 8 and stripping the red review markup
  (`\new{}` / `\newcolor`). Expect both to be gone in the returning copy.
- If asked to work on the paper before the copy returns, **ask for the current
  Overleaf export first** rather than editing the stale tree.

Remove this warning when the author's final copy is committed.

## Released Claims (most recent first)

### 2026-08-19 03:10 CDT — COMPLETED / RELEASED (Claude Code, claude-opus-5)

- Agent: Claude Code (claude-opus-5)
- Scope: ingest evidence package v2 (3D synthesis + board timing); promote the
  hardware claim from 4D-vs-5D to the full three-way 3D/4D/5D parity
- Files:
  - `hngac-package-v2-from-farouq/**` (new, evidence — treat as immutable)
  - `.gitignore`
  - `paper/main.tex`, `paper/figures/fig-system-bd.pdf` (new)
  - `analysis/make_paper_figures.py`
  - `CLAUDE.md`, `docs/canonical-context.md`, `docs/status-log.md`,
    `docs/decision-log.md`, `docs/coordination-board.md`
- Not touched: `hngac-package-from-farouq/` (immutable, v1), `fpga/hls/` kernel source
- Note for co-authors: v2 is a strict superset of v1; only its 32-file delta is
  tracked. See `hngac-package-v2-from-farouq/PROVENANCE.md`.
- Outcome: three-way parity claim landed; paper rebuilt with zero overfull boxes
  and zero undefined references.
- Follow-up in the same session, at author request: the Vivado block design is now
  wired in as `Fig.~1` (a `figure*` in Section IV, cropped to its CropBox), and all
  2026-08-19 edits are marked in red for author review.
- **Paper is at 9 pages.** IPCCC gives 8 free, then $100/page. The author is
  trimming verbiage manually to get back to 8.
- **Review markup convention:** edits made when evidence package v2 landed are
  wrapped in `\new{...}` (inline) or preceded by `\newcolor` (whole new tables),
  defined in the `main.tex` preamble. **To turn marking off before submission,**
  redefine `\new` to `{#1}` and `\newcolor` to empty. Both must stay *defined* —
  they are used inside table bodies. Instructions repeat in the preamble comment.
- CLAIM RELEASED

## Released Claims (most recent first)

### 2026-08-07 — COMPLETED / RELEASED (Claude Code, Opus 5)

- Agent: Claude Code (Opus 5)
- Scope: ingest, verify and commit the HW evidence package; move the hardware
  claim from placeholder to measured
- Files:
  - `hngac-package-from-farouq/**` (new, evidence — treat as immutable)
  - `.gitignore`
  - `docs/canonical-context.md`
  - `docs/decision-log.md`
  - `docs/status-log.md`
  - `docs/project-overview.md`
  - `docs/coordination-board.md`
  - `CLAUDE.md`
  - `analysis/make_hw_sw_charts.py` (new)
  - `docs/hngac-fpga-status-review-2026-08-07.md` (new)
- Note for co-authors: `hngac-package-from-farouq/` is evidence, not working
  source. The live kernel remains `fpga/hls/src/hngac_kernel.cpp`. See
  `hngac-package-from-farouq/PROVENANCE.md`.
- CLAIM RELEASED

## Released Claims

### 2026-04-20 17:22 CDT — STALE/RELEASED 2026-04-26 15:39 CDT

- Agent: Codex (GPT-5.4)
- Scope: reviewer-fair in-memory 5D hash baseline + benchmark/doc alignment
- Files:
  - `fpga/hls/bench/hngac_compare_benchmark.cpp`
  - `fpga/hls/scripts/run_local_compare_sweep.sh`
  - `docs/benchmark-gap-analysis.md`
  - `docs/local-benchmark-notes.md`
  - `docs/decision-log.md`
  - `docs/status-log.md`
  - `docs/coordination-board.md`
- Checkpoint:
  - add a flattened in-memory 5D decision baseline alongside the current hash, DAG, SQLite, and H-NGAC paths
  - widen reported metrics to cover tail latency, jitter, memory/build/update cost, and scaling notes needed for reviewer-facing fairness
  - align stale 4D/SOCC-era benchmark docs to the current 5D/IPCCC scope
- Release note:
  - no other agent is currently running on this repo/project
  - current Codex session is taking over cleanup and non-HW doc alignment
  - HW-dependent evidence placeholders remain pending the Badawy/HW-team synthesis handoff

### 2026-04-20 09:55 CDT — COMPLETED

- Agent: Codex (GPT-5.4)
- Scope: bibliography verification and IPCCC reference expansion
- Files:
  - `paper/refs.bib`
  - `docs/coordination-board.md`
- Checkpoint:
  - updated local `paper/refs.bib` with verified ROS2/DDS, FPGA security,
    WCET/safety, ISO 26262, and NVD CVE entries
  - normalized existing `mell2017ngac`, `sros2`, and `robotcore2022`
    metadata; flagged low-confidence self-cite publication details
  - `paper/` remains gitignored in this repo, so bibliography edits are
    local-only
  - CLAIM RELEASED

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

### 2026-04-13 (Claude Code, claude-sonnet-4-6) — COMPLETED

- Agent: Claude Code (claude-sonnet-4-6)
- Scope: README.md update + benchmarks/ legacy note
- Files: `README.md`, `benchmarks/README.md`
- Checkpoint: README.md updated (IPCCC, 7 models, 45 tests); benchmarks/README.md
  updated (SOCC→IPCCC, 5→7 models) — CLAIM RELEASED

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

### 2026-04-13 16:02 CDT — COMPLETED

- Agent: Codex (GPT-5.4)
- Scope: local AWS launch env preset
- Files:
  - `scripts/aws/fpga-dev.env`
  - `.gitignore`
  - `docs/status-log.md`
  - `docs/coordination-board.md`
- Checkpoint:
  - added a machine-local `scripts/aws/fpga-dev.env` with sane defaults for this workstation
  - updated `.gitignore` so `scripts/aws/fpga-dev.env` stays local and customization-safe

### 2026-04-14 09:10 CDT — COMPLETED

- Agent: Codex (GPT-5.4)
- Scope: PDF walkthrough for AWS setup without SSO
- Files:
  - `docs/aws-setup-screen-by-screen.tex`
  - `docs/aws-setup-screen-by-screen.pdf`
  - `docs/status-log.md`
  - `docs/coordination-board.md`
- Checkpoint:
  - added `docs/aws-setup-screen-by-screen.tex` as a dedicated solo-account AWS walkthrough
  - generated `docs/aws-setup-screen-by-screen.pdf` for offline use outside chat

## Outstanding Unclaimed Work

- `fpga/hls/scripts/vitis_hls.tcl`: DONE — commit `6d6b1bd`
- `benchmarks/`: legacy reconciliation — completed previously; preserved as DCAS baseline context
- `README.md`: 5D/IPCCC alignment in progress in current Codex cleanup
- HLS synthesis/co-simulation reports: pending Badawy/HW-team handoff; keep hardware result placeholders until reports arrive
- optional OPA baseline (out-of-process, separate latency scale)

### 2026-04-20 17:00 CDT — COMPLETED (Claude Code, claude-sonnet-4-6)

- Agent: Claude Code (claude-sonnet-4-6)
- Scope: coordination policy update — venue pivot to IPCCC, 5D in scope, HW team re-oriented
- Files:
  - `docs/coordination-board.md`
  - `docs/project-overview.md` (new)
  - `docs/hw-team-update-2026-04-20.md` (new)
  - `docs/decision-log.md`
  - `README.md`
  - `paper/main.tex` (gitignored — local only)
  - `paper/refs.bib` (gitignored — local only)
- Checkpoint: commit `ff680da` — CLAIM RELEASED
- Standing rules added (permanent, not a work claim):
  - Human owners: Hassan (paper), Badawy lead (HLS/HW), Deepti (review), Sai (AWS)
  - `fpga/hls/src/hngac_kernel.cpp` locked to agents while HW team runs synthesis
  - `paper/main.tex` and `paper/refs.bib` — one agent at a time
  - All agents must use `### YYYY-MM-DD HH:MM CDT — STATUS` timestamp format on this board

### 2026-08-07 11:05 CDT — COMPLETED (Claude Code, claude-opus-5)

- Agent: Claude Code (claude-opus-5)
- Scope: (1) attack-class taxonomy fix across docs; (2) H-NGAC / HyperNGAC terminology split
- Files:
  - `CLAUDE.md`, `AGENTS.md`
  - `docs/canonical-context.md`
  - `docs/project-overview.md`
  - `docs/evidence-record-2026-04-18.md`
  - `docs/decision-log.md`
  - `docs/ipccc-2026-abstract-draft.md`
  - `README.md`
  - `ros2_ws/src/ngac_auth/src/{gatekeeper_5d_node,compromised_ros2_node}.cpp` (comments/log strings only)
  - `scripts/ros2_demo/*.sh` (comments/echo strings only)
- Not touched: `fpga/hls/` kernel or bench source; `hngac-package-from-farouq/` (immutable);
  `data/attack2_*.log` filenames (committed evidence identifiers)
- Checkpoint: taxonomy + terminology landed; CLAIM RELEASED

### 2026-08-07 15:55 CDT — COMPLETED (Claude Code)

- Agent: Claude Code
- Scope: manuscript scaffold + full draft v1
- Files: `docs/manuscript-scaffold-ipccc-2026.md` (new), `paper/main.tex` (new),
  `paper/refs.bib` (new), `paper/figures/`, `docs/ipccc-2026-manuscript-draft-v1.docx`,
  `.gitignore`, `docs/status-log.md`
- Note: `paper/` is now TRACKED (April skeleton was lost to the old gitignore rule).
  Standing rule "paper/main.tex one agent at a time" still applies.
- Checkpoint: this commit — CLAIM RELEASED

### 2026-08-07 17:10 CDT — COMPLETED (Claude Code)

- Agent: Claude Code (orchestrating 4 background sweep agents)
- Scope: related-work sweep (task 2 of manuscript-agent-prompt) + integration
- Files: `docs/related-work-sweep-2026-08-07.md` (new), `paper/refs.bib`,
  `paper/main.tex` (Section VII rewrite + Deng cite in Section II),
  `docs/manuscript-scaffold-ipccc-2026.md`, `docs/ipccc-2026-manuscript-draft-v1.docx`
- Checkpoint: this commit — CLAIM RELEASED
