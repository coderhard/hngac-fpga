# Hardware Synthesis Results — Brief

Date: 2026-04-19. Vitis 2025.2 on AWS F2 Developer AMI (c7i-flex.large).
Target part: xcvu47p-fsvh2892-2L-e (F2-compatible VU47P).

## Results

| Metric | Value |
|---|---|
| Clock achieved (csynth) | 297 MHz |
| Clock achieved (post-route) | 353 MHz |
| LUT (csynth / post-route) | 5,426 / 796 |
| FF (csynth / post-route) | 948 / 942 |
| BRAM / DSP / URAM | 0 / 0 / 0 |
| RTL latency min (cosim) | 3 cycles = 10 ns |
| RTL latency avg (cosim) | 36 cycles = 121 ns |
| RTL latency max (cosim) | 1030 cycles = 3.47 µs |

The cosim max matches the csynth bound exactly because the testbench now includes a 512-rule worst-case stress test. Compared to the DCAS paper's ROS 2 baseline of 1.05 µs average and 157 µs worst case, the hardware is roughly 8× faster on average and 45× tighter on worst case.

Four verification layers green: CTest (g++), csim (Vitis), csynth (RTL generation), cosim (RTL vs C++ equivalence). All 35 test cases pass in each layer. Real F2 hardware measurement is the remaining layer and needs a larger instance.

## Repo changes

`hngac_authorize` was moved out of `namespace hngac::fpga` to file scope so Vitis 2025.2 cosim can resolve its generated stub symbol. Types and helpers stay inside the namespace; a `using namespace hngac::fpga;` at the top of `hngac_kernel.cpp` keeps the function body unchanged from the original. The declaration in `hngac_kernel.hpp` uses fully qualified parameter types. The testbench `hngac_kernel_tb.cpp` and benchmark `hngac_compare_benchmark.cpp` each had their `using hngac::fpga::hngac_authorize;` line removed since the function is now at file scope.

Six docs updated. README, fpga/hls/README, setup guides, the LaTeX walkthrough, and CLAUDE.md now show `vitis-run --mode hls --tcl ...` as the primary invocation (Vitis 2025.x dropped the standalone `vitis_hls` command) with `vitis_hls -f` kept as a commented fallback for Vitis 2024.x. Default target part changed from xcu250 to the F2-installed VU47P.

## Reproduce

```bash
source /opt/Xilinx/2025.2/Vitis/settings64.sh
export HNGAC_HLS_PART=xcvu47p-fsvh2892-2L-e
export HNGAC_HLS_CLOCK_NS=5.0
export HNGAC_HLS_COSIM=1
export HNGAC_HLS_WORKDIR=/tmp/hngac-fpga-hls
vitis-run --mode hls --tcl fpga/hls/scripts/vitis_hls.tcl
```

About 3 minutes for csim + csynth + cosim on c7i-flex.large. Add `export HNGAC_HLS_EXPORT=1` to also run Vivado place-and-route and export the IP; that adds roughly 12 minutes.

## Artifacts

`syn_report/` holds the csynth reports. `cosim_report/` holds the RTL simulation results. `vivado_pr_report/` holds the post-route Vivado reports. `hngac_authorize_ip.zip` is the packaged IP output from the Vivado flow. The long-form writeup is in `SUMMARY.md`.
