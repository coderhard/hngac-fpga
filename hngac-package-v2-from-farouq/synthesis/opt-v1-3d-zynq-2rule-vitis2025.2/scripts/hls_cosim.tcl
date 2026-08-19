open_project -reset hngac_opt_v1_3d_cosim
set_top hngac_authorize
add_files src/hngac_kernel.cpp -cflags "-Iinclude"
add_files -tb tb/hngac_kernel_tb.cpp -cflags "-Iinclude"
open_solution -reset solution1
set_part {xc7z020clg400-1}
create_clock -period 10 -name default
csim_design
csynth_design
cosim_design -rtl verilog
exit
