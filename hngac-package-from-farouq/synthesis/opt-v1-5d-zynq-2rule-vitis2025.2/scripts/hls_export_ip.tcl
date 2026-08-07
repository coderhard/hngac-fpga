# Export IP for Zynq-7020 (opt-v1 5D padded)
open_project -reset hngac_opt_v1_5d_export
set_top hngac_authorize
add_files src/hngac_kernel.cpp -cflags "-Iinclude"
open_solution -reset solution1
set_part {xc7z020clg400-1}
create_clock -period 10 -name default
csynth_design
export_design -format ip_catalog -vendor "user.org" -library "hls" -version "3.0" -display_name "HNGAC Authorize Opt-v1 5D"
exit
