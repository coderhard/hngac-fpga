"""Extract per-rule-count kernel latency from cosim transaction reports.

Reads result.transaction.rpt (per-call latency measured by the cosim framework,
ap_start to ap_done) and tmp.log (testbench stdout labeling rule-count sections).
Produces a per-rule-count min/avg/max table.

Usage:
    python3 extract_cosim_per_rule.py ../hw-opt-v1-5d
"""
import re, sys, os, csv

def extract_latencies(txn_file):
    lats = []
    for line in open(txn_file, errors='replace'):
        m = re.match(r'transaction\s+(\d+):\s+(\d+)', line)
        if m:
            lats.append(int(m.group(2)))
    return lats

def extract_sections(wrapc_file):
    sections = []
    cur_rules = None
    cur_count = 0
    for line in open(wrapc_file, errors='replace'):
        m = re.match(r'=== (\d+) rules ===', line)
        if m:
            if cur_rules is not None:
                sections.append((cur_rules, cur_count))
            cur_rules = int(m.group(1))
            cur_count = 0
        elif 'requests=' in line and cur_rules is not None:
            m2 = re.search(r'requests=(\d+)', line)
            if m2:
                cur_count = int(m2.group(1))
    if cur_rules is not None:
        sections.append((cur_rules, cur_count))
    return sections

def main():
    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} <hw-results-dir> [--csv output.csv]")
        sys.exit(1)

    base = sys.argv[1]
    txn = os.path.join(base, "cosim_report/verilog/result.transaction.rpt")
    wrapc = os.path.join(base, "cosim_report/verilog/tmp.log")

    lats = extract_latencies(txn)
    sections = extract_sections(wrapc)

    print(f"source: {txn}")
    print(f"transactions: {len(lats)}, sections: {sections}\n")
    print(f"{'rules':>6} {'reqs':>5} {'min':>5} {'avg':>5} {'max':>5}  (cosim kernel cycles @ 100 MHz)")
    print("-" * 48)

    rows = []
    idx = 0
    for rules, nreqs in sections:
        chunk = lats[idx:idx + nreqs]
        idx += nreqs
        if chunk:
            mn, avg, mx = min(chunk), sum(chunk) / len(chunk), max(chunk)
            print(f"{rules:>6} {nreqs:>5} {mn:>5} {avg:>5.0f} {mx:>5}")
            rows.append({"rules": rules, "requests": nreqs,
                         "min_cycles": mn, "avg_cycles": round(avg),
                         "max_cycles": mx})

    csv_out = None
    if "--csv" in sys.argv:
        csv_out = sys.argv[sys.argv.index("--csv") + 1]
    if csv_out:
        with open(csv_out, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=["rules", "requests",
                               "min_cycles", "avg_cycles", "max_cycles"])
            w.writeheader()
            w.writerows(rows)
        print(f"\nwrote {csv_out}")

if __name__ == "__main__":
    main()
