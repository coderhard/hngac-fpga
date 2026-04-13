# Hardware-Accelerated NGAC Authorization for Real-Time Multi-Robot Systems

Companion code for the IEEE DCAS 2026 paper:

> Hassan Karim, Sai Sitharaman, Deepti Gupta. "Hardware-Accelerated NGAC Authorization for Real-Time Multi-Robot Systems." *IEEE DCAS 2026.*

This repository contains the ROS 2 implementation, microbenchmarks, raw experimental data, and analysis scripts used to produce the results in the paper.

---

## Repository Structure

```
hw-gac_project_share/
├── ros2_ws/src/ngac_auth/src/
│   ├── auth_node.cpp          # NGAC Gatekeeper node (policy enforcement)
│   └── bad_actor_node.cpp     # Fault injection node (authorized + unauthorized commands)
├── benchmarks/
│   ├── ngac_benchmark.cpp     # Core bitmask authorization microbenchmark
│   ├── ngac_jitter.cpp        # Jitter characterization benchmark
│   └── run_memory_profile.sh  # Memory footprint profiling script
├── analysis/
│   ├── plot_publication.py    # Generates the latency scatter plot (Figure 1)
│   ├── calculate_stats.py     # Computes latency statistics from log data
│   └── plot_results.py        # Exploratory plot (earlier version)
├── data/
│   ├── final_data.log         # Raw ROS 2 authorization latency log (N=1,631 samples)
│   └── results_ngac_jitter.txt # Microbenchmark jitter results
└── getting-started/
    ├── STEP_1_BEFORE_ROS2.md  # Environment setup guide
    ├── install_ROS2.bash       # ROS 2 Jazzy install script
    └── jitter_checker.bash     # System jitter diagnostic
```

---

## Requirements

- Ubuntu 24.04 (or WSL2 equivalent)
- ROS 2 Jazzy
- C++17
- Python 3.12+ with `matplotlib`

See `getting-started/STEP_1_BEFORE_ROS2.md` for full environment setup.

---

## Building the ROS 2 Workspace

```bash
cd ros2_ws
colcon build
source install/setup.bash
```

---

## Running the Experiment

Open three terminals, sourcing the workspace in each:

**Terminal 1 — NGAC Gatekeeper:**
```bash
ros2 run ngac_auth auth_node
```

**Terminal 2 — Fault Injection (Bad Actor):**
```bash
ros2 run ngac_auth bad_actor_node
```

**Terminal 3 — Log capture:**
```bash
ros2 topic echo /rosout > final_data.log
```

The bad actor alternates between Subject 1 (authorized) and Subject 99 (unauthorized) at 2 Hz. The gatekeeper logs `[PASS]` or `[BLOCK]` with nanosecond timestamps.

---

## Reproducing the Paper Figure

From the repo root:

```bash
python analysis/plot_publication.py
```

Outputs `ngac_latency_results_v2.pdf` — the scatter plot used as Figure 1 in the paper.

---

## Key Results

| Metric | Value |
|---|---|
| Average latency (ROS 2) | 1.05 μs |
| p99 latency | 6.37 μs |
| Worst-case (OS preemption) | 157 μs |
| Pure C++ microbenchmark | 38 ns |
| Memory footprint | < 10 KB |
| Samples | N = 1,631 |

---

## License

This software is licensed under the [Business Source License 1.1 (BUSL-1.1)](https://mariadb.com/bsl11/).

- **Licensor:** Stable Cyber LLC
- **Licensed Work:** Hardware-Accelerated NGAC Authorization for Real-Time Multi-Robot Systems
- **Additional Use Grant:** Use for academic research, education, and non-commercial experimentation is permitted, provided the accompanying paper is cited.
- **Change Date:** 2031-04-30
- **Change License:** MIT

On the Change Date, this software will automatically convert to the MIT License. Until then, commercial use, production deployment, and derivative products require a separate commercial license from Stable Cyber LLC.

For licensing inquiries: hassan@stablecyber.com
