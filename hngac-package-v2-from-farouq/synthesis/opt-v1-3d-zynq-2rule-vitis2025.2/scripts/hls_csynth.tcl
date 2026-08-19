open_project -reset hngac_opt_v1_3d
set_top hngac_authorize
add_files src/hngac_kernel.cpp -cflags "-Iinclude"
open_solution -reset solution1
set_part {xc7z020clg400-1}
create_clock -period 10 -name default
csynth_design
exit
