# H-NGAC Harness Inspection Report
## Pre-Pivot / Post-Prompt Audit
**Date:** 2026-04-18
**Venue pivot:** IEEE SOCC 2026 (abandoned) -> IEEE IPCCC 2026 (target, ~July 14 deadline)
**Auditor:** Claude Code (claude-sonnet-4-6), session initiated by Hassan Karim

---

## 1. `benchmarks/ngac_benchmark.cpp`

**Model:** Legacy single implementation. Uses `std::bitset<128>` + `std::vector<Hyperedge>`. Not the canonical harness. 3D only (subject + object + attribute linear scan).

**Timing method:** Single `high_resolution_clock` start/end wrapping a 100k-iteration loop. Bulk average only. No per-call distribution.

**Warmup:** Yes, 1000 iterations before timed loop.

**RBAC:** None.

**NGAC-DAG:** None.

**OPA:** None.

**5D:** None.

**Output:** Avg latency (ns and µs), memory footprint (bytes). No p99, no max.

**CLI args:** None. Hardcoded 100 rules, 100k iterations, single attribute (bit 5).

**Status:** Legacy context only. Does not belong in the paper comparison table. The canonical harness supersedes it.

---

## 2. `benchmarks/ngac_jitter.cpp`

Same `std::bitset<128>` + `std::vector<Hyperedge>` 3D model as `ngac_benchmark.cpp`. Difference: measures per-call latency by bracketing each individual iteration with `high_resolution_clock::now()`. The per-call clock read adds 20-50 ns overhead, which the file itself acknowledges in its output header.

**Output:** min/avg/p95/p99/max. Previously measured result in `data/results_ngac_jitter.txt`: avg 49.9 ns, p99 79 ns, max 16,511 ns. Those numbers include 20-50 ns clock overhead per call, so the actual operation is approximately 0-30 ns. Not usable as a paper data point without that adjustment noted.

**Status:** Useful for understanding jitter shape, but uses the legacy model and carries instrument overhead. Not in the paper comparison.

---

## 3. HLS source: `fpga/hls/src/hngac_kernel.cpp` + `include/`

**Top function signature:**
```cpp
bool hngac::fpga::hngac_authorize(
    const PolicyRule policy[kMaxPolicyRules],
    uint16_t rule_count,
    const AuthorizationRequest& request)
```

**Pragmas present:**
- `HLS INTERFACE s_axilite port=return`
- `HLS INTERFACE s_axilite port=rule_count`
- `HLS INTERFACE s_axilite port=request`
- `HLS INTERFACE bram port=policy`
- `HLS PIPELINE II=1` on the rule-scan loop

**Dimensions implemented:**
- Subject bit test: Yes (3D)
- Object bit test: Yes (3D)
- Attribute containment: Yes (3D)
- State mask containment (`contains_all_states`): Yes (4D)
- Provenance check: No. `reserved_provenance` field exists in both `PolicyRule` and `AuthorizationRequest` (type `ProvenanceMask = uint32_t`) but the kernel reads neither field. It is a stub.

**5D status:** Fields reserved. Logic not written. The implementation plan explicitly deferred 5D to the current build cycle (IPCCC), which means it must be implemented now.

---

## 4. `docs/implementation-plan-4d-hngac.md`

Plan scope was 4D. It correctly reserved 5D fields without activating them. All planned local items are complete: kernel compiles, testbench passes, local benchmark runs. Plan says "AWS is for final hardware evidence only" — that still holds.

The plan is a valid record but is now outdated relative to IPCCC scope: it does not mention 5D implementation or the adversarial simulation requirements for Attack Class 2.

---

## 5. `data/final_data.log`

**Content:** ROS2 runtime log from the DCAS baseline demonstration. 3266 lines. Not a benchmark file.

