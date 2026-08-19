"""Extract per-decision SW cycle counts from perf_all_models_scaling.log.

Pairs SUMMARY lines (per-decision mean nanoseconds) with the cpu_core/cycles
and elapsed time from the perf stat block to derive the effective clock rate,
then converts nanoseconds to cycles.

Usage:
    python3 extract_sw_cycles.py [--csv sw_cycles.csv]
"""
import re, sys, os, csv, collections

LOG = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                   "perf_all_models_scaling.log")

hdr = re.compile(
    r"Performance counter stats for '.*?\s+(\d+)\s+\d+\s+(\d+)\s+(.+?)':")
cyc_re = re.compile(r"^\s*([\d,]+)\s+cpu_core/cycles/")
els_re = re.compile(r"^\s*([\d.]+) seconds time elapsed")
summ_re = re.compile(r"^SUMMARY\|([^|]+)\|([\d.]+)\|")

ns_data = collections.defaultdict(dict)
p95_data = collections.defaultdict(dict)
p99_data = collections.defaultdict(dict)
max_data = collections.defaultdict(dict)
freqs = []
cur = None
pending = None

for line in open(LOG, errors='replace'):
    s = summ_re.match(line)
    if s:
        parts = line.strip().split("|")
        pending = (parts[1].strip(), float(parts[2]),
                   float(parts[3]), float(parts[4]), float(parts[5]))

    m = hdr.search(line)
    if m:
        cur = (m.group(3).strip(), int(m.group(2)))
        c0 = None
        continue

    if cur:
        c = cyc_re.match(line)
        if c:
            c0 = int(c.group(1).replace(",", ""))
        e = els_re.match(line)
        if e and c0:
            f = c0 / float(e.group(1)) / 1e9
            freqs.append(f)
            if pending:
                # perf stat uses the CLI model name (e.g. "NGAC-DAG"),
                # SUMMARY uses the display name (e.g. "NGAC-DAG traversal").
                # Match if the perf name is a prefix of the SUMMARY name.
                perf_name = cur[0]
                summ_name = pending[0]
                if perf_name == summ_name or summ_name.startswith(perf_name):
                    ns_data[summ_name][cur[1]] = pending[1]
                    p95_data[summ_name][cur[1]] = pending[2]
                    p99_data[summ_name][cur[1]] = pending[3]
                    max_data[summ_name][cur[1]] = pending[4]
            cur = None

ghz = sum(freqs) / len(freqs) if freqs else 0
rules = sorted({r for v in ns_data.values() for r in v})

print(f"measured clock: {ghz:.2f} GHz (mean of {len(freqs)} runs)\n")

rows = []
print(f"{'model':<30} {'rules':>5} {'mean':>6} {'p95':>6} {'p99':>6} {'max':>9}")
print("-" * 68)
for mdl in sorted(ns_data):
    for r in rules:
        if r not in ns_data[mdl]:
            continue
        mean = round(ns_data[mdl][r] * ghz)
        p95 = round(p95_data[mdl][r] * ghz) if r in p95_data[mdl] else None
        p99 = round(p99_data[mdl][r] * ghz) if r in p99_data[mdl] else None
        mx = round(max_data[mdl][r] * ghz) if r in max_data[mdl] else None
        print(f"{mdl:<30} {r:>5} {mean:>6} {p95:>6} {p99:>6} {mx:>9}")
        rows.append({"model": mdl, "rules": r, "mean": mean,
                      "p95": p95, "p99": p99, "max": mx})

print(f"\nper-decision cycles = SUMMARY mean_ns x {ghz:.2f} GHz.")

if "--csv" in sys.argv:
    csv_out = sys.argv[sys.argv.index("--csv") + 1]
    with open(csv_out, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["model", "rules", "mean", "p95", "p99", "max"])
        w.writeheader()
        w.writerows(rows)
    print(f"wrote {csv_out}")
