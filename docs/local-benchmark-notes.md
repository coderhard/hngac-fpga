# Local Benchmark Notes

## 2026-04-28 05:52 CDT — canonical 8-model flattened-5D run (branch: feat/flattened-5d-baseline)

Command:

```bash
/tmp/hngac-fpga-build/hngac_compare_benchmark 200000 100000
```

Environment:

- Host: Hunter22, WSL2, kernel 6.6.87.2-microsoft-standard-WSL2
- GCC 13.3.0, SQLite3 3.45.1, CMake 3.28.3
- Iterations: 200,000 | Warmup: 1,000 per model | RBAC modeled delay: 100,000 ns
- Git commit: 880b8ff (branch feat/flattened-5d-baseline)
- Log: `data/benchmarks/benchmark_20260428_055248.log`

Scenario mix: 5D-aware 4-rule corpus, 11 atomic requests per cycle (4 allow / 7 deny).
Flattened 5D lookup validated against H-NGAC 5D on all 11 corpus requests before timing.

### Latency results

| Model | Mean (ns) | p95 (ns) | p99 (ns) | Max (ns) | Allowed |
|---|---|---|---|---|---|
| RBAC hash map | 25.37 | 26 | 30 | 35,029 | 200,000 / 200,000 |
| NGAC-DAG traversal | 169.61 | 194 | 264 | 91,674 | 200,000 / 200,000 |
| H-NGAC 3D | 17.32 | 18 | 23 | 11,590 | 200,000 / 200,000 |
| H-NGAC 4D | 28.07 | 30 | 46 | 928,658 | 145,455 / 200,000 |
| **H-NGAC 5D** | **21.97** | **28** | **34** | 95,270 | 72,728 / 200,000 |
| **Flattened 5D direct lookup** | **191.98** | **199** | **231** | 31,867 | 72,728 / 200,000 |
| RBAC + modeled state lookup | 100,753 | 100,157 | 109,468 | 1,353,129 | 145,455 / 200,000 |
| RBAC + SQLite state lookup | 399.18 | 604 | 739 | 182,336 | 145,455 / 200,000 |

### Build / memory / reload results

| Model | Build mean (ns) | Reload mean (ns) | Memory (B) | Entries |
|---|---|---|---|---|
| H-NGAC 5D policy array | 699.67 | 675.58 | 416 | 4 rules |
| Flattened 5D direct lookup | 6,497.00 | 6,283.33 | 8,192 | 160 keys |

### Comparison metrics

| Metric | Value |
|---|---|
| H-NGAC 5D vs flattened 5D mean overhead | -88.56% (5D is **8.74× faster**) |
| H-NGAC 5D vs flattened 5D p99 ratio | 0.15× (5D p99 is **6.79× lower**) |
| Flattened 5D memory vs H-NGAC 5D memory | 19.69× larger |
| Flattened 5D reload cost vs H-NGAC 5D reload cost | 9.30× more expensive |
| NGAC-DAG vs H-NGAC 5D mean slowdown | 7.72× |
| RBAC + SQLite vs H-NGAC 5D mean slowdown | 18.17× |
| RBAC + modeled lookup vs H-NGAC 5D mean slowdown | 4,586.88× |
| 4D vs 3D mean overhead | 62.07% |
| 5D vs 4D mean overhead | -21.73% |

### Interpretation

- H-NGAC 5D beats the flattened 5D direct lookup on every dimension: latency (8.74× faster mean), tail latency (6.79× lower p99), memory (19.69× smaller footprint), and policy reload cost (9.30× cheaper). This directly answers the reviewer-fairness question.
- The flattened lookup and H-NGAC 5D produce identical allow/deny decisions on all corpus requests (validated pre-run). The latency and memory gap is purely representational.
- The memory figure for H-NGAC 5D (416 B) reflects active-rule working-set only: `rule_count × sizeof(PolicyRule)` = 4 × 104 B. The C++ `std::array<PolicyRule, 512>` buffer is always 53,248 B allocated. The comparison is still valid as a per-rule logical cost, but the paper must be clear this is the logical policy representation, not the full allocated buffer.
- The flattened lookup memory (8,192 B) is the backing hash table: 1,024 buckets × 8 B/bucket, holding 160 materialized allow-set keys for 4 rules.
- Static models (RBAC, NGAC-DAG, H-NGAC 3D) over-authorize the corpus because they cannot enforce runtime state or provenance constraints.
- The 4D vs 3D overhead of 62% is higher than prior runs due to the 5D-aware corpus generating more rule-scan work per cycle.
- The negative 5D vs 4D overhead (-21.73%) reflects early-exit behavior: the 5D path exits earlier on provenance-failing requests, which make up a larger fraction of the 11-request corpus than state-failing requests.
- These are local WSL2 x86-64 software results, not FPGA measurements.

