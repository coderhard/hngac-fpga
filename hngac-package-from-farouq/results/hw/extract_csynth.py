"""Extract csynth resource and latency numbers from csynth.rpt.

Usage:
    python3 extract_csynth.py ../cosim-opt-v1-5d/syn_report/csynth.rpt
    python3 extract_csynth.py ../cosim-opt-v1-4d/syn_report/csynth.rpt ../cosim-opt-v1-5d/syn_report/csynth.rpt --csv hw_csynth.csv
"""
import re, sys, csv, os

def parse_csynth(path):
    text = open(path, errors='replace').read()

    top = re.search(
        r'\|\+\s+hngac_authorize\s+\|'
        r'\s*[^|]*\|'    # issue type
        r'\s*[^|]*\|'    # violation type
        r'\s*[^|]*\|'    # iteration latency
        r'\s*(\d+)\|'    # interval
        r'\s*[^|]*\|'    # trip count
        r'\s*[^|]*\|'    # pipelined
        r'\s*(\d+)\|'    # latency cycles
        r'\s*[^|]*\|'    # latency ns
        r'\s*[^|]*\|'    # slack
        r'\s*[^|]*\|'    # BRAM
        r'\s*[^|]*\|'    # DSP
        r'\s*(\d+)\s*\(([^)]+)\)\|'  # FF (%)
        r'\s*(\d+)\s*\(([^)]+)\)\|', # LUT (%)
        text
    )

    loop = re.search(
        r'\|\s*o\s+VITIS_LOOP_\d+_\d+\s+\|'
        r'\s*[^|]*\|'    # issue
        r'\s*[^|]*\|'    # violation
        r'\s*(\d+)\|'    # iteration latency
        r'\s*(\d+)\|',   # II
        text
    )

    bram = re.search(r'\|\+\s+hngac_authorize\s+\|[^|]*\|[^|]*\|\s*(\d+)\s*\|', text)

    if not top:
        print(f"ERROR: could not parse {path}")
        return None

    return {
        "source": path,
        "II": int(loop.group(2)) if loop else None,
        "iteration_latency": int(loop.group(1)) if loop else None,
        "latency_max": int(top.group(2)),
        "interval_max": int(top.group(1)),
        "FF": int(top.group(3)),
        "FF_pct": top.group(4).strip(),
        "LUT": int(top.group(5)),
        "LUT_pct": top.group(6).strip(),
        "BRAM": int(bram.group(1)) if bram else 0,
        "DSP": 0,
    }

csv_out = None
if "--csv" in sys.argv:
    idx = sys.argv.index("--csv")
    csv_out = sys.argv[idx + 1]
    args = sys.argv[1:idx] + sys.argv[idx + 2:]
else:
    args = sys.argv[1:]
paths = [a for a in args if not a.startswith("--")]

results = []
for p in paths:
    r = parse_csynth(p)
    if r:
        results.append(r)
        label = os.path.basename(os.path.dirname(os.path.dirname(p)))
        print(f"=== {label}")
        print(f"source: {p}")
        print(f"{'metric':<20} {'value':>12}")
        print("-" * 34)
        print(f"{'II':<20} {r['II']:>12}")
        print(f"{'iteration latency':<20} {r['iteration_latency']:>12}")
        print(f"{'latency (max)':<20} {r['latency_max']:>12} cycles")
        print(f"{'interval (max)':<20} {r['interval_max']:>12} cycles")
        print(f"{'FF':<20} {r['FF']:>12} ({r['FF_pct']})")
        print(f"{'LUT':<20} {r['LUT']:>12} ({r['LUT_pct']})")
        print(f"{'BRAM':<20} {r['BRAM']:>12}")
        print(f"{'DSP':<20} {r['DSP']:>12}")
        print()

if csv_out and results:
    fields = ["source", "II", "iteration_latency", "latency_max", "interval_max",
              "FF", "FF_pct", "LUT", "LUT_pct", "BRAM", "DSP"]
    with open(csv_out, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(results)
    print(f"wrote {csv_out}")