- **816 `[PASS]`** events: Subject 1 (authorized), authorization latency range 534-20,022 ns. The 20 µs spike is ROS2 middleware jitter, not the authorization logic.
- **815 `[MALICIOUS]` sends** from `bad_actor_node`: Subject 99, alternating with valid commands at 1 Hz.
- **815 `[BLOCK]`** responses: Subject 99 denied every time. 100% block rate for Subject 99.
- **0 `[DENY]`** strings: auth_node uses `[BLOCK]`, not `[DENY]`, as the deny label.

**What this log proves:** 3D bitmask blocks an unknown subject (Subject 99 is simply not in the policy). This demonstrates the mechanism but not the 5D claim. The "compromised authenticated node" scenario requires a node that is in the policy but carries the wrong provenance type. That test does not exist yet.

**What this log does not prove:** Slip rate, timing-window bypass, or any 5D provenance enforcement.

---

## 6. `benchmarks/CMakeLists.txt`

CMake 3.5, C++17, `-O3 -Wall -Wextra`. No LTO. No `-march=native`. No sanitizers. Two targets: `ngac_benchmark` and `ngac_jitter`. Legacy only; does not build the canonical harness.

**Canonical harness build:** `fpga/hls/CMakeLists.txt` — CMake 3.16, C++17, `-O3 -Wall -Wextra -Wpedantic`. Optional `SQLite3` linkage with `HNGAC_HAVE_SQLITE` compile definition. Builds `hngac_kernel_tb` and `hngac_compare_benchmark`. The built binary is already present at `/home/abuhassan/local/projects/hngac-fpga-build/hngac_compare_benchmark`.

---

## 7. Fresh Benchmark Run (2026-04-18, 200k iterations, 1k warmup per model)

Command: `hngac_compare_benchmark 200000 100000`

```
RBAC hash map:           mean=28.52 ns   p99=35 ns    max=20389 ns   allowed=200000/200000
NGAC-DAG traversal:      mean=195.72 ns  p99=317 ns   max=53404 ns   allowed=200000/200000
3D baseline:             mean=17.70 ns   p99=23 ns    max=136564 ns  allowed=200000/200000
4D state-aware:          mean=19.37 ns   p99=21 ns    max=37734 ns   allowed=100000/200000
RBAC + state lookup:     mean=100554 ns  p99=103930 ns               allowed=100000/200000
RBAC + SQLite lookup:    mean=414.88 ns  p99=699 ns   max=230304 ns  allowed=100000/200000

4D vs 3D mean overhead:      +9.5%
NGAC-DAG vs 4D slowdown:     10.1x
RBAC+SQLite vs 4D slowdown:  21.4x
RBAC+modeled vs 4D slowdown: 5190x
```

**Platform:** WSL2, x86-64, ~3.3 GHz. Numbers are software-path only.

**Over-authorization in RBAC and 3D:** `allowed=200000/200000` for RBAC hash map and 3D means both models grant 100% of requests, including the 4 state-failing scenarios in the mix. They have no state dimension and cannot enforce the 4D safety constraints. The paper should present this as a correctness gap, not just a latency gap.

**4D allowed=100000/200000 is correct:** Exactly 50% of the 8-request scenario mix satisfies the state requirement. This is expected behavior.

**Max values are WSL2 OS jitter, not operation cost.** 3D max of 136,564 ns and 4D max of 37,734 ns should not appear in any claim about software WCET. The ICCCN paper's 157 µs WCET figure came from a different environment. Do not mix these in the same table.

**The -2.83% overhead figure from the prior April 13 smoke run is superseded.** Authoritative overhead is +9.5% mean. P99 is lower for 4D (21 ns) than 3D (23 ns), consistent with the 4D state check short-circuiting early on deny cases.

---

## 8. Gap Analysis for All Six Required Models

| Model | Empirical? | Gap severity | What is needed |
|---|---|---|---|
| RBAC hash map | Yes | Low | Present, compiles, runs. Paper framing must note over-authorization. |
| RBAC + external state (SQLite) | Yes | Low | Present when sqlite3 available. Confirm sqlite3 installed at build time. |
| NGAC-DAG (real BFS) | Yes | Low | Adjacency-list BFS in harness. Does not encode state; defensible as a 3D comparison. |
| H-NGAC 3D | Yes | Low | Present in canonical harness. Legacy `benchmarks/` versions are superseded. |
| H-NGAC 4D | Yes, local | High (hardware) | Local software numbers in hand. Synthesis and F1 results still pending. |
| H-NGAC 5D | No | Critical | Zero implementation. Fields exist; logic does not. Kernel, harness model, testbench cases, and HLS synthesis all needed. |

