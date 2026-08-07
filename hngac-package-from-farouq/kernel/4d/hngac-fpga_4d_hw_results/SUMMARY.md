# First Hardware Synthesis Results

**Date:** 2026-04-19
**Instance:** AWS c7i-flex.large (2 vCPU, 4 GB RAM), F2 Developer AMI `ami-092fc5deb8f3c0f7d`
**Tool:** Vitis 2025.2 (Build 6295257, Nov 13 2025)
**Target part:** `xcvu47p-fsvh2892-2L-e` (Virtex UltraScale+ HBM, F2-compatible)
**Target clock:** 5.00 ns (200 MHz)

## Headline numbers

| Metric | Value |
|---|---|
| Clock target | 5.00 ns (200 MHz) |
| Clock achieved (csynth estimate) | 3.364 ns (297.27 MHz) |
| Target beat by | ~49% |
| Latency min (csynth) | 1 cycle (5 ns @ target, 3.4 ns @ achieved) |
| Latency max (csynth) | 1030 cycles (5.15 µs @ target) |
| Loop II achieved | 2 (resource-limited; II=1 not reached) |

The 1030-cycle max is csynth's worst-case bound across the full 512-rule policy. cosim's measurement over the 34-case workload is far smaller (see below).

## Resource utilization on xcvu47p

| Resource | Used | Available | Percent |
|---|---|---|---|
| BRAM_18K | 0 | 4,032 | 0% |
| DSP | 0 | 9,024 | 0% |
| FF | 948 | 2,607,360 | <0.01% |
| LUT | 5,426 | 1,303,680 | ~0.4% |
| URAM | 0 | 960 | 0% |

The design occupies well under 0.5% of the chip. Plenty of headroom.

## Verification layers passed

- [x] Layer 1: CTest with g++ (35/35 cases)
- [x] **Layer 2: csim with Vitis HLS (35/35 cases)**
- [x] **Layer 3: csynth (RTL generated, timing met)**
- [x] **Layer 4: cosim — RTL simulated vs C++ reference, 35/35 match**
- [ ] Layer 5: real FPGA hardware (needs c7i.4xlarge for AFI build + f2 instance)

### Cosim measured RTL latency (at 297 MHz)

| Metric | Cycles | Absolute time |
|---|---|---|
| Min | 3 | 10 ns |
| Avg | 36 | 121 ns |
| Max | 1030 | 3.47 µs |
| Total (35 cases) | 1872 | ~6.3 µs |

Cosim's measured max matches the csynth worst-case bound exactly. The
testbench now includes a 512-rule full-scan stress test that deliberately
forces the loop to iterate through every policy slot without matching,
empirically validating the csynth-predicted upper bound.

### Why the top function is at file scope

Vitis 2025.2 cosim generates an auto-stub for the top function using
the unqualified symbol name. When the top function is inside a C++
namespace, the generated stub reference does not match the mangled
namespaced symbol, and the linker fails with `undefined symbol:
hngac_authorize_hw_stub`. Hoisting the top out of the namespace
avoids this. Types and helpers (`PolicyRule`, `test_bit`, etc.) stay
inside `hngac::fpga`; only the top function lives at file scope.

The function body in `hngac_kernel.cpp` is otherwise identical to a
namespaced version. A single `using namespace hngac::fpga;` at the
top of the file lets the body refer to `PolicyRule`, `test_bit`, and
the rest without qualification.

## What was needed to get here

1. Installed AWS CLI v2 locally
2. Configured profile `hngac-aws`
3. Subscribed to F2 Developer AMI in Marketplace
4. Got past first-paid-instance gate with `c7i-flex.large`
5. SSH'd into instance, discovered Vitis 2025.2 has no `vitis_hls` binary. The new command is `vitis-run --mode hls --tcl ...`; docs were updated to show both invocations.
6. Changed target part from `xcu250-figd2104-2L-e` (not installed on the F2 AMI) to `xcvu47p-fsvh2892-2L-e` (F2-compatible VU47P, installed).
7. Hoisted `hngac_authorize` out of `namespace hngac::fpga` so the top function lives at file scope, since Vitis 2025.2 cosim cannot resolve a namespaced top. Types and helpers stay inside the namespace.

## Caveats

- csynth **estimates** a bound; cosim **measures** the worst observed case. csynth says max latency = 1030 cycles; cosim measures max = 1030 cycles on the 35-case workload (which includes the 512-rule full-scan stress test). The two numbers now agree.
- csynth flagged the inner loop at II=2 rather than the II=1 the pragma requested. Investigation of the scheduling log identified the root cause as a ~3.7 ns combinational critical path through a 256-bit variable-offset bit extract, which does not fit in one 5 ns clock. We tried a dual-port BRAM pragma (ignored by Vitis) and cyclic ARRAY_PARTITION (pushed Fmax down to 242 MHz with no II improvement). II=1 would require redesigning the bitmask extraction itself, not a pragma change. See `syn_report/sol1.log` for the critical-path detail.
- cosim's call-to-call interval (min 17, avg 24, max 32 cycles) is a different metric from the inner-loop II. It measures how often the top function can be re-invoked over AXI-Lite, and includes the `ap_ctrl_hs` handshake overhead between calls. It is not directly comparable to the csynth inner-loop II.
- Worst-case latency of 1030 cycles = 3.47 µs at 297 MHz applies only to requests that must scan a fully populated 512-rule policy without matching. Realistic requests exit early, which is why the 35-case workload averaged 36 cycles = 121 ns.
- All numbers are post-HLS. Real silicon after Vivado place-and-route and on actual F2 hardware may differ — still pending.

