import re
import statistics
import math

log_path = "ros2_ws/final_data.log"

latencies = []

print(f"--- Analyzing {log_path} ---")

try:
    with open(log_path, "r") as f:
        for line in f:
            # Regex to find "Time: <number> ns"
            match = re.search(r'Time:\s+(\d+)\s+ns', line)
            if match:
                ns = int(match.group(1))
                latencies.append(ns)

    if not latencies:
        print("Error: No latency data found in log file.")
        exit()

    # Sort for percentiles
    latencies.sort()
    count = len(latencies)
    avg_ns = statistics.mean(latencies)
    min_ns = latencies[0]
    max_ns = latencies[-1]
    
    # Calculate Percentiles manually to avoid external dependencies
    p95_idx = math.ceil(0.95 * count) - 1
    p99_idx = math.ceil(0.99 * count) - 1
    p95_ns = latencies[p95_idx]
    p99_ns = latencies[p99_idx]

    # Convert to Microseconds for easier reading
    print("\n=== NGAC Gatekeeper Performance Metrics ===")
    print(f"Total Samples: {count}")
    print("-" * 40)
    print(f"{'Metric':<10} | {'Nanoseconds (ns)':<15} | {'Microseconds (µs)':<15}")
    print("-" * 40)
    print(f"{'Min':<10} | {min_ns:<15} | {min_ns/1000:.3f}")
    print(f"{'Average':<10} | {avg_ns:<15.2f} | {avg_ns/1000:.3f}")
    print(f"{'p95':<10} | {p95_ns:<15} | {p95_ns/1000:.3f}")
    print(f"{'p99':<10} | {p99_ns:<15} | {p99_ns/1000:.3f}")
    print(f"{'Max':<10} | {max_ns:<15} | {max_ns/1000:.3f}")
    print("-" * 40)

except FileNotFoundError:
    print(f"Error: Could not find {log_path}")
