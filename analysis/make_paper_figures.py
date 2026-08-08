"""IEEE print figures for the IPCCC 2026 manuscript (paper/main.tex).

Run:  python3 analysis/make_paper_figures.py   (needs matplotlib)
Writes vector PDFs to paper/figures/ (what the paper includes) and PNG
previews to docs/figures/ for quick viewing.

Distinct from make_hw_sw_charts.py, which renders presentation/dashboard
versions with embedded titles. Paper figures carry no internal title,
subtitle or source line; the LaTeX caption does that work. Sized at
column width (3.5 in) so fonts print at their stated size.

Data sources (docs/canonical-context.md wins on any disagreement):
  SW cycles:  derived, mean ns x 4.96 GHz perf-measured clock,
              hngac-package-from-farouq/results/sw/sw_cycles.csv
  SW ns:      SUMMARY| lines in results/sw/perf_all_models_scaling.log
              (mean and max fields; max = worst observed, not a bound)
  HW cycles:  co-simulation, results/hw/hw_cycles_per_rule_{4d,5d}.csv,
              min = mean = max at every point, L(n) = 12 + n/2
"""
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_PDF = os.path.join(HERE, "..", "paper", "figures")
OUT_PNG = os.path.join(HERE, "..", "docs", "figures")
for d in (OUT_PDF, OUT_PNG):
    os.makedirs(d, exist_ok=True)

# IEEEtran body is Times; match it and embed TrueType outlines.
plt.rcParams.update({
    "font.family": "serif",
    "font.serif": ["Times New Roman", "Nimbus Roman", "Nimbus Roman No9 L",
                   "TeX Gyre Termes", "STIXGeneral", "DejaVu Serif"],
    "mathtext.fontset": "stix",
    "pdf.fonttype": 42,
    "font.size": 8,
    "axes.labelsize": 8,
    "xtick.labelsize": 7,
    "ytick.labelsize": 7,
    "axes.linewidth": 0.6,
    "grid.linewidth": 0.4,
    "lines.linewidth": 1.1,
})

INK = "#000000"
GRID = "#c8c8c8"
# categorical slots from the dataviz palette, fixed order; every series also
# carries a distinct marker + linestyle so grayscale print keeps identity
S1, S2, S3 = "#2a78d6", "#eb6834", "#1baf7a"

RULES = [4, 10, 50, 100, 200, 500]

# software cycles: derived (mean ns x 4.96 GHz), NOT counter reads
SW3D = [70, 75, 115, 151, 211, 390]
SW4D = [74, 78, 120, 194, 292, 582]
SW5D = [80, 82, 130, 221, 360, 685]
# hardware cycles: co-simulated, min = mean = max, 4D and 5D identical
HW = [14, 17, 37, 62, 112, 262]

# 5D software wall clock, ns, from perf SUMMARY lines (mean, max fields)
SW5D_MEAN_NS = [16.13, 16.62, 26.30, 44.58, 72.69, 138.19]
SW5D_MAX_NS = [298814, 1913, 117854, 27275, 355410, 17230]
# hardware wall clock, ns: L(n) cycles at 10 ns, bound = mean = max
HW_NS = [c * 10 for c in HW]


def base_axes(figsize):
    fig, ax = plt.subplots(figsize=figsize)
    ax.grid(True, color=GRID, zorder=0)
    ax.set_axisbelow(True)
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)
    ax.tick_params(length=2, width=0.6)
    return fig, ax


def save(fig, stem):
    fig.savefig(os.path.join(OUT_PDF, stem + ".pdf"))
    fig.savefig(os.path.join(OUT_PNG, stem + "-preview.png"), dpi=200)
    plt.close(fig)


def label_end(ax, x, y, text, color, dy=0):
    ax.annotate(text, xy=(x, y), xytext=(4, dy), textcoords="offset points",
                color=color, fontsize=7.5, va="center", ha="left")


# ---------------------------------------------------------------- key finding
fig, ax = base_axes((3.5, 2.5))
series = [
    ("SW 3D", SW3D, S1, "o", (0, 2)),
    ("SW 4D", SW4D, S2, "s", (0, 0)),
    ("SW 5D", SW5D, S3, "^", (0, 2)),
]
for name, ys, color, marker, _ in series:
    ax.plot(RULES, ys, color=color, marker=marker, markersize=3, zorder=3)
ax.plot(RULES, HW, color=INK, marker="D", markersize=3, zorder=3)

label_end(ax, 500, SW5D[-1], "SW 5D", S3)
label_end(ax, 500, SW4D[-1], "SW 4D", S2, dy=-3)
label_end(ax, 500, SW3D[-1], "SW 3D", S1)
label_end(ax, 500, HW[-1], "HW 4D = 5D", INK)

ax.set_xlabel("Policy size (rules)")
ax.set_ylabel("Cycles per decision")
ax.set_xticks([0, 100, 200, 300, 400, 500])
ax.set_xlim(0, 640)  # right margin carries the direct labels
ax.set_ylim(0, 740)
fig.tight_layout(pad=0.3)
save(fig, "fig-key-finding")

# ----------------------------------------------------------------------- tail
fig, ax = base_axes((3.5, 2.5))
ax.set_yscale("log")

sw_max_us = [v / 1000 for v in SW5D_MAX_NS]
sw_mean_us = [v / 1000 for v in SW5D_MEAN_NS]
hw_us = [v / 1000 for v in HW_NS]

ax.plot(RULES, sw_max_us, color=S2, marker="^", markersize=3.5,
        linestyle=":", zorder=3)
ax.plot(RULES, hw_us, color=INK, marker="D", markersize=3, zorder=3)
ax.plot(RULES, sw_mean_us, color=S1, marker="o", markersize=3,
        linestyle="--", zorder=3)

label_end(ax, 500, sw_max_us[-1], "SW worst observed", S2)
label_end(ax, 500, hw_us[-1], "HW bound", INK, dy=3)
label_end(ax, 500, sw_mean_us[-1], "SW mean", S1, dy=-2)

ax.set_xlabel("Policy size (rules)")
ax.set_ylabel(r"Latency per decision ($\mu$s)")
ax.set_xticks([0, 100, 200, 300, 400, 500])
ax.set_xlim(0, 700)
ax.set_ylim(0.008, 2000)
fig.tight_layout(pad=0.3)
save(fig, "fig-tail")

print(f"wrote fig-key-finding.pdf and fig-tail.pdf to {os.path.abspath(OUT_PDF)}")
print(f"wrote previews to {os.path.abspath(OUT_PNG)}")
