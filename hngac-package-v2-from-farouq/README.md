# HNGAC Evidence Package

```
hngac-package/
├── kernel/
│   ├── 4d/
│   └── 5d/
│
├── synthesis/
│   ├── opt-v1-3d-zynq-2rule-vitis2025.2/{src,include,tb,scripts}
│   ├── opt-v1-4d-zynq-2rule-vitis2025.2/{src,include,tb,scripts}
│   └── opt-v1-5d-zynq-2rule-vitis2025.2/{src,include,tb,scripts}
│
├── board-test/opt-v1-5d-pynqz1-vitis2025.2/
│   ├── board-scripts/
│   │   ├── bare_metal_latency.c        functional verification + latency
│   │   ├── bare_metal_latency.log      output from board run
│   │   └── pynq_functional_verify.py   Python verification (2307 requests)
│   ├── vivado-scripts/vivado_build_bitstream.tcl
│   ├── ip/hngac_authorize_opt_v1_5d/
│   └── bitstream/hngac_test.{bit,hwh}
│
└── results/
    ├── README.md                       all tables
    ├── sw/
    │   ├── perf_all_models_scaling.log
    │   ├── run_perf_benchmark.sh
    │   ├── extract_sw_cycles.py
    │   ├── sw_cycles.csv               mean, p95, p99, max per model
    │   └── system-snapshot.txt
    ├── cosim-opt-v1-3d/
    │   ├── cosim_report/{hngac_authorize_cosim.rpt, verilog/{result.transaction.rpt, tmp.log, lat.rpt}}
    │   └── syn_report/{csynth.rpt, ...}
    ├── cosim-opt-v1-4d/                same shape
    ├── cosim-opt-v1-5d/                same shape
    └── hw/
        ├── extract_cosim_per_rule.py
        ├── extract_csynth.py
        ├── hw_csynth.csv               3D + 4D + 5D resources
        ├── hw_cycles_per_rule_3d.csv
        ├── hw_cycles_per_rule_4d.csv
        └── hw_cycles_per_rule_5d.csv
```

## Reproducing

SW benchmark:
```bash
bash results/sw/run_perf_benchmark.sh
```

HW cosim:
```bash
cd synthesis/opt-v1-5d-zynq-2rule-vitis2025.2
vitis_hls -f scripts/hls_cosim_benchmark.tcl
```

Board test (PYNQ-Z1 @ 192.168.2.99, creds xilinx/xilinx):
```bash
scp bitstream/* board-scripts/* xilinx@192.168.2.99:/tmp/hngac/
ssh xilinx@192.168.2.99
cd /tmp/hngac && gcc -O2 -o bare_metal_latency bare_metal_latency.c
sudo ./bare_metal_latency
```

Board scripts resolve the bitstream relative to their own location. The `.bit`, `.hwh`, and scripts must be in the same directory.

## Key files

| file | what it contains |
|---|---|
| `results/sw/perf_all_models_scaling.log` | SW per-decision cycles (perf stat, 6 models x 6 rule counts) |
| `results/cosim-opt-v1-*/cosim_report/verilog/result.transaction.rpt` | per-call kernel cycles from cosim (2307 transactions) |
| `results/cosim-opt-v1-*/cosim_report/verilog/tmp.log` | testbench stdout (rule-count sections, allow/deny counts) |
| `results/cosim-opt-v1-*/syn_report/csynth.rpt` | II, latency, FF, LUT, BRAM, DSP |
| `board-test/.../board-scripts/bare_metal_latency.log` | board verification + round-trip ns + AXI Timer capture cycles |