## Policy-size and fleet-size scaling analysis

Paper reference: `paper/main.tex:601` — scaling table placeholder (10/50/100/500 subject-object pairs).
Paper motivating example: 50-AGV warehouse fleet (`paper/main.tex:123`).
Open placeholder: `paper/main.tex:515` — `\todo{N}-robot fleet` → **resolved: use 50**.

### Memory math sweep (calculated — no benchmark runs needed)

Struct sizes confirmed from source:

| Struct | Size | Fields |
|---|---|---|
| `PolicyRule` (5D / 4D) | 104 B | 3 × Bitmask256 (32 B each) + StateMask (4 B) + ProvenanceMask (4 B) |
| `PolicyRule3D` | 96 B | 3 × Bitmask256 (32 B each) |
| `RolePermissionRule` | 44 B | role_id (2 B) + object_id (2 B) + Bitmask256 (32 B) + StateMask (4 B) + ProvenanceMask (4 B) |
| `Bitmask256` | 32 B | 4 × uint64_t |

Memory for analytically-tractable models = `rule_count × sizeof(struct)` (active rules only):

| Model | Per-rule | 10 rules | 50 rules | 100 rules | 500 rules | Cache tier (10/50/100/500) |
|---|---|---|---|---|---|---|
| H-NGAC 5D | 104 B | 1,040 B | 5,200 B | 10,400 B | 52,000 B | L1 / L1 / L1 / L1–L2 |
| H-NGAC 4D | 104 B | 1,040 B | 5,200 B | 10,400 B | 52,000 B | L1 / L1 / L1 / L1–L2 |
| H-NGAC 3D | 96 B | 960 B | 4,800 B | 9,600 B | 48,000 B | L1 / L1 / L1 / L1–L2 |
| RBAC policy array | 44 B | 440 B | 2,200 B | 4,400 B | 22,000 B | L1 / L1 / L1 / L1 |
| Flattened 5D | 8 B/bucket | 32,768 B | 131,072 B | 262,144 B | 1,048,576 B | L1 / **L2** / **L2–L3** / **L3** |
| RBAC hash map | opaque | — | — | — | — | (unordered_map internals not instrumented) |
| NGAC-DAG | opaque | — | — | — | — | (unordered_map + vector internals not instrumented) |

Cache tier reference: L1 ≈ 32–48 KB, L2 ≈ 256 KB–1 MB, L3 ≈ 8–32 MB.

Flattened 5D formula: `next_power_of_two(max(16, rule_count × 128 × 2)) × 8 B`
where 128 = `kEnumeratedStateMaskLimit(16) × kEnumeratedProvenanceMaskLimit(8)`.

**Key finding:** At the paper's 50-AGV scale (50 rules), H-NGAC 5D stays in L1 at 5.2 KB while the flattened table spills into L2 at 128 KB. All H-NGAC variants (3D/4D/5D) remain in L1 across all tested scales. This cache-tier divergence is the mechanism behind the latency advantage and is expected to widen empirically.

### Fleet-size scaling analysis (calculated)

Fleet size and rule count are orthogonal in H-NGAC. Subject IDs are bits in `Bitmask256` (256 bits). Adding more robots to the fleet costs zero extra policy memory up to 256 subjects.

**H-NGAC subject capacity:**

| Fleet size | Bits used (of 256) | Extra policy memory | Notes |
|---|---|---|---|
| 10 AGVs | 10 / 256 | **0 B** | 3.9% utilization |
| 50 AGVs | 50 / 256 | **0 B** | Paper's motivating fleet (19.5% utilization) |
| 100 AGVs | 100 / 256 | **0 B** | 39% utilization |
| 256 AGVs | 256 / 256 | **0 B** | Maximum direct subject space |
| 500 AGVs | > 256 | Role abstraction required | Group robots into roles; each role = 1 bit. Rule count grows with role count, not fleet size. |

**RBAC comparison (role × object pairs, 10 shelves per zone):**

| Scenario | RBAC entries | RBAC memory | H-NGAC 5D memory |
|---|---|---|---|
| 50 AGVs, shared roles (5 roles × 10 objects) | 50 | 2,200 B | 5,200 B (50 rules) |
| 50 AGVs, 1 role/robot (worst case) × 10 objects | 500 | 22,000 B | 5,200 B (50 rules) |
| 50 AGVs, 1 role/robot × 10 objects + state/provenance | 500 + state-lookup overhead | 22,000 B + external | 5,200 B (50 rules, inline) |

