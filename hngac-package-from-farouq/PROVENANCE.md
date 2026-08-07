# Provenance and commit scope

**Source:** hardware evidence package delivered by Farouq (HW lead), received 2026-08-07.
Original archive: `fpga/hngac-package-from-farouq.zip` (gitignored, byte-for-byte
duplicate of this extracted tree).

**Toolchain:** Vitis HLS / Vivado 2025.2, build 6295257.
**Target part:** `xc7z020-clg400-1` (Zynq-7020, the PYNQ-Z1 part), 10 ns clock (100 MHz).
**Synthesis dates:** 4D 2026-08-05 16:14 MDT, 5D 2026-08-05 16:26 MDT.
**Software benchmark date:** 2026-08-04.

## This directory is evidence, not working source

`hngac-package-from-farouq/` is an immutable record of what the HW team measured.
**Do not edit anything here to change project behaviour.** The live kernel is
`fpga/hls/src/hngac_kernel.cpp` at the repo root.

Note that the delivered `synthesis/opt-v1-{4d,5d}/src/hngac_kernel.cpp` is an
**optimized** kernel that differs from the repo's current kernel: it checks two
rules per clock in a two-stage pipeline, giving 0.5 cycles per rule instead of 1.
Reconciling that optimization back into `fpga/hls/src/` is tracked as open work.

## What was committed, and what was left out

Committed:

| Path | Why |
|---|---|
| `results/` | csynth, cosim, per-call transaction reports, SW perf log, extraction scripts, derived CSVs |
| `synthesis/opt-v1-{4d,5d}-zynq-2rule-vitis2025.2/` | exact HLS source, testbenches and TCL that produced the reports |
| `board-test/opt-v1-5d-pynqz1-vitis2025.2/` | bitstream, IP, PYNQ verification script, board run log |
| `kernel/5d/hngac-fpga/fpga/hls/` | the exact software source measured by `results/sw/run_perf_benchmark.sh`, which differs from the repo copy |
| `kernel/4d/hngac-fpga_4d_hw_results/` | earlier 4D run, and the **only** Vivado place-and-route data we have (routed timing WNS +2.170 ns, 0 failing endpoints, plus power reports) |

Left out, via `.gitignore`:

| Path | Why |
|---|---|
| `kernel/4d/` (except `hngac-fpga_4d_hw_results/`) | full mirror of this repo at an older commit; duplicates the working tree and carries a `scripts/aws/fpga-dev.env` that this repo deliberately gitignores |
| `kernel/5d/hngac-fpga/` (except `fpga/hls/`) | full mirror of this repo; duplicates the working tree |
| `*:Zone.Identifier` (255 files) | Windows mark-of-the-web metadata, no content |
| `*.backup` | editor artifact |
| `fpga/hngac-package-from-farouq.zip` | duplicate of this extracted tree |

Secret scan before commit found no keys, credentials, tfstate or tfvars. The
`fpga-dev.env` present in the 4D mirror was inspected and contains only blank
template values, but is excluded anyway to stay consistent with repo policy.

## Reproducing

Software benchmark:

```bash
bash results/sw/run_perf_benchmark.sh 200000
python3 results/sw/extract_sw_cycles.py --csv results/sw/sw_cycles.csv
```

Hardware co-simulation:

```bash
cd synthesis/opt-v1-5d-zynq-2rule-vitis2025.2
vitis_hls -f scripts/hls_cosim_benchmark.tcl
python3 ../../results/hw/extract_cosim_per_rule.py ../../results/cosim-opt-v1-5d
```

Board test: copy `board-test/.../bitstream/*.{bit,hwh}` and
`board-test/.../pynq-scripts/*.py` into the same directory on the PYNQ-Z1, then
`sudo /usr/local/share/pynq-venv/bin/python3 pynq_functional_verify.py`.

All derived tables in `results/README.md` were re-derived from the raw reports on
2026-08-07 and reproduce exactly.
