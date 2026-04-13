set script_dir [file dirname [file normalize [info script]]]
set hls_root [file normalize [file join $script_dir ..]]

if {![info exists ::env(HNGAC_HLS_PART)]} {
    puts "Set HNGAC_HLS_PART to the installed VU9P part name before running this script."
    exit 1
}

set clock_ns 5.0
if {[info exists ::env(HNGAC_HLS_CLOCK_NS)]} {
    set clock_ns $::env(HNGAC_HLS_CLOCK_NS)
}

open_project -reset [file join $hls_root work hngac_authorize]
set_top hngac_authorize

add_files [file join $hls_root src hngac_kernel.cpp] -cflags "-I[file join $hls_root include]"
add_files -tb [file join $hls_root tb hngac_kernel_tb.cpp] -cflags "-I[file join $hls_root include]"

open_solution -reset sol1
set_part $::env(HNGAC_HLS_PART)
create_clock -period $clock_ns -name default

csim_design
csynth_design

puts "HLS synthesis completed. Uncomment cosim/export steps after the interface is finalized."
exit
