# hngac-fpga

`hngac-fpga` is the FPGA/HLS working repo for the H-NGAC authorization primitive.

This repo starts from the IEEE DCAS 2026 software baseline and adds a clean path for the next step: pushing the core permit/deny primitive through Vitis HLS and onto FPGA.

## Current Repo Shape

There are two useful layers here:

- the imported software baseline under `benchmarks/`, `ros2_ws/`, `analysis/`, and `data/`
- the new FPGA work area under `fpga/hls/`

The software baseline is preserved so the original logic, measurements, and ROS2 harness stay close at hand. The active HLS candidate lives in `fpga/hls/`.

## Where To Start

If the goal is software reproduction, use the original benchmark and ROS2 paths.

If the goal is FPGA bring-up, start here:

- [`fpga/hls/README.md`](fpga/hls/README.md)
- [`fpga/hls/src/hngac_kernel.cpp`](fpga/hls/src/hngac_kernel.cpp)
- [`fpga/hls/tb/hngac_kernel_tb.cpp`](fpga/hls/tb/hngac_kernel_tb.cpp)

## Local HLS Candidate Test

```bash
cmake -S fpga/hls -B fpga/hls/build
cmake --build fpga/hls/build
ctest --test-dir fpga/hls/build --output-on-failure
```

## Baseline DCAS Context

The imported software package contains:

- `benchmarks/ngac_benchmark.cpp`: standalone microbenchmark for the bitmask authorization primitive
- `benchmarks/ngac_jitter.cpp`: jitter characterization
- `ros2_ws/src/ngac_auth/src/auth_node.cpp`: ROS2 gatekeeper implementation
- `analysis/plot_publication.py`: paper-figure generation
- `data/final_data.log`: prior ROS2 latency log

Representative prior results from the software baseline:

| Metric | Value |
|---|---|
| Average latency (ROS 2) | 1.05 μs |
| p99 latency | 6.37 μs |
| Worst-case (OS preemption) | 157 μs |
| Pure C++ microbenchmark | 38 ns |
| Memory footprint | < 10 KB |

## License

This software is licensed under the [Business Source License 1.1 (BUSL-1.1)](https://mariadb.com/bsl11/).

- **Licensor:** Stable Cyber LLC
- **Licensed Work:** Hardware-Accelerated NGAC Authorization for Real-Time Multi-Robot Systems
- **Additional Use Grant:** Use for academic research, education, and non-commercial experimentation is permitted, provided the accompanying paper is cited.
- **Change Date:** 2031-04-30
- **Change License:** MIT

On the Change Date, this software will automatically convert to the MIT License. Until then, commercial use, production deployment, and derivative products require a separate commercial license from Stable Cyber LLC.
