# H-NGAC HLS Work Area

This directory is the active FPGA/HLS starting point for `hngac-fpga`.

The imported DCAS repository gives us the software benchmark, ROS2 harness, and prior analysis. This directory strips that down to the core authorization primitive in a form that is easier to drive through Vitis HLS.

The current implementation target is the 4D kernel:

- subject
- object
- attribute
- state

The interface also reserves a provenance field so a later 5D variant can be added without reshaping the top-level request and policy types.

## What Is Here

- `include/hngac_types.hpp`: fixed-size policy, request, state, and bitmask types
- `include/hngac_kernel.hpp`: top-level HLS candidate declaration
- `src/hngac_kernel.cpp`: HLS-oriented authorization function
- `tb/hngac_kernel_tb.cpp`: local C++ testbench for logic validation
- `bench/hngac_compare_benchmark.cpp`: local 3D vs 4D vs RBAC+lookup comparison harness
- `scripts/run_local_compare.sh`: local benchmark build-and-run wrapper
- `scripts/vitis_hls.tcl`: starter Vitis HLS script with environment-driven target settings

## Design Choices

This path deliberately avoids:

- ROS2 dependencies
- dynamic allocation
- `std::vector` policy storage
- host/runtime concerns unrelated to the authorization primitive

The goal is to keep the HLS candidate narrow: fixed-size policy array in, permit/deny out.

The first four named state bits are:

- `battery_low`
- `maintenance_mode`
- `safety_interlock`
- `calibration_required`

## Local Test

From the repo root:

```bash
cmake -S fpga/hls -B fpga/hls/build
cmake --build fpga/hls/build
ctest --test-dir fpga/hls/build --output-on-failure
./fpga/hls/build/hngac_compare_benchmark 20000 100000
```

The benchmark arguments are:

- first argument: iteration count
- second argument: modeled RBAC external-state lookup delay in nanoseconds

The RBAC comparison is intentionally modeled as a separate authorization step plus a configurable external state fetch delay so the overhead assumptions stay explicit.

The current benchmark request set is intentionally mixed:

- four requests satisfy the state constraints and should be allowed by 4D H-NGAC
- four requests match subject/object/attribute but fail the state constraints

This means the 3D baseline will tend to over-authorize relative to the 4D and RBAC+lookup paths, which is part of the point of the comparison.

For a repeatable local run with a saved log:

```bash
./fpga/hls/scripts/run_local_compare.sh 20000 100000 /tmp/hngac-fpga-local-compare
```

## Vitis HLS Starter

Set the FPGA part in the environment before launching Vitis HLS:

```bash
export HNGAC_HLS_PART=<your-installed-vu9p-part>
export HNGAC_HLS_CLOCK_NS=5.0
export HNGAC_HLS_WORKDIR=/tmp/hngac-fpga-hls
vitis_hls -f fpga/hls/scripts/vitis_hls.tcl
```

The script intentionally does not hardcode the exact part string because that can vary with the installed Vitis platform package.

By default the HLS project is created under `/tmp/hngac-fpga-hls` so synthesis work does not depend on write access inside the repo checkout.
