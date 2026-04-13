import re
import matplotlib.pyplot as plt
import os

# 1. Read the Log File (Adjust path if needed)
# log_path = "final_data.log"
# ".." means go up one level, then into "data"
log_path = os.path.join("..", "data", "final_data.log")
latencies_pass = []
latencies_block = []
trials_pass = []
trials_block = []

print(f"Reading log: {log_path}...")

try:
    with open(log_path, "r") as f:
        lines = f.readlines()
        
    trial_count = 0
    for line in lines:
        # Regex to find PASS or BLOCK and the nanoseconds
        match = re.search(r'(PASS|BLOCK).*?Time:\s+(\d+)\s+ns', line)
        if match:
            outcome = match.group(1)
            ns = int(match.group(2))
            us = ns / 1000.0  # Convert to microseconds
            
            trial_count += 1
            if outcome == "PASS":
                latencies_pass.append(us)
                trials_pass.append(trial_count)
            else:
                latencies_block.append(us)
                trials_block.append(trial_count)

    # 2. Plotting
    # --- Conference publication figure ---
    plt.figure(figsize=(10, 6))

    # Plot Passes (Green) and Blocks (Red)
    plt.scatter(trials_pass, latencies_pass, color='green', label='Authorized (Subject 1)', marker='o')
    plt.scatter(trials_block, latencies_block, color='red', label='Blocked (Subject 99)', marker='x')

    # Formatting
    plt.xlabel('Message Sequence Number')
    plt.ylabel('Latency (microseconds)')
    plt.axhline(y=0, color='black', linewidth=1)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend()

    # Save conference version
    output_file = "ngac_latency_results.png"
    plt.savefig(output_file)
    print(f"\nSuccess! Chart saved to: {output_file}")

    # --- 48x36 poster version ---
    # Figure sized to fill ~40" wide x 24" tall on a 48x36 poster (with margins).
    # 300 DPI yields ~12000x7200 px — sufficient for large-format print.
    POSTER_FONT_SCALE = 5.0  # relative to conference defaults

    plt.rcParams.update({
        'font.size':        14 * POSTER_FONT_SCALE,
        'axes.titlesize':   16 * POSTER_FONT_SCALE,
        'axes.labelsize':   14 * POSTER_FONT_SCALE,
        'xtick.labelsize':  12 * POSTER_FONT_SCALE,
        'ytick.labelsize':  12 * POSTER_FONT_SCALE,
        'legend.fontsize':  13 * POSTER_FONT_SCALE,
    })

    fig_poster, ax_poster = plt.subplots(figsize=(40, 24))

    ax_poster.scatter(trials_pass,  latencies_pass,  color='green', label='Authorized (Subject 1)',
                      marker='o', s=200)
    ax_poster.scatter(trials_block, latencies_block, color='red',   label='Blocked (Subject 99)',
                      marker='x', s=200, linewidths=3)

    ax_poster.set_title('NGAC Gatekeeper Performance: Latency per Authorization Check')
    ax_poster.set_xlabel('Message Sequence Number')
    ax_poster.set_ylabel('Latency (microseconds)')
    ax_poster.axhline(y=0, color='black', linewidth=2)
    ax_poster.grid(True, linestyle='--', alpha=0.7)
    ax_poster.legend()

    poster_file = "ngac_latency_results_poster.png"
    fig_poster.savefig(poster_file, dpi=300, bbox_inches='tight')
    print(f"Poster chart saved to: {poster_file}")

    # Reset rcParams so nothing bleeds into other figures
    plt.rcParams.update(plt.rcParamsDefault)

except FileNotFoundError:
    print("Error: Could not find the log file. Make sure you ran the experiment!")
