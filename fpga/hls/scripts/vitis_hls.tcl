set script_dir [file dirname [file normalize [info script]]]
set hls_root   [file normalize [file join $script_dir ..]]

# ---------------------------------------------------------------------------
# Required: target part
# ---------------------------------------------------------------------------
if {![info exists ::env(HNGAC_HLS_PART)]} {
    puts "ERROR: Set HNGAC_HLS_PART before running this script."
    puts "  Example: export HNGAC_HLS_PART=xcu250-figd2104-2L-e"
    exit 1
}

# ---------------------------------------------------------------------------
# Optional env-var overrides
# ---------------------------------------------------------------------------
set clock_ns 5.0
if {[info exists ::env(HNGAC_HLS_CLOCK_NS)]} {
    set clock_ns $::env(HNGAC_HLS_CLOCK_NS)
}

set work_root "/tmp/hngac-fpga-hls"
if {[info exists ::env(HNGAC_HLS_WORKDIR)]} {
    set work_root $::env(HNGAC_HLS_WORKDIR)
}

# Set HNGAC_HLS_COSIM=1 to run co-simulation after C synthesis.
# Co-sim is slow; leave off for iterative pragma tuning.
set run_cosim 0
if {[info exists ::env(HNGAC_HLS_COSIM)] && $::env(HNGAC_HLS_COSIM) eq "1"} {
    set run_cosim 1
}

# Set HNGAC_HLS_EXPORT=1 to export the synthesised IP at the end.
set run_export 0
if {[info exists ::env(HNGAC_HLS_EXPORT)] && $::env(HNGAC_HLS_EXPORT) eq "1"} {
    set run_export 1
}

# ---------------------------------------------------------------------------
# Project setup
# ---------------------------------------------------------------------------
set project_dir [file normalize [file join $work_root hngac_authorize]]
file mkdir $work_root

puts "====================================================="
puts "  hngac-fpga Vitis HLS synthesis"
puts "====================================================="
puts "  Part:           $::env(HNGAC_HLS_PART)"
puts "  Clock:          ${clock_ns} ns  ([expr {int(1000.0/$clock_ns)}] MHz target)"
puts "  Work dir:       $project_dir"
puts "  Co-simulation:  $run_cosim"
puts "  Export IP:      $run_export"
puts "====================================================="

open_project -reset $project_dir

# Top function lives inside namespace hngac::fpga — must use qualified name.
set_top hngac::fpga::hngac_authorize

set cflags "-std=c++17 -I[file join $hls_root include]"
add_files      [file join $hls_root src hngac_kernel.cpp]    -cflags $cflags
add_files -tb  [file join $hls_root tb hngac_kernel_tb.cpp]  -cflags $cflags

open_solution -reset sol1
set_part $::env(HNGAC_HLS_PART)
create_clock -period $clock_ns -name default

# ---------------------------------------------------------------------------
# C simulation — runs the 34-case testbench through the HLS compiler
# ---------------------------------------------------------------------------
csim_design

# ---------------------------------------------------------------------------
# C synthesis — generates RTL and timing/resource estimates
# ---------------------------------------------------------------------------
csynth_design

# ---------------------------------------------------------------------------
# Co-simulation (optional — gate with HNGAC_HLS_COSIM=1)
# ---------------------------------------------------------------------------
if {$run_cosim} {
    puts "Running co-simulation..."
    cosim_design
}

# ---------------------------------------------------------------------------
# IP export (optional — gate with HNGAC_HLS_EXPORT=1)
# ---------------------------------------------------------------------------
if {$run_export} {
    puts "Exporting IP catalog..."
    export_design -format ip_catalog
}

set report_dir [file join $project_dir sol1 syn report]
puts ""
puts "Synthesis complete."
puts "Reports: $report_dir"
puts "  hngac_authorize_csynth.rpt  — latency and resource summary"
puts "  hngac_authorize_utilization_routed.rpt  — post-route (if cosim ran)"
exit
