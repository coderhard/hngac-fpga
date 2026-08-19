# Results

## Software per-decision mean cycles

Source: `sw/perf_all_models_scaling.log`
System: `sw/system-snapshot.txt`
Reproduce: `bash sw/run_perf_benchmark.sh`
Extract: `python3 sw/extract_sw_cycles.py --csv sw/sw_cycles.csv`
CSV has: mean, p95, p99, max per model per rule count.

| model | 4 | 10 | 50 | 100 | 200 | 500 |
|---|---|---|---|---|---|---|
| H-NGAC 3D | 70 | 75 | 115 | 151 | 211 | 390 |
| H-NGAC 4D | 74 | 78 | 120 | 194 | 292 | 582 |
| H-NGAC 5D | 80 | 82 | 130 | 221 | 360 | 685 |
| RBAC hash map | 97 | 87 | 93 | 91 | 89 | 100 |
| NGAC-DAG traversal | 608 | 638 | 636 | 629 | 636 | 1032 |
| Flattened 5D direct lookup | 990 | 1016 | 970 | 1003 | 1003 | 984 |

## Hardware csynth (opt-v1)

Source: `cosim-opt-v1-{3d,4d,5d}/syn_report/csynth.rpt`
Extract: `python3 hw/extract_csynth.py cosim-opt-v1-5d/syn_report/csynth.rpt`

| metric | 3D | 4D | 5D |
|---|---|---|---|
| II | 1 | 1 | 1 |
| iteration latency | 2 | 3 | 3 |
| latency (max) | 270 | 270 | 270 |
| interval (max) | 271 | 271 | 271 |
| FF | 2348 (2%) | 2579 (2%) | 2679 (2%) |
| LUT | 3768 (7%) | 4580 (8%) | 5104 (9%) |
| BRAM | 0 | 0 | 0 |
| DSP | 0 | 0 | 0 |

## Hardware cosim kernel cycles (opt-v1 3D, II=1)

Source: `cosim-opt-v1-3d/cosim_report/verilog/result.transaction.rpt`
Extract: `python3 hw/extract_cosim_per_rule.py cosim-opt-v1-3d`

| rules | requests | min | avg | max |
|---|---|---|---|---|
| 4 | 11 | 14 | 14 | 14 |
| 10 | 27 | 17 | 17 | 17 |
| 50 | 134 | 37 | 37 | 37 |
| 100 | 267 | 62 | 62 | 62 |
| 200 | 534 | 112 | 112 | 112 |
| 500 | 1334 | 262 | 262 | 262 |

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

## SW 3D vs HW 3D

| rules | SW mean cycles | HW cycles | cycle ratio |
|---|---|---|---|
| 4 | 70 | 14 | 5.0x |
| 10 | 75 | 17 | 4.4x |
| 50 | 115 | 37 | 3.1x |
| 100 | 151 | 62 | 2.4x |
| 200 | 211 | 112 | 1.9x |
| 500 | 390 | 262 | 1.5x |

## SW 4D vs HW 4D

| rules | SW mean cycles | HW cycles | cycle ratio |
|---|---|---|---|
| 4 | 74 | 14 | 5.3x |
| 10 | 78 | 17 | 4.6x |
| 50 | 120 | 37 | 3.2x |
| 100 | 194 | 62 | 3.1x |
| 200 | 292 | 112 | 2.6x |
| 500 | 582 | 262 | 2.2x |

## SW 5D vs HW 5D

| rules | SW mean cycles | HW cycles | cycle ratio |
|---|---|---|---|
| 4 | 80 | 14 | 5.7x |
| 10 | 82 | 17 | 4.8x |
| 50 | 130 | 37 | 3.5x |
| 100 | 221 | 62 | 3.6x |
| 200 | 360 | 112 | 3.2x |
| 500 | 685 | 262 | 2.6x |

## Board verification (bare-metal C, PYNQ-Z1 @ 100 MHz)

Source: `../board-test/opt-v1-5d-pynqz1-vitis2025.2/board-scripts/bare_metal_latency.log`

| rules | reqs | allow | deny | result |
|---|---|---|---|---|
| 4 | 11 | 4 | 7 | PASS |
| 10 | 27 | 10 | 17 | PASS |
| 50 | 134 | 50 | 84 | PASS |
| 100 | 267 | 100 | 167 | PASS |
| 200 | 534 | 200 | 334 | PASS |
| 500 | 1334 | 500 | 834 | PASS |

## Board round-trip latency (CLOCK_MONOTONIC, full corpus)

Source: `../board-test/opt-v1-5d-pynqz1-vitis2025.2/board-scripts/bare_metal_latency.log`

| rules | reqs | min (ns) | avg (ns) | max (ns) |
|---|---|---|---|---|
| 4 | 11 | 2055 | 2090 | 2148 |
| 10 | 27 | 2043 | 2087 | 2751 |
| 50 | 134 | 2246 | 2436 | 81686 |
| 100 | 267 | 2566 | 2608 | 61729 |
| 200 | 534 | 3058 | 3117 | 31858 |
| 500 | 1334 | 4511 | 4765 | 5920702 |

## Board AXI Timer capture (full corpus, hardware-latched)

Source: `../board-test/opt-v1-5d-pynqz1-vitis2025.2/board-scripts/bare_metal_latency.log`

| rules | reqs | min (cyc) | avg (cyc) | max (cyc) |
|---|---|---|---|---|
| 4 | 11 | 39 | 39 | 39 |
| 10 | 27 | 42 | 42 | 42 |
| 50 | 134 | 62 | 62 | 62 |
| 100 | 267 | 87 | 87 | 87 |
| 200 | 534 | 137 | 137 | 137 |
| 500 | 1334 | 287 | 287 | 287 |