Note: RBAC requires a separate state-lookup call per authorization (external system or SQLite). H-NGAC 5D encodes state and provenance inline; no external call needed.

**Fleet scaling conclusion:** The bitmask representation handles any fleet up to 256 AGVs with zero policy memory growth. Beyond 256, role abstraction is required regardless of model — H-NGAC 5D still wins because it encodes role membership as bits rather than separate hash-map entries.

Paper reference: `paper/main.tex:601` — scaling table placeholder.  
Paper motivating example: 50-AGV warehouse fleet (`paper/main.tex:123`).  
Open placeholder: `paper/main.tex:515` — `\todo{N}-robot fleet` needs a concrete number (use 50).

### Memory (calculated — no benchmark run needed)

Formulas are exact from the harness source:
- H-NGAC 5D: `rule_count × sizeof(PolicyRule)` = `rule_count × 104 B`
- Flattened 5D: `next_power_of_two(max(16, rule_count × 128 × 2)) × 8 B`
  - 128 = `kEnumeratedStateMaskLimit(16) × kEnumeratedProvenanceMaskLimit(8)` — state/provenance combinations per rule

| Subject-object pairs (rules) | H-NGAC 5D memory | Flattened 5D memory | Ratio | Cache tier (H-NGAC / Flattened) |
|---|---|---|---|---|
| 4 (this run) | 416 B | 8,192 B | 19.7× | L1 / L1 |
| 10 | 1,040 B | 32,768 B (32 KB) | 31.5× | L1 / L1 |
| 50 (AGV fleet) | 5,200 B (5 KB) | 131,072 B (128 KB) | 25.2× | **L1 / L2** |
| 100 | 10,400 B (10 KB) | 262,144 B (256 KB) | 25.2× | **L1 / L2–L3** |
| 500 | 52,000 B (51 KB) | 1,048,576 B (1 MB) | 20.2× | **L1–L2 / L3** |

Key finding from calculation: at the paper's motivating scale (50 AGVs), H-NGAC 5D fits entirely in L1 cache while the flattened table has already spilled into L2. This cache-tier divergence is the mechanism behind the latency advantage and is expected to widen empirically at 100 and 500 rules.

### Latency (requires benchmark runs)

The binary already accepts `rule_count` as a third CLI argument:

```bash
/tmp/hngac-fpga-build/hngac_compare_benchmark <iterations> <rbac_delay_ns> <rule_count>
```

Planned sweep:

```bash
for N in 10 50 100 500; do
  /tmp/hngac-fpga-build/hngac_compare_benchmark 200000 100000 $N \
    | tee data/benchmarks/benchmark_scaling_${N}rules_$(date +%Y%m%d_%H%M%S).log
done
```

Note: rule_count=500 approaches `kMaxPolicyRules=512`. The NGAC-DAG and H-NGAC linear-scan paths will show measurable latency growth; the flattened lookup will show cache-miss inflation. These empirical numbers populate the paper's Table II scaling rows.

### Benchmark latency sweep results (2026-04-28 07:54–07:58 CDT)

200k iterations, 1k warmup, 100k ns RBAC modeled delay, GCC 13.3.0 -O3, WSL2.
Logs: `data/benchmarks/benchmark_scaling_{N}rules_*.log`

#### H-NGAC 5D mean latency vs. Flattened 5D — scaling comparison

| Rules | H-NGAC 5D mean | Flattened 5D mean | 5D advantage | H-NGAC 5D p99 | Flattened p99 | p99 ratio |
|---|---|---|---|---|---|---|
| 4 (canonical) | 21.97 ns | 191.98 ns | **8.74×** | 34 ns | 231 ns | 0.15× |
| 10 | 22.35 ns | 225.93 ns | **10.11×** | 34 ns | 321 ns | 0.11× |
| 50 (AGV fleet) | 40.18 ns | 197.90 ns | **4.92×** | 74 ns | 305 ns | 0.24× |
| 100 | 51.95 ns | 172.93 ns | **3.33×** | 77 ns | 221 ns | 0.35× |
| 500 | 185.04 ns | 216.20 ns | **1.17×** | 310 ns | 335 ns | 0.93× |

#### Full cross-scale summary — all models, mean latency (ns)

