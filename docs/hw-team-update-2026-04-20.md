# H-NGAC Hardware Team Update

**To:** Badawy Lab HW Lead  
**From:** Hassan Karim  
**Date:** April 20, 2026  
**Re:** Scope update, revised timeline, and what we need from you

---

## The Short Version

Good news on two fronts. The deadline pressure is gone, and the repo you already have is
the right one. A few things changed since our April 12 handoff — this document gets you
current.

---

## 1. Venue Changed: IPCCC, Not SOCC

We pivoted from IEEE SOCC to **IEEE IPCCC 2026**. The IPCCC deadlines have not been
published yet. Best current estimate: abstract submission around June, final paper around
July. You have time to do this right.

The paper title and technical content are unchanged. Only the target venue shifted.

---

## 2. You Have the Right Repo

You are working from `https://github.com/coderhard/hngac-fpga`. That is correct.

The older repo I mentioned in the first message —
`https://github.com/coderhard/h-ngac-ieeedcas2026` — is the prior DCAS 2026 software
baseline. It is preserved for reference but it has none of the HLS infrastructure. Ignore
it for synthesis work.

Everything you need for HLS is in `hngac-fpga`:

| File | Purpose |
|---|---|
| `fpga/hls/src/hngac_kernel.cpp` | HLS top function — `hngac_authorize()` |
| `fpga/hls/include/` | All required types (`Bitmask256`, `StateMask`, `PolicyRule`, `AuthorizationRequest`) |
| `fpga/hls/scripts/vitis_hls.tcl` | Synthesis script — reads three env vars and runs |
| `fpga/hls/tb/hngac_kernel_tb.cpp` | Correctness testbench — 45 cases |

---

## 3. Scope Expanded: 5D Is Now in the Paper

When we last talked, the paper covered the 3D and 4D kernels. Since then we added the
5D provenance-aware extension and it is now a full section of the paper with measured
results. The 5D kernel adds one AND operation for command provenance (source type:
authenticated ROS2 node, local terminal, remote operator).

**What this means for you:** Run synthesis on both the 4D and 5D kernels. They live in
the same source file (`hngac_kernel.cpp`) and use the same TCL script. The paper needs
a synthesis report for each so we can show that 5D adds zero additional LUT stages over
4D — that is the zero-cost security dimensionality claim.

The 5D software overhead is +16.5% in our local benchmark. The hardware claim is that
this disappears on fabric because a 5-input AND tree resolves in the same LUT stage count
as a 3-input AND tree on UltraScale+. Your synthesis report will either confirm or
correct that claim.

---

## 4. Synthesis Is the Minimum Viable Deliverable

You do not need physical FPGA hardware to give us what the paper needs for submission.
Here is the priority order:

**Priority 1 — Synthesis + co-sim (required):**  
Vitis HLS synthesis reports for the 4D and 5D kernels, plus a co-simulation pass.
This runs on your local machine. No cloud access required. This gives us the LUT/timing
tables and confirms hardware/software correctness agreement.

**Priority 2 — Real hardware run (strongly preferred if available):**  
If your lab has a Xilinx UltraScale+ board (or compatible), loading the bitstream and
running the measurement harness gives us a measured round-trip latency column in the
paper. This is a meaningful upgrade but not a blocker.

**Priority 3 — AWS F2 (alternative to Priority 2):**  
AWS F1 is EOL. If we go cloud, it is F2. This requires account setup and billing
coordination with me. We can discuss if Priority 2 is not available.

---

## 5. What to Run

```bash
# Step 1: Verify correctness baseline (should print "45 passed, 0 failed")
cmake -S fpga/hls -B /tmp/hngac-build
cmake --build /tmp/hngac-build
ctest --test-dir /tmp/hngac-build --output-on-failure

# Step 2: Run synthesis (set your part ID, adjust clock if needed)
export HNGAC_HLS_PART=<your-part-id>     # e.g. xcvu9p-flgb2104-2-i for VU9P
export HNGAC_HLS_CLOCK_NS=5.0            # 200 MHz target
export HNGAC_HLS_WORKDIR=/tmp/hngac-hls  # keep off the repo tree
export HNGAC_HLS_COSIM=1                 # run co-simulation after synthesis

vitis_hls -f fpga/hls/scripts/vitis_hls.tcl
```

Synthesis reports land in `$HNGAC_HLS_WORKDIR/hngac_authorize/sol1/syn/report/`.

**Clock fallback rule:** If 200 MHz does not close timing, drop to 100 MHz
(`HNGAC_HLS_CLOCK_NS=10.0`) and proceed. At 100 MHz, a 1–3 cycle primitive is
10–30 ns — this still satisfies the paper's sub-100 ns claim. Do not burn time on
timing optimization. Drop the clock and move on.

---

## 6. What to Hand Back

Send me the following. A short email with attachments is fine.

**Required:**
- Synthesis report for the 4D kernel: LUT (logic + RAM), FF, BRAM, DSP, latency
  (cycles), initiation interval, max achievable clock frequency
- Synthesis report for the 5D kernel: same fields
- Co-simulation result: pass/fail and number of test cases matched
- Methods note (3–5 sentences): Vitis HLS version, target part, clock target,
  any fallback decisions made

**If hardware run completed:**
- Raw CSV: `request_id, latency_ns` for 10,000 requests at each of these policy
  sizes: 10, 50, 100, 500 subject-object pairs
- Two latency classes reported separately: fabric latency (from cycle count × clock)
  and host-to-FPGA round-trip (measured end-to-end)

**Format:** Plain text or CSV is fine. I will incorporate the numbers into the paper.
You do not need to draft paper text — just get me the data with enough context that I
can cite it accurately.

---

## 7. Software Baseline for Context

For reference, here are the April 18 software measurements we are comparing against.
These are from our local WSL2 benchmark (200k iterations, 1k warmup):

| Model | Mean (ns) | P99 (ns) |
|---|---|---|
| H-NGAC 3D (software) | 17.4 | 23 |
| H-NGAC 4D (software) | 18.3 | 28 |
| H-NGAC 5D (software) | 21.3 | 31 |
| RBAC + SQLite state | 376.8 | 674 |

The hardware claim is that FPGA synthesis produces a deterministic WCET bounded at
synthesis time, which the software path cannot provide regardless of mean performance.
Raw speed improvement over the software mean is a secondary benefit.

---

## Questions?

Message me directly. If something is missing from the repo or the TCL script behavior
is unclear, flag it early rather than guessing.

The full project state document is at `docs/project-overview.md` in the repo.
