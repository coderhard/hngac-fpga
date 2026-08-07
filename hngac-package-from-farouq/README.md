# HNGAC Evidence Package

Evidence bundle for the Hardware-accelerated NGAC (H-NGAC) paper. Contains
source code, synthesis and co-simulation reports, software benchmark data,
board verification, and the scripts to reproduce every derived number.

## Structure

```
hngac-package/
├── kernel/{4d,5d}/                     full project mirrors
├── synthesis/
│   ├── opt-v1-4d-zynq-2rule-vitis2025.2/{src,include,tb,scripts}
│   └── opt-v1-5d-zynq-2rule-vitis2025.2/{src,include,tb,scripts}
├── board-test/opt-v1-5d-pynqz1-vitis2025.2/
│   ├── pynq-scripts/pynq_functional_verify.py
│   ├── vivado-scripts/vivado_build_bitstream.tcl
│   ├── ip/hngac_authorize_opt_v1_5d/
│   ├── bitstream/hngac_test.{bit,hwh}
│   └── board-verification-run.txt
└── results/
    ├── README.md                       all tables with provenance
    ├── software/
    │   ├── perf_all_models_scaling.log
    │   ├── system-snapshot.txt
    │   ├── extract_sw_cycles.py
    │   └── sw_cycles.csv
    ├── hw-opt-v1-4d/
    │   ├── cosim_report/{hngac_authorize_cosim.rpt, verilog/{result.transaction.rpt,tmp.log,lat.rpt}}
    │   └── syn_report/{csynth.rpt, ...}
    ├── hw-opt-v1-5d/                   same shape
    └── hw-extract/
        ├── extract_cosim_per_rule.py
        ├── hw_cycles_per_rule_4d.csv
        └── hw_cycles_per_rule_5d.csv
```

## Key artifacts

| artifact | what it proves | source file |
|---|---|---|
| result.transaction.rpt | per-call kernel cycles, cosim-measured | results/hw-opt-v1-{4d,5d}/cosim_report/verilog/ |
| perf_all_models_scaling.log | SW per-decision cycles via perf stat | results/software/ |
| board-verification-run.txt | 2307 requests PASS on PYNQ-Z1 silicon | board-test/.../board-verification-run.txt |
| csynth.rpt | II=1, resource utilization | results/hw-opt-v1-{4d,5d}/syn_report/ |
| hngac_authorize_cosim.rpt | cosim aggregate latency + pass/fail | results/hw-opt-v1-{4d,5d}/cosim_report/ |

## Reproducing

SW benchmark: build `kernel/5d/.../hngac_compare_benchmark.cpp`, run with
`perf stat`. See `results/software/system-snapshot.txt` for the environment.

HW cosim: `cd synthesis/opt-v1-5d-zynq-2rule-vitis2025.2 && vitis_hls -f scripts/hls_cosim_benchmark.tcl`

Board test: copy `bitstream/*.{bit,hwh}` and `pynq-scripts/*.py` to the
PYNQ-Z1 in the same directory, then
`sudo /usr/local/share/pynq-venv/bin/python3 pynq_functional_verify.py`