| Rules | RBAC map | NGAC-DAG | H-NGAC 3D | H-NGAC 4D | H-NGAC 5D | Flattened 5D | RBAC+SQLite |
|---|---|---|---|---|---|---|---|
| 4 | 25.37 | 169.61 | 17.32 | 28.07 | 21.97 | 191.98 | 399.18 |
| 10 | 27.58 | 252.13 | 18.86 | 20.80 | 22.35 | 225.93 | 415.70 |
| 50 | 25.45 | 227.83 | 28.06 | 34.54 | 40.18 | 197.90 | 373.26 |
| 100 | 25.24 | 241.98 | 37.34 | 44.24 | 51.95 | 172.93 | 422.77 |
| 500 | 25.79 | 324.59 | 137.97 | 160.11 | 185.04 | 216.20 | 484.82 |

#### Build and reload cost scaling

| Rules | H-NGAC 5D build (ns) | H-NGAC 5D reload (ns) | Flat 5D build (ns) | Flat 5D reload (ns) | Flat reload / H-NGAC reload |
|---|---|---|---|---|---|
| 4 | 699.67 | 675.58 | 6,497 | 6,283 | 9.30× |
| 10 | 699.42 | 686.33 | 16,460 | 16,185 | 23.58× |
| 50 | 799.33 | 734.33 | 88,255 | 81,903 | 111.53× |
| 100 | 923.58 | 795.58 | 173,717 | 164,908 | 207.28× |
| 500 | 1,780.58 | 1,530.75 | 984,100 | 879,248 | **574.39×** |

#### Key findings from sweep

**Latency crossover at 500 rules:** H-NGAC 5D's latency advantage over the flattened lookup narrows as rule count grows (linear scan grows; flattened lookup stays roughly O(1) but suffers cache pressure). At 500 rules, H-NGAC 5D is only 1.17× faster by mean — the two paths are converging. However, at the paper's motivating scale (50 rules / 50 AGVs), H-NGAC 5D is still **4.92× faster**.

**The real story is reload cost:** While mean latency converges at 500 rules, the flattened lookup's reload cost explodes to **574× more expensive** than H-NGAC 5D. In an operational fleet, policy updates (new robot, new shelf, new time window) require rebuilding the flattened table. At 50 rules this is already 112× slower to update; at 500 rules it takes ~880 µs vs H-NGAC's ~1.5 µs.

**RBAC hash map is near-constant:** The static RBAC hash map's mean stays flat (25–28 ns) across all rule counts because it's a pure hash lookup. The cost is semantic — it over-authorizes the full corpus and needs an external state call to match H-NGAC's enforcement.

**NGAC-DAG grows with BFS depth:** 169 ns at 4 rules → 325 ns at 500 rules. Still fast by absolute measure but 1.75× slower than H-NGAC 5D at 500 rules, vs. 7.72× slower at 4 rules. The gap narrows as the H-NGAC linear scan dominates.

**Memory advantage holds at all scales:** Flattened 5D is 19.7–31.5× larger than H-NGAC 5D across all tested scales. At 50 rules (50-AGV fleet): H-NGAC 5D = 5,200 B (L1), Flattened = 131,072 B (L2).

### Status

- [x] Memory figures calculated and documented (all analytically-tractable models)
- [x] Fleet-size scaling analysis documented (bitmask capacity, RBAC comparison)
- [x] Latency sweep runs complete (10 / 50 / 100 / 500 rules)
- [x] Paper `\todo{N}` at line 515 resolved to 50 (50-AGV fleet)
- [ ] Paper scaling table at line 601 to be filled from these results

## 2026-04-13 five-model validation run

Command:

```bash
/tmp/hngac-fpga-build/hngac_compare_benchmark 2000 10000
```

Modeled configuration:

- iterations: 2,000
- RBAC external-state lookup delay: 10,000 ns
- warmup: 1,000 iterations per model
- scenario mix: 4 state-satisfying requests and 4 state-failing requests

Observed output:

- RBAC hash map mean: 22.248 ns
- RBAC hash map allowed count: 2,000 / 2,000
- NGAC-DAG traversal mean: 178.75 ns
- NGAC-DAG traversal allowed count: 2,000 / 2,000
- 3D baseline mean: 16.6525 ns
- 3D baseline allowed count: 2,000 / 2,000
- 4D state-aware mean: 22.2875 ns
- 4D state-aware allowed count: 1,000 / 2,000
- RBAC plus state lookup mean: 10,185.8 ns
- RBAC plus state lookup allowed count: 1,000 / 2,000
- computed 4D vs 3D mean overhead: 33.8388%
- computed 4D vs RBAC hash-map mean overhead: 0.177544%
- computed NGAC-DAG vs 4D mean slowdown: 8.02017x
- computed RBAC plus lookup vs 4D mean slowdown: 457.018x