## Contents of this directory

```
hw_results/
├── SUMMARY.md                        ← this file (long-form)
├── SUMMARY_BRIEF.md                  ← one-page version
├── hls_cosim_flow_reference.tcl      ← csim + csynth + cosim flow
├── vivado_pr_flow_reference.tcl      ← csynth + Vivado P&R + IP export flow
├── hngac_authorize_ip.zip            ← packaged IP from Vivado P&R
├── syn_report/                    ← csynth reports (cosim-validated run)
├── cosim_report/                     ← RTL simulation reports (35/35 PASS)
└── vivado_pr_report/                 ← post-route reports + its own SUMMARY.md
```

Key files inside `syn_report/`:
- `*_csynth.rpt` — per-module synthesis reports (latency, resources)
- `csynth.rpt` — overall synthesis summary
- `csynth_design_size.rpt` — instruction count per compilation phase
- `.xml` versions of each for machine parsing

Key files inside `cosim_report/`:
- `*_cosim.rpt` — measured RTL latency distribution
- `verilog/` — generated Verilog + simulation waveform data

## Changes made to the `hngac-fpga` repo

### Files modified

| File | What changed |
|---|---|
| `fpga/hls/src/hngac_kernel.cpp` | `hngac_authorize` moved out of `namespace hngac::fpga` to file scope. `using namespace hngac::fpga;` added at the top so the function body refers to types and helpers unchanged from the original. |
| `fpga/hls/include/hngac_kernel.hpp` | Declaration moved to file scope with fully qualified parameter types. |
| `fpga/hls/tb/hngac_kernel_tb.cpp` | Removed `using hngac::fpga::hngac_authorize;` since the function is now at file scope. All 35 test-case calls unchanged. |
| `fpga/hls/bench/hngac_compare_benchmark.cpp` | Removed `using hngac::fpga::hngac_authorize;` for the same reason. |
| `fpga/hls/scripts/vitis_hls.tcl` | Help text mentions both F2 and U250 parts. Project directory name unchanged. |
| `README.md`, `fpga/hls/README.md`, `docs/setup-aws-fpga-ami.md`, `docs/setup-vitis-wsl.md`, `docs/aws-setup-screen-by-screen.tex`, `CLAUDE.md` | Replaced `vitis_hls -f ...` with `vitis-run --mode hls --tcl ...` for Vitis 2025.x, keeping the old invocation commented as a fallback for Vitis 2024.x. Default target part swapped from `xcu250-figd2104-2L-e` to `xcvu47p-fsvh2892-2L-e` (F2-compatible VU47P). |
| `scripts/aws/fpga-dev.env` | (local only, gitignored) Set `CREATE_KEY_PAIR=true`, `TAG_OWNER=omar`, `AMI_ID=ami-092fc5deb8f3c0f7d`, `INSTANCE_TYPE=c7i-flex.large`. |
| `scripts/aws/launch_fpga_dev_instance.sh` | File mode change only (`chmod +x`), no content edit. |

### Files unchanged

| File | Why unchanged |
|---|---|
| `fpga/hls/include/hngac_types.hpp` | Types (`PolicyRule`, `Bitmask256`, `StateMask`, helpers) stay inside `hngac::fpga`. |
| All other source files | Not relevant to the HLS flow. |

## Reproduction steps

On any F2 Developer AMI. Verified with Vitis 2025.2 at `/opt/Xilinx/2025.2/`:

```bash
# 1. Source Vitis environment
source /opt/Xilinx/2025.2/Vitis/settings64.sh

# 2. Set env vars
export HNGAC_HLS_PART=xcvu47p-fsvh2892-2L-e
export HNGAC_HLS_CLOCK_NS=5.0
export HNGAC_HLS_COSIM=1        # enable cosim (slow, opt-in)
export HNGAC_HLS_WORKDIR=/tmp/hngac-fpga-hls

# 3. Run the flow (Vitis 2025.x uses vitis-run, 2024.x uses vitis_hls)
vitis-run --mode hls --tcl fpga/hls/scripts/vitis_hls.tcl
```

Expected runtime on `c7i-flex.large`: ~3 minutes total (csim ~30s, csynth ~10s, cosim ~2m).

For Vitis 2024.x the invocation is `vitis_hls -f fpga/hls/scripts/vitis_hls.tcl` instead of the `vitis-run` line above; everything else is identical.
