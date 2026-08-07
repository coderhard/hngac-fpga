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
freqs = []
cur = None
pending = None

for line in open(LOG, errors='replace'):
    s = summ_re.match(line)
    if s:
        pending = (s.group(1).strip(), float(s.group(2)))

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
            cur = None

ghz = sum(freqs) / len(freqs) if freqs else 0
rules = sorted({r for v in ns_data.values() for r in v})

print(f"measured clock: {ghz:.2f} GHz (mean of {len(freqs)} runs)\n")
print(f"{'model':<22}" + "".join(f"{r:>9}" for r in rules))
print("-" * (22 + 9 * len(rules)))

rows = []
for mdl in sorted(ns_data):
    vals = []
    for r in rules:
        if r in ns_data[mdl]:
            c = round(ns_data[mdl][r] * ghz)
            vals.append(c)
        else:
            vals.append(None)
    print(f"{mdl:<22}" + "".join(
        f"{v:>9}" if v is not None else f"{'-':>9}" for v in vals))
    row = {"model": mdl}
    for r, v in zip(rules, vals):
        row[f"cycles_{r}"] = v
    rows.append(row)

print(f"\nper-decision cycles = SUMMARY mean_ns x {ghz:.2f} GHz.")
print("Excludes process startup. See system-snapshot.txt for the environment.")

if "--csv" in sys.argv:
    csv_out = sys.argv[sys.argv.index("--csv") + 1]
    fields = ["model"] + [f"cycles_{r}" for r in rules]
    with open(csv_out, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)
    print(f"\nwrote {csv_out}")
