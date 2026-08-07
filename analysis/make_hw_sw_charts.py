"""Paper figures: hardware vs software cost per authorization decision.

Run:  python3 analysis/make_hw_sw_charts.py   (needs matplotlib)
Writes docs/figures/chart-sw-models.png and docs/figures/chart-key-finding.png.

Data source: hngac-package-from-farouq/results/{sw/sw_cycles.csv, hw/hw_cycles_per_rule_5d.csv}
Palette: dataviz reference categorical slots 1-6 (validated light mode, all checks pass;
contrast WARN relieved by direct labels on every series plus the table view in the doc).
"""
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "docs", "figures")
os.makedirs(OUT, exist_ok=True)

SURFACE = "#fcfcfb"
INK = "#0b0b0b"
INK2 = "#52514e"
GRID = "#e0dfda"

# categorical slots, fixed order, never cycled
S1, S2, S3, S4, S5, S6 = "#2a78d6", "#eb6834", "#1baf7a", "#eda100", "#e87ba4", "#008300"

RULES = [4, 10, 50, 100, 200, 500]

SW = [
    ("H-NGAC 3D",           [70, 75, 115, 151, 211, 390],      S1),
    ("H-NGAC 4D",           [74, 78, 120, 194, 292, 582],      S2),
    ("H-NGAC 5D",           [80, 82, 130, 221, 360, 685],      S3),
    ("RBAC hash map",       [97, 87, 93, 91, 89, 100],         S4),
    ("NGAC-DAG traversal",  [608, 638, 636, 629, 636, 1032],   S5),
    ("Flattened 5D lookup", [990, 1016, 970, 1003, 1003, 984], S6),
]
HW = [14, 17, 37, 62, 112, 262]

MEASURED_AT = "Measured at policy sizes 4, 10, 50, 100, 200 and 500 rules."


def style(ax, title, subtitle, ylabel):
    fig = ax.figure
    ax.set_facecolor(SURFACE)
    # title/subtitle in figure space so multi-line subtitles cannot ride up into the title
    fig.text(0.098, 0.965, title, color=INK, fontsize=12, fontweight="bold",
             va="top", ha="left")
    fig.text(0.098, 0.905, subtitle, color=INK2, fontsize=8.8,
             va="top", ha="left", linespacing=1.4)
    ax.set_xlabel("Policy size (number of rules)", color=INK2, fontsize=9.5, labelpad=7)
    ax.set_ylabel(ylabel, color=INK2, fontsize=9.5, labelpad=7)
    ax.grid(True, color=GRID, linewidth=0.8, zorder=0)
    ax.set_axisbelow(True)
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)
    for s in ("left", "bottom"):
        ax.spines[s].set_color(GRID)
        ax.spines[s].set_linewidth(1.0)
    ax.tick_params(colors=INK2, labelsize=9, length=0)
    # regular ticks: 4 and 10 are unlabelable at this scale, the subtitle names them
    ax.set_xticks([0, 100, 200, 300, 400, 500])
    ax.set_xlim(-8, 500)


def declutter(labels, min_gap):
    """Nudge overlapping right-edge labels apart, preserving vertical order."""
    labels.sort(key=lambda t: t[0])
    for i in range(1, len(labels)):
        if labels[i][0] - labels[i - 1][0] < min_gap:
            labels[i] = (labels[i - 1][0] + min_gap, *labels[i][1:])
    return labels


# ---------- Chart 1: the six software models ----------
fig, ax = plt.subplots(figsize=(7.2, 4.4), dpi=200)
fig.patch.set_facecolor(SURFACE)

for name, ys, color in SW:
    ax.plot(RULES, ys, color=color, linewidth=2, marker="o", markersize=5,
            markeredgecolor=SURFACE, markeredgewidth=1.1, label=name, zorder=3,
            clip_on=False)

style(ax, "Software cost per authorization decision",
      "CPU cycles on Intel i7-12800H at 4.96 GHz, 200k iterations per point.\n"
      "Lower is better. " + MEASURED_AT,
      "CPU cycles per decision")
ax.set_ylim(0, 1120)

for y, name, color in declutter([(ys[-1], name, color) for name, ys, color in SW], 66):
    ax.text(510, y, name, color=color, fontsize=8.8, va="center", ha="left",
            fontweight="bold")

ax.legend(frameon=False, fontsize=8.6, labelcolor=INK2, ncol=3,
          loc="upper left", bbox_to_anchor=(0, -0.19), handlelength=1.7,
          columnspacing=1.4)
fig.text(0.012, 0.012, "Source: results/sw/perf_all_models_scaling.log (2026-08-04)",
         color=INK2, fontsize=7.6)
fig.subplots_adjust(left=0.098, right=0.735, top=0.775, bottom=0.30)
fig.savefig(os.path.join(OUT, "chart-sw-models.png"), facecolor=SURFACE)
plt.close(fig)

# ---------- Chart 2: the key finding, software slope vs hardware slope ----------
fig, ax = plt.subplots(figsize=(7.2, 4.4), dpi=200)
fig.patch.set_facecolor(SURFACE)

for name, ys, color in SW[:3]:
    ax.plot(RULES, ys, color=color, linewidth=2, marker="o", markersize=5,
            markeredgecolor=SURFACE, markeredgewidth=1.1,
            label=f"{name} (software)", zorder=3, clip_on=False)

ax.plot(RULES, HW, color=INK, linewidth=2.6, marker="s", markersize=5,
        markeredgecolor=SURFACE, markeredgewidth=1.1,
        label="Hardware 4D and 5D (one line, identical)", zorder=4, clip_on=False)

style(ax, "Adding a dimension is free in hardware, not in software",
      "Cycles per decision. Each software dimension steepens the slope; the 4D and 5D\n"
      "hardware curves coincide exactly and plot as one line.\n" + MEASURED_AT,
      "Cycles per decision")
ax.set_ylim(0, 745)

lab = [(ys[-1], name, f"{(ys[-1] - ys[0]) / 496:.2f} cycles/rule", color)
       for name, ys, color in SW[:3]]
lab.append((HW[-1], "Hardware 4D = 5D",
            f"{(HW[-1] - HW[0]) / 496:.2f} cycles/rule", INK))
for y, name, slope, color in declutter(lab, 78):
    ax.text(510, y + 14, name, color=color, fontsize=8.8, va="center",
            ha="left", fontweight="bold")
    ax.text(510, y - 14, slope, color=color, fontsize=8.2, va="center", ha="left")

ax.legend(frameon=False, fontsize=8.6, labelcolor=INK2, ncol=2,
          loc="upper left", bbox_to_anchor=(0, -0.19), handlelength=1.7,
          columnspacing=1.4)
fig.text(0.012, 0.012,
         "Source: results/sw/perf_all_models_scaling.log; "
         "results/hw/hw_cycles_per_rule_4d.csv and _5d.csv",
         color=INK2, fontsize=7.6)
fig.subplots_adjust(left=0.098, right=0.735, top=0.775, bottom=0.30)
fig.savefig(os.path.join(OUT, "chart-key-finding.png"), facecolor=SURFACE)
plt.close(fig)

print(f"wrote chart-sw-models.png and chart-key-finding.png to {OUT}")