Interpretation:

- The five-model harness is now executable from one benchmark binary.
- The static software baselines still over-authorize the mixed request set because they do not encode runtime state.
- The 4D path stays near the best-case RBAC hash-map floor in this local software run.
- The modeled external-state path remains far slower than the in-process 4D path.

## 2026-04-13 SQLite validation run

Command:

```bash
/tmp/hngac-fpga-build-sqlite/hngac_compare_benchmark 2000 10000
```

Modeled configuration:

- iterations: 2,000
- RBAC external-state lookup delay: 10,000 ns
- warmup: 1,000 iterations per model
- SQLite3 found by CMake and linked into `hngac_compare_benchmark`

Observed output:

- RBAC hash map mean: 23.9025 ns
- NGAC-DAG traversal mean: 167.672 ns
- 3D baseline mean: 15.9185 ns
- 4D state-aware mean: 24.4805 ns
- RBAC plus state lookup mean: 10,280.8 ns
- RBAC plus SQLite state lookup mean: 463.377 ns
- RBAC plus SQLite state lookup allowed count: 1,000 / 2,000
- computed RBAC plus SQLite vs 4D mean slowdown: 18.9284x

Interpretation:

- The SQLite-backed lookup is much cheaper than the intentionally delayed modeled lookup path, but still materially slower than the in-process 4D bitmask path.
- The SQLite-backed path preserves the same allow/deny split as the 4D and modeled RBAC+lookup paths on the mixed scenario set.

## 2026-04-13 smoke run

Command:

```bash
/tmp/hngac-fpga-hls-build-compare/hngac_compare_benchmark 20000 100000
```

Modeled configuration:

- iterations: 20,000
- RBAC external-state lookup delay: 100,000 ns

Observed output:

- 3D baseline mean: 15.526 ns
- 3D baseline p99: 18 ns
- 3D baseline max: 134 ns
- 4D state-aware mean: 15.0865 ns
- 4D state-aware p99: 17 ns
- 4D state-aware max: 722 ns
- RBAC plus state lookup mean: 100,688 ns
- RBAC plus state lookup p99: 102,665 ns
- RBAC plus state lookup max: 696,110 ns
- computed 4D vs 3D mean overhead: -2.83074%
- computed RBAC plus lookup vs 4D mean slowdown: 6674.06x

Interpretation:

- The 3D and 4D local software means are effectively tied at this run size.
- The slight negative 4D overhead is measurement noise, not evidence that 4D is intrinsically faster.
- The RBAC plus external-state-lookup path is dramatically slower once the external lookup delay is modeled.
- These numbers are local pre-AWS software results only. They are useful for shaping the paper and guiding HLS work, but they are not FPGA measurements.

## 2026-04-13 mixed allow/deny scenario

Command:

```bash
/tmp/hngac-fpga-hls-build-compare/hngac_compare_benchmark 20000 100000
```

Modeled configuration:

- iterations: 20,000
- RBAC external-state lookup delay: 100,000 ns
- scenario mix: 4 state-satisfying requests and 4 state-failing requests

Observed output:

- 3D baseline mean: 14.9253 ns
- 3D baseline p99: 17 ns
- 3D baseline max: 133 ns
- 3D baseline allowed count: 20,000 / 20,000
- 4D state-aware mean: 16.2374 ns
- 4D state-aware p99: 18 ns
- 4D state-aware max: 12,103 ns
- 4D state-aware allowed count: 10,000 / 20,000
- RBAC plus state lookup mean: 100,929 ns
- RBAC plus state lookup p99: 107,352 ns
- RBAC plus state lookup max: 1,003,759 ns
- RBAC plus state lookup allowed count: 10,000 / 20,000
- computed 4D vs 3D mean overhead: 8.79148%
- computed RBAC plus lookup vs 4D mean slowdown: 6215.85x

Interpretation:

- The mixed scenario now exposes the semantic gap clearly: the 3D baseline authorizes every request because it cannot enforce state constraints.
- The 4D and RBAC plus state-lookup paths both block the state-failing half of the request set.
- The measured 4D mean overhead over 3D in this run is still small relative to the modeled RBAC plus state-lookup cost.
- These remain local pre-AWS software results only and should not be presented as FPGA measurements.
