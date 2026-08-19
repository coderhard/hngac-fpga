# Provenance and commit scope — evidence package v2

**Source:** second hardware evidence package delivered by Md Omar Faruque (NMSU),
announced 2026-08-13, received into this working tree 2026-08-19.
Original archive: `fpga/hngac-package_v2.zip` (gitignored).

**Toolchain:** Vitis HLS / Vivado 2025.2.
**Target part:** `xc7z020-clg400-1` (Zynq-7020, the PYNQ-Z1 part), 10 ns clock (100 MHz).
**3D synthesis and co-simulation:** 2026-08-13.
**Board run (bare-metal C, latency + AXI Timer):** 2026-08-13.

## This directory holds the delta only

The v2 archive is a **strict superset** of the v1 archive: 27 files added, 5 files
changed, none removed. Committing it whole would have duplicated ~7 MB already
tracked under `hngac-package-from-farouq/`, including a 4 MB bitstream that is
byte-for-byte identical.

So this directory contains **only what v2 added or changed**. For any file not
present here, the v1 copy in `hngac-package-from-farouq/` is current and
authoritative. Both directories are evidence, not working source. The live kernel
remains `fpga/hls/src/hngac_kernel.cpp` at the repo root.

Changed relative to v1:

| Path | What changed |
|---|---|
| `README.md` | package index now lists the 3D tree and `board-scripts/` |
| `results/README.md` | all derived tables re-cut with 3D columns, board round-trip and AXI Timer sections added |
| `results/hw/hw_csynth.csv` | 3D row added alongside 4D and 5D |
| `results/sw/extract_sw_cycles.py` | now emits p95, p99 and max, not mean alone |
| `results/sw/sw_cycles.csv` | re-extracted with the wider statistics and the 3D rows |

## What this package answers

It was requested in two parts. Recording both outcomes plainly:

**Ask 1 — a 3D synthesis run: DELIVERED.** `synthesis/opt-v1-3d-zynq-2rule-vitis2025.2/src/hngac_kernel.cpp`
is the opt-v1 4D kernel with the state term removed and nothing else changed:

```diff
-        return subj_ok && obj_ok && attr_ok;
+        bool state_ok = (rule.required_states & request.object_state) == rule.required_states;
+        return subj_ok && obj_ok && attr_ok && state_ok;
```

This restores the three-way parity claim. 3D, 4D and 5D resolve in identical cycle
counts at every policy size (14 / 17 / 37 / 62 / 112 / 262), at II=1, with an
identical 6.965 ns estimated clock against a 10.00 ns target and 2.70 ns
uncertainty. All three co-simulations Pass.

One row is **not** identical and must not be reported as if it were: iteration
latency is 2 for 3D and 3 for 4D and 5D. Pipeline depth grows by one stage when
the state dimension arrives, then stops. Because II=1 that depth is amortized and
per-decision cycle count does not move.

Area is not flat across the full span either. LUT rises 3,768 → 4,580 → 5,104,
so 3D → 5D costs +1,336 LUT (+35.5%) and +331 FF (+14.1%). The +11.4% LUT figure
quoted elsewhere is 5D-versus-4D only.

**Ask 2 — a software baseline on the board's own Cortex-A9: NOT DELIVERED.**
`results/sw/perf_all_models_scaling.log` is md5-identical to the v1 copy
(`4f0f18ed8ea2cabc09bff9c450c665cb`) and `results/sw/system-snapshot.txt` still
reports an i7-12800H. The v2 software table looks new only because it was
re-extracted with wider statistics and a 3D row. The 4.96 GHz-versus-100 MHz
asymmetry in the software comparison stands unaddressed.

## Two results that were not requested and matter

**Over-authorization is now measured in RTL, not only in software.** The 3D and 4D
co-simulations ran the same corpus as 5D on the same silicon-bound hardware
(`results/cosim-opt-v1-*/cosim_report/verilog/tmp.log`). At 500 rules, 3D admits
all 1,334 requests, 4D admits 917, and 5D admits 500 — at identical cycle cost.

**On-board hardware-latched cycle counts now exist.** The AXI Timer capture in
`board-test/.../board-scripts/bare_metal_latency.log` reads 39 / 42 / 62 / 87 /
137 / 287 cycles with min = avg = max at every policy size, exactly 25 cycles
above the co-simulated kernel counts at all six points. The 25 cycles are fixed
AXI-Lite handshake overhead. This supersedes the standing rule that the board run
is functional-only and carries no timing.

Read the round-trip table in the same log with care. `CLOCK_MONOTONIC` round-trip
maxima reach 5,920,702 ns at 500 rules. That tail belongs to the Linux userspace
stack on the PS side, not to the fabric, which the AXI Timer shows to be
invariant. Do not publish the round-trip tail without that framing.

## Left out, via `.gitignore`

| Path | Why |
|---|---|
| every v2 file identical to its v1 counterpart | already tracked under `hngac-package-from-farouq/` |
| `fpga/hngac-package_v2.zip` | as-delivered archive; the delta is extracted here |
| `*:Zone.Identifier` | Windows mark-of-the-web metadata, no content |

Secret scan before commit found no keys, credentials, tfstate or tfvars. Omar's
`README.md` documents the lab board as `192.168.2.99` with the stock PYNQ
`xilinx/xilinx` login. That is a vendor default on a private lab subnet, not a
secret, and it is retained because it is part of the as-delivered record.

## Reproducing

3D co-simulation:

```bash
cd synthesis/opt-v1-3d-zynq-2rule-vitis2025.2
vitis_hls -f scripts/hls_cosim_benchmark.tcl
python3 ../../hngac-package-from-farouq/results/hw/extract_cosim_per_rule.py ../../results/cosim-opt-v1-3d
```

Board latency and AXI Timer capture, from the board-test directory of the v1 tree
(the bitstream and IP are unchanged in v2):

```bash
scp hngac-package-from-farouq/board-test/opt-v1-5d-pynqz1-vitis2025.2/bitstream/* \
    hngac-package-v2-from-farouq/board-test/opt-v1-5d-pynqz1-vitis2025.2/board-scripts/* \
    xilinx@<board>:/tmp/hngac/
ssh xilinx@<board>
cd /tmp/hngac && gcc -O2 -o bare_metal_latency bare_metal_latency.c && sudo ./bare_metal_latency
```

All derived tables in `results/README.md` were re-derived from the raw reports on
2026-08-19 and reproduce exactly.
