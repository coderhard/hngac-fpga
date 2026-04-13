import re
import matplotlib.pyplot as plt

# 1. Setup for Publication Quality
plt.rcParams.update({
    'font.size': 14,
    'axes.labelsize': 16,
    'xtick.labelsize': 14,
    'ytick.labelsize': 14,
    'legend.fontsize': 14,
    'font.family': 'serif'
})

log_path = "ros2_ws/final_data.log"
latencies_pass = []
latencies_block = []
trials_pass = []
trials_block = []

try:
    with open(log_path, "r") as f:
        lines = f.readlines()
        
    trial_count = 0
    for line in lines:
        match = re.search(r'(PASS|BLOCK).*?Time:\s+(\d+)\s+ns', line)
        if match:
            outcome = match.group(1)
            ns = int(match.group(2))
            us = ns / 1000.0
            
            trial_count += 1
            if outcome == "PASS":
                latencies_pass.append(us)
                trials_pass.append(trial_count)
            else:
                latencies_block.append(us)
                trials_block.append(trial_count)

    # 3. Plotting
    plt.figure(figsize=(10, 6))
    
    # INCREASED MARKER SIZE (s=50) for visibility
    plt.scatter(trials_pass, latencies_pass, color='green', label='Authorized (Subject 1)', marker='o', s=50, alpha=0.8, edgecolors='black', linewidth=0.5)
    plt.scatter(trials_block, latencies_block, color='red', label='Blocked (Subject 99)', marker='x', s=50, alpha=0.9, linewidth=1.5)

    # Formatting
    plt.xlabel('Message Sequence Number')
    plt.ylabel(r'Latency ($\mu$s)')
    plt.axhline(y=0, color='black', linewidth=1)
    plt.grid(True, linestyle='--', alpha=0.5)
    
    # FIX 1: Move Legend to Upper Left (away from the outlier)
    plt.legend(loc='upper left', framealpha=1.0, edgecolor='black')

    # FIX 2: Add 15% headroom above the highest point so it doesn't touch the border
    max_y = max(max(latencies_pass), max(latencies_block))
    plt.ylim(top=max_y * 1.15)
    
    plt.tight_layout()
    
    # Save as PDF
    output_file = "ngac_latency_results_v2.pdf"
    plt.savefig(output_file, format='pdf', dpi=300)
    print(f"\nSuccess! Fixed chart saved to: {output_file}")

except FileNotFoundError:
    print(f"Error: Could not find {log_path}")