---

## 9. What Does Not Exist Yet

**5D kernel logic (~15 lines):** Add provenance containment check to `hngac_authorize`. Mirror the `contains_all_states` pattern for `ProvenanceMask`. Rename `reserved_provenance` in `PolicyRule` to `required_provenance`. Rename `reserved_provenance` in `AuthorizationRequest` to `source_provenance`.

**5D benchmark model (~30 lines):** Add a `5D provenance-aware` run to `hngac_compare_benchmark.cpp`. The key paper claim (zero LUT overhead from 5D) requires showing the same measured latency as 4D on the software path and the same cycle count from HLS synthesis.

**5D testbench cases (~30 lines):** Add to `hngac_kernel_tb.cpp`. At a minimum: provenance bit present and allowed, provenance bit absent and denied, wrong provenance type denied, superset provenance allowed, zero required_provenance is a wildcard.

**Adversarial ROS2 simulation for IPCCC (new file, ~150 lines):** The existing `bad_actor_node.cpp` demonstrates 3D blocking of an unknown subject. For IPCCC two more demonstrations are required:

- Attack Class 1 (timing window bypass): Run `auth_node` under CPU load. Record slip rate: how many malicious `/cmd_vel` messages reach the actuator subscriber before the deny arrives. Show that under FPGA timing (authorization latency < 100 ns vs DDS propagation 50-200 µs), the window is closed.
- Attack Class 2 (compromised authenticated node, 5D): A node with a valid DDS credential (Subject 1 is in the policy) but source provenance that does not match the allowed provenance type. Under 4D this passes. Under 5D it is denied. This node does not exist yet.

---

## 10. Build Order (A-to-Z standard)

**Step 1 — Implement 5D** (no hardware dependency; blocks paper claims):
1. Add provenance check to `fpga/hls/src/hngac_kernel.cpp`
2. Add 5D model to `fpga/hls/bench/hngac_compare_benchmark.cpp`
3. Add 5D testbench cases to `fpga/hls/tb/hngac_kernel_tb.cpp`
4. Confirm zero measured overhead vs 4D on software path

**Step 2 — Lock benchmark numbers** (needed for Tables 2, 3, 4):
- Run `hngac_compare_benchmark 200000 100000` after 5D is added
- Numbers from Section 7 above are current ground truth for 3D/4D/RBAC/DAG

**Step 3 — Build adversarial ROS2 simulation nodes** (blocks Section V):
- Slip rate test under loaded CPU (Attack Class 1)
- 5D compromised-node demo (Attack Class 2)

**Step 4 — HLS synthesis (Badawy lab path):**
- Vitis HLS TCL is ready at `fpga/hls/scripts/vitis_hls.tcl`
- Target: xcvu9p (or AWS F1 xcvu9p). Clock: 5 ns (200 MHz).
- Run `csim_design` then `csynth_design`. Capture utilization and timing reports.
- Zero LUT overhead claim requires synthesis report showing 5D adds no additional pipeline stage vs 4D.

**Step 5 — F1 hardware run** (blocked on AWS service limit increase):
- Fill all `\result{}` placeholders in Tables 3 and 5.

---

## 11. Prior Number Corrections Required in main.tex

| Location | Old value | Replacement |
|---|---|---|
| Section referencing -2.83% overhead | -2.83% | +9.5% mean; p99 4D (21 ns) < p99 3D (23 ns) |
| April 13 point estimates (15.526/15.087 ns) | Point estimates | Ranges from 200k-iteration warm runs |
| Any RBAC speedup from modeled baseline | 6,674x | Must become empirical; modeled number cannot appear as measurement |
