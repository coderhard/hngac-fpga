# Results

## Software per-decision cycles

Source: `sw/perf_all_models_scaling.log` (2026-08-04)
System: `sw/system-snapshot.txt`
Reproduce: `bash sw/run_perf_benchmark.sh`
Extract: `python3 sw/extract_sw_cycles.py --csv sw/sw_cycles.csv`

| model | 4 | 10 | 50 | 100 | 200 | 500 |
|---|---|---|---|---|---|---|
| H-NGAC 3D | 70 | 75 | 115 | 151 | 211 | 390 |
| H-NGAC 4D | 74 | 78 | 120 | 194 | 292 | 582 |
| H-NGAC 5D | 80 | 82 | 130 | 221 | 360 | 685 |
| RBAC hash map | 97 | 87 | 93 | 91 | 89 | 100 |
| NGAC-DAG traversal | 608 | 638 | 636 | 629 | 636 | 1032 |
| Flattened 5D direct lookup | 990 | 1016 | 970 | 1003 | 1003 | 984 |

## Hardware csynth (opt-v1)

Source: `cosim-opt-v1-{4d,5d}/syn_report/csynth.rpt`
Extract: `python3 hw/extract_csynth.py cosim-opt-v1-5d/syn_report/csynth.rpt`

| metric | 4D | 5D |
|---|---|---|
| II | 1 | 1 |
| iteration latency | 3 | 3 |
| latency (max) | 270 | 270 |
| interval (max) | 271 | 271 |
| FF | 2579 (2%) | 2679 (2%) |
| LUT | 4580 (8%) | 5104 (9%) |
| BRAM | 0 | 0 |
| DSP | 0 | 0 |

## Hardware cosim kernel cycles (opt-v1 4D, II=1)

Source: `cosim-opt-v1-4d/cosim_report/verilog/result.transaction.rpt`
Extract: `python3 hw/extract_cosim_per_rule.py cosim-opt-v1-4d`

| rules | requests | min | avg | max |
|---|---|---|---|---|
| 4 | 11 | 14 | 14 | 14 |
| 10 | 27 | 17 | 17 | 17 |
| 50 | 134 | 37 | 37 | 37 |
| 100 | 267 | 62 | 62 | 62 |
| 200 | 534 | 112 | 112 | 112 |
| 500 | 1334 | 262 | 262 | 262 |

## Hardware cosim kernel cycles (opt-v1 5D, II=1)

Source: `cosim-opt-v1-5d/cosim_report/verilog/result.transaction.rpt`
Extract: `python3 hw/extract_cosim_per_rule.py cosim-opt-v1-5d`

| rules | requests | min | avg | max |
|---|---|---|---|---|
| 4 | 11 | 14 | 14 | 14 |
| 10 | 27 | 17 | 17 | 17 |
| 50 | 134 | 37 | 37 | 37 |
| 100 | 267 | 62 | 62 | 62 |
| 200 | 534 | 112 | 112 | 112 |
| 500 | 1334 | 262 | 262 | 262 |

## SW 4D vs HW 4D

| rules | SW cycles | HW cycles | cycle ratio |
|---|---|---|---|
| 4 | 74 | 14 | 5.3x |
| 10 | 78 | 17 | 4.6x |
| 50 | 120 | 37 | 3.2x |
| 100 | 194 | 62 | 3.1x |
| 200 | 292 | 112 | 2.6x |
| 500 | 582 | 262 | 2.2x |

## SW 5D vs HW 5D

| rules | SW cycles | HW cycles | cycle ratio |
|---|---|---|---|
| 4 | 80 | 14 | 5.7x |
| 10 | 82 | 17 | 4.8x |
| 50 | 130 | 37 | 3.5x |
| 100 | 221 | 62 | 3.6x |
| 200 | 360 | 112 | 3.2x |
| 500 | 685 | 262 | 2.6x |

## Board verification

Source: `../board-test/opt-v1-5d-pynqz1-vitis2025.2/board-verification-run.txt`
Cosim match: `cosim-opt-v1-5d/cosim_report/verilog/tmp.log`

| rules | reqs | allow | deny | result |
|---|---|---|---|---|
| 4 | 11 | 4 | 7 | PASS |
| 10 | 27 | 10 | 17 | PASS |
| 50 | 134 | 50 | 84 | PASS |
| 100 | 267 | 100 | 167 | PASS |
| 200 | 534 | 200 | 334 | PASS |
| 500 | 1334 | 500 | 834 | PASS |
