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

Color encoding. Blue family = software, orange accent = hardware, held
constant across both figures so a hue always names the same entity. The
three software series in the key-finding figure are an ordered tier
(3D -> 4D -> 5D), so they take one hue in monotone lightness steps rather
than three unrelated hues: the reader sees the dimensional order in the
color, and the ramp survives grayscale printing. Validated with the
dataviz skill's checker against a white print surface:

  validate_palette.js "#5598e7,#256abf,#104281" --ordinal --surface "#ffffff"
      -> monotone L, adjacent dL >= 0.06, light end 2.99:1, hue spread 3 deg
  validate_palette.js "#5598e7,#eb6834"          --surface "#ffffff"
      -> CVD dE 24.7 (protan), normal-vision dE 29.9
  validate_palette.js "#256abf,#eb6834,#5598e7"  --surface "#ffffff"
      -> CVD dE 23.8 (protan), normal-vision dE 29.9

The lightest blue sits at 2.99:1 on white, just under the 3:1 mark floor.
The relief is the direct labels on every series plus Tables I-III, which
carry the same numbers. Series text is set in ink, never in the series
color: colored 7.5 pt type would not clear the text-contrast floor, and
the line end beside the label already carries the hue.
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
    "lines.linewidth": 1.0,
})

INK = "#0b0b0b"        # primary ink, all series labels
SECONDARY = "#52514e"  # tick labels
GRID = "#d5d4ce"       # solid hairline, one shade off the page
AXIS = "#898781"

# software = blue, ordered light -> dark by dimension count
SW_3D, SW_4D, SW_5D = "#5598e7", "#256abf", "#104281"
# hardware = the orange accent, the one series the figures are about
HW_C = "#eb6834"
# every series also carries a distinct marker and line style, so identity
# survives grayscale print and full color-vision deficiency
LW_CTX, LW_ACCENT = 1.0, 1.6
# surface ring so overlapping markers stay separable where the series converge
RING = dict(markeredgecolor="#ffffff", markeredgewidth=0.5)

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
    ax.grid(True, color=GRID, linestyle="-", zorder=0)  # solid hairline, never dashed
    ax.set_axisbelow(True)
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)
    for s in ("left", "bottom"):
        ax.spines[s].set_color(AXIS)
    ax.tick_params(length=2, width=0.6, color=AXIS, labelcolor=SECONDARY)
    return fig, ax


def save(fig, stem):
    fig.savefig(os.path.join(OUT_PDF, stem + ".pdf"))
    fig.savefig(os.path.join(OUT_PNG, stem + "-preview.png"), dpi=200)
    plt.close(fig)


def label_end(ax, x, y, text, dy=0, weight="normal"):
    """Direct label at a line end, always in ink; the mark carries the hue."""
    ax.annotate(text, xy=(x, y), xytext=(5, dy), textcoords="offset points",
                color=INK, fontsize=7.5, va="center", ha="left", weight=weight)


# ---------------------------------------------------------------- key finding
fig, ax = base_axes((3.5, 2.5))
for ys, color, marker in ((SW3D, SW_3D, "o"),
                          (SW4D, SW_4D, "s"),
                          (SW5D, SW_5D, "^")):
    ax.plot(RULES, ys, color=color, marker=marker, markersize=3.4,
            linewidth=LW_CTX, zorder=3, **RING)
ax.plot(RULES, HW, color=HW_C, marker="D", markersize=3.6,
        linewidth=LW_ACCENT, zorder=4, **RING)

label_end(ax, 500, SW5D[-1], "SW 5D")
label_end(ax, 500, SW4D[-1], "SW 4D", dy=-3)
label_end(ax, 500, SW3D[-1], "SW 3D")
label_end(ax, 500, HW[-1], "HW 4D = 5D", weight="bold")

ax.set_xlabel("Policy size (rules)", color=INK)
ax.set_ylabel("Cycles per decision", color=INK)
ax.set_xticks([0, 100, 200, 300, 400, 500])
ax.set_xlim(0, 690)  # right margin carries the direct labels
ax.set_ylim(0, 740)
fig.tight_layout(pad=0.3)
save(fig, "fig-key-finding")

# ----------------------------------------------------------------------- tail
fig, ax = base_axes((3.5, 2.5))
ax.set_yscale("log")

sw_max_us = [v / 1000 for v in SW5D_MAX_NS]
sw_mean_us = [v / 1000 for v in SW5D_MEAN_NS]
hw_us = [v / 1000 for v in HW_NS]

# two statistics of one software path: one hue, two shades, darker = larger
ax.plot(RULES, sw_max_us, color=SW_4D, marker="^", markersize=3.6,
        linestyle=":", linewidth=LW_CTX, zorder=3, **RING)
ax.plot(RULES, hw_us, color=HW_C, marker="D", markersize=3.6,
        linewidth=LW_ACCENT, zorder=4, **RING)
ax.plot(RULES, sw_mean_us, color=SW_3D, marker="o", markersize=3.4,
        linestyle="--", linewidth=LW_CTX, zorder=3, **RING)

label_end(ax, 500, sw_max_us[-1], "SW worst observed")
label_end(ax, 500, hw_us[-1], "HW bound", dy=3, weight="bold")
label_end(ax, 500, sw_mean_us[-1], "SW mean", dy=-2)

ax.set_xlabel("Policy size (rules)", color=INK)
ax.set_ylabel(r"Latency per decision ($\mu$s)", color=INK)
ax.set_xticks([0, 100, 200, 300, 400, 500])
ax.set_xlim(0, 830)
ax.set_ylim(0.008, 2000)
fig.tight_layout(pad=0.3)
save(fig, "fig-tail")

print(f"wrote fig-key-finding.pdf and fig-tail.pdf to {os.path.abspath(OUT_PDF)}")
print(f"wrote previews to {os.path.abspath(OUT_PNG)}")
