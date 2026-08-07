# =============================================================================
# build_zybo_test.tcl — Build HNGAC + AXI Timer block design for Zybo
#
# Usage:
#   vivado -mode batch -source build_zybo_test.tcl
#
# Outputs (copy these two onto the PYNQ SD card next to test_method1_pynq.py):
#   ./build/hngac_test.bit
#   ./build/hngac_test.hwh
# =============================================================================

# ---- User-configurable settings -------------------------------------------
set base_dir    "C:/Users/write/test/vivado_test/board-test/opt-v1-5d-pynqz1-vitis2025.2"
set proj_name   "hngac_test"
set proj_dir    [file join $base_dir "vivado-proj"]
set ip_repo     [file join $base_dir "ip/hngac_authorize_opt_v1_5d"]
set output_dir  [file join $base_dir "bitstream"]
set script_dir  [file join $base_dir "vivado-scripts"]

set board_part "www.digilentinc.com:pynq-z1:part0:1.0"

# Set to 0 to only create the project (skip ~10-min bitstream build)
set run_bitstream 1
# ---------------------------------------------------------------------------

puts "DEBUG script_dir=$script_dir"
puts "DEBUG proj_dir=$proj_dir"
puts "DEBUG ip_repo=$ip_repo"
puts "DEBUG output_dir=$output_dir"
file mkdir $output_dir

# ---- Create project --------------------------------------------------------
create_project $proj_name $proj_dir -force
set_property board_part $board_part [current_project]
# Disable the IP cache. Successive HLS exports keep the same module name
# (hngac_authorize) across versions, so a cached out-of-context netlist from an
# earlier version could in principle be reused, giving a bitstream whose .hwh
# advertises the new register map while the fabric holds the old logic.
# Kept as hygiene for a repo that re-exports IP under a stable module name.
# Note: this was NOT the cause of the hw_cycle_count=0 symptom -- that was a
# stale bitstream loaded by relative path, and the denies were the BRAM address
# slice below.
config_ip_cache -disable_cache
set_property ip_repo_paths $ip_repo [current_project]
update_ip_catalog

# ---- Create block design ---------------------------------------------------
create_bd_design "system"

# 1. Zynq PS with board preset, then override FCLK_CLK0 to 100 MHz
#    (Zybo Z7-20 preset defaults to 50 MHz; the design meets timing at 100 MHz)
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7 processing_system7_0
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 \
    -config { make_external "FIXED_IO, DDR" apply_board_preset "1" \
              Master "Disable" Slave "Disable" } \
    [get_bd_cells processing_system7_0]
set_property -dict [list CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {100}] \
    [get_bd_cells processing_system7_0]

# 2. HNGAC authorize IP
create_bd_cell -type ip -vlnv user.org:hls:hngac_authorize:3.0 hngac_authorize_0

# 3. AXI Timer
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_timer axi_timer_0

# Hardware-triggered capture on the *unmodified* kernel. The HLS IP asserts
# `interrupt` on ap_done; that latches the free-running counter into TLR0 at the
# completion clock, with no PS polling inside the measurement window. This is
# how the shipped opt-v1 kernel gets a board-measured latency without carrying
# an instrumentation counter in the design. Requires GIER+IP_IER enabled on the
# IP so interrupt fires, and ARHT clear so a later trigger cannot overwrite the
# captured value.
connect_bd_net [get_bd_pins hngac_authorize_0/interrupt] \
               [get_bd_pins axi_timer_0/capturetrig0]

# 4. Connection Automation — auto-builds AXI Interconnect + reset + clocking
apply_bd_automation -rule xilinx.com:bd_rule:axi4 \
    -config { Master "/processing_system7_0/M_AXI_GP0" Clk "Auto" } \
    [get_bd_intf_pins hngac_authorize_0/s_axi_control]

apply_bd_automation -rule xilinx.com:bd_rule:axi4 \
    -config { Master "/processing_system7_0/M_AXI_GP0" Clk "Auto" } \
    [get_bd_intf_pins axi_timer_0/S_AXI]

# 5. Policy BRAM — wire hngac_authorize_0/policy_PORTA to a BRAM, with a
#    second port exposed to PS via AXI BRAM Controller so rules can be loaded
#    from Python at runtime.

# 5a. Two BMGs with manual pin wiring (proven approach from 2019.1 build).
# BMG0: PortA(1024-bit) -> IP PORTA, PortB(32-bit) -> AXI BRAM Ctrl 0 (PS writes)
# BMG1: PortA(1024-bit) -> IP PORTB, PortB(32-bit) -> AXI BRAM Ctrl 1 (PS writes)

create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen blk_mem_gen_0
set_property -dict [list \
    CONFIG.Memory_Type {True_Dual_Port_RAM} \
    CONFIG.use_bram_block {Stand_Alone} \
    CONFIG.Assume_Synchronous_Clk {true} \
    CONFIG.Write_Width_A {1024} CONFIG.Write_Depth_A {512} CONFIG.Read_Width_A {1024} \
    CONFIG.Operating_Mode_A {WRITE_FIRST} \
    CONFIG.Enable_A {Use_ENA_Pin} CONFIG.Use_RSTA_Pin {true} \
    CONFIG.Register_PortA_Output_of_Memory_Primitives {false} \
    CONFIG.Write_Width_B {32} CONFIG.Read_Width_B {32} \
    CONFIG.Operating_Mode_B {WRITE_FIRST} \
    CONFIG.Enable_B {Use_ENB_Pin} CONFIG.Use_RSTB_Pin {false} \
    CONFIG.Register_PortB_Output_of_Memory_Primitives {false} \
    CONFIG.Port_B_Clock {100} CONFIG.Port_B_Write_Rate {50} CONFIG.Port_B_Enable_Rate {100} \
] [get_bd_cells blk_mem_gen_0]

create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen blk_mem_gen_1
set_property -dict [list \
    CONFIG.Memory_Type {True_Dual_Port_RAM} \
    CONFIG.use_bram_block {Stand_Alone} \
    CONFIG.Assume_Synchronous_Clk {true} \
    CONFIG.Write_Width_A {1024} CONFIG.Write_Depth_A {512} CONFIG.Read_Width_A {1024} \
    CONFIG.Operating_Mode_A {WRITE_FIRST} \
    CONFIG.Enable_A {Use_ENA_Pin} CONFIG.Use_RSTA_Pin {true} \
    CONFIG.Register_PortA_Output_of_Memory_Primitives {false} \
    CONFIG.Write_Width_B {32} CONFIG.Read_Width_B {32} \
    CONFIG.Operating_Mode_B {WRITE_FIRST} \
    CONFIG.Enable_B {Use_ENB_Pin} CONFIG.Use_RSTB_Pin {false} \
    CONFIG.Register_PortB_Output_of_Memory_Primitives {false} \
    CONFIG.Port_B_Clock {100} CONFIG.Port_B_Write_Rate {50} CONFIG.Port_B_Enable_Rate {100} \
] [get_bd_cells blk_mem_gen_1]

# 5b. AXI BRAM Controllers
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl axi_bram_ctrl_0
set_property -dict [list CONFIG.SINGLE_PORT_BRAM {1}] [get_bd_cells axi_bram_ctrl_0]
apply_bd_automation -rule xilinx.com:bd_rule:axi4 \
    -config { Master "/processing_system7_0/M_AXI_GP0" Clk "Auto" } \
    [get_bd_intf_pins axi_bram_ctrl_0/S_AXI]
set_property range 64K \
    [get_bd_addr_segs {processing_system7_0/Data/SEG_axi_bram_ctrl_0_Mem0}]

create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl axi_bram_ctrl_1
set_property -dict [list CONFIG.SINGLE_PORT_BRAM {1}] [get_bd_cells axi_bram_ctrl_1]
apply_bd_automation -rule xilinx.com:bd_rule:axi4 \
    -config { Master "/processing_system7_0/M_AXI_GP0" Clk "Auto" } \
    [get_bd_intf_pins axi_bram_ctrl_1/S_AXI]
set_property range 64K \
    [get_bd_addr_segs {processing_system7_0/Data/SEG_axi_bram_ctrl_1_Mem0}]

# 5c. Wire BMG0 PortB to AXI BRAM Ctrl 0, hand-wired through an address slice.
# The controller drives a 16-bit BYTE address; the BMG's 32-bit-wide Port B
# takes a 14-bit WORD address. connect_bd_intf_net cannot bridge that: it wires
# bram_addr_a(16) straight to addrb(14) as "lower order bits only", dropping the
# divide-by-4. PS reads still look correct because writes and reads share the
# same wrong mapping, but the IP reads Port A at the true rule stride and sees
# zeros -- every request denied. The [15:2] slice is what makes them agree.
# bram_we_a(4) -> web(1) truncation is safe here: the PS only ever issues full
# 32-bit word writes, so all four byte enables assert together.
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice portb0_addr_slice
set_property -dict [list CONFIG.DIN_WIDTH {16} CONFIG.DIN_FROM {15} CONFIG.DIN_TO {2} CONFIG.DOUT_WIDTH {14}] [get_bd_cells portb0_addr_slice]
connect_bd_net [get_bd_pins axi_bram_ctrl_0/bram_clk_a]    [get_bd_pins blk_mem_gen_0/clkb]
connect_bd_net [get_bd_pins axi_bram_ctrl_0/bram_en_a]     [get_bd_pins blk_mem_gen_0/enb]
connect_bd_net [get_bd_pins axi_bram_ctrl_0/bram_we_a]     [get_bd_pins blk_mem_gen_0/web]
connect_bd_net [get_bd_pins axi_bram_ctrl_0/bram_addr_a]   [get_bd_pins portb0_addr_slice/Din]
connect_bd_net [get_bd_pins portb0_addr_slice/Dout]        [get_bd_pins blk_mem_gen_0/addrb]
connect_bd_net [get_bd_pins axi_bram_ctrl_0/bram_wrdata_a] [get_bd_pins blk_mem_gen_0/dinb]
connect_bd_net [get_bd_pins blk_mem_gen_0/doutb]           [get_bd_pins axi_bram_ctrl_0/bram_rddata_a]

# 5d. Wire BMG0 PortA to IP PORTA (IP reads, 1024-bit)
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice porta0_addr_slice
set_property -dict [list CONFIG.DIN_WIDTH {32} CONFIG.DIN_FROM {15} CONFIG.DIN_TO {7} CONFIG.DOUT_WIDTH {9}] [get_bd_cells porta0_addr_slice]
connect_bd_net [get_bd_pins hngac_authorize_0/policy_Clk_A]  [get_bd_pins blk_mem_gen_0/clka]
connect_bd_net [get_bd_pins hngac_authorize_0/policy_EN_A]   [get_bd_pins blk_mem_gen_0/ena]
connect_bd_net [get_bd_pins hngac_authorize_0/policy_Addr_A] [get_bd_pins porta0_addr_slice/Din]
connect_bd_net [get_bd_pins porta0_addr_slice/Dout]          [get_bd_pins blk_mem_gen_0/addra]
# Tie wea to 0 (IP only reads, never writes -- prevents WEN width mismatch from corrupting data)
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant const_wea_0
set_property -dict [list CONFIG.CONST_WIDTH {1} CONFIG.CONST_VAL {0}] [get_bd_cells const_wea_0]
connect_bd_net [get_bd_pins const_wea_0/dout] [get_bd_pins blk_mem_gen_0/wea]
# Din_A left unconnected (no writes through Port A)
connect_bd_net [get_bd_pins blk_mem_gen_0/douta]             [get_bd_pins hngac_authorize_0/policy_Dout_A]
connect_bd_net [get_bd_pins hngac_authorize_0/policy_Rst_A]  [get_bd_pins blk_mem_gen_0/rsta]

# 5e. Wire BMG1 PortB to AXI BRAM Ctrl 1, same address slice as 5c.
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice portb1_addr_slice
set_property -dict [list CONFIG.DIN_WIDTH {16} CONFIG.DIN_FROM {15} CONFIG.DIN_TO {2} CONFIG.DOUT_WIDTH {14}] [get_bd_cells portb1_addr_slice]
connect_bd_net [get_bd_pins axi_bram_ctrl_1/bram_clk_a]    [get_bd_pins blk_mem_gen_1/clkb]
connect_bd_net [get_bd_pins axi_bram_ctrl_1/bram_en_a]     [get_bd_pins blk_mem_gen_1/enb]
connect_bd_net [get_bd_pins axi_bram_ctrl_1/bram_we_a]     [get_bd_pins blk_mem_gen_1/web]
connect_bd_net [get_bd_pins axi_bram_ctrl_1/bram_addr_a]   [get_bd_pins portb1_addr_slice/Din]
connect_bd_net [get_bd_pins portb1_addr_slice/Dout]        [get_bd_pins blk_mem_gen_1/addrb]
connect_bd_net [get_bd_pins axi_bram_ctrl_1/bram_wrdata_a] [get_bd_pins blk_mem_gen_1/dinb]
connect_bd_net [get_bd_pins blk_mem_gen_1/doutb]           [get_bd_pins axi_bram_ctrl_1/bram_rddata_a]

# 5f. Wire BMG1 PortA to IP PORTB (IP reads, 1024-bit)
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice porta1_addr_slice
set_property -dict [list CONFIG.DIN_WIDTH {32} CONFIG.DIN_FROM {15} CONFIG.DIN_TO {7} CONFIG.DOUT_WIDTH {9}] [get_bd_cells porta1_addr_slice]
connect_bd_net [get_bd_pins hngac_authorize_0/policy_Clk_B]  [get_bd_pins blk_mem_gen_1/clka]
connect_bd_net [get_bd_pins hngac_authorize_0/policy_EN_B]   [get_bd_pins blk_mem_gen_1/ena]
connect_bd_net [get_bd_pins hngac_authorize_0/policy_Addr_B] [get_bd_pins porta1_addr_slice/Din]
connect_bd_net [get_bd_pins porta1_addr_slice/Dout]          [get_bd_pins blk_mem_gen_1/addra]
# Tie wea to 0 for BMG1 as well
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant const_wea_1
set_property -dict [list CONFIG.CONST_WIDTH {1} CONFIG.CONST_VAL {0}] [get_bd_cells const_wea_1]
connect_bd_net [get_bd_pins const_wea_1/dout] [get_bd_pins blk_mem_gen_1/wea]
connect_bd_net [get_bd_pins blk_mem_gen_1/douta]             [get_bd_pins hngac_authorize_0/policy_Dout_B]
connect_bd_net [get_bd_pins hngac_authorize_0/policy_Rst_B]  [get_bd_pins blk_mem_gen_1/rsta]

# ---- Debug: print port widths -----------------------------------------------
puts "DEBUG BMG0 addra width: [get_property LEFT [get_bd_pins blk_mem_gen_0/addra]]:[get_property RIGHT [get_bd_pins blk_mem_gen_0/addra]]"
puts "DEBUG BMG0 douta width: [get_property LEFT [get_bd_pins blk_mem_gen_0/douta]]:[get_property RIGHT [get_bd_pins blk_mem_gen_0/douta]]"
puts "DEBUG IP Addr_A width: [get_property LEFT [get_bd_pins hngac_authorize_0/policy_Addr_A]]:[get_property RIGHT [get_bd_pins hngac_authorize_0/policy_Addr_A]]"
puts "DEBUG IP Dout_A width: [get_property LEFT [get_bd_pins hngac_authorize_0/policy_Dout_A]]:[get_property RIGHT [get_bd_pins hngac_authorize_0/policy_Dout_A]]"
puts "DEBUG slicer output width: [get_property LEFT [get_bd_pins porta0_addr_slice/Dout]]:[get_property RIGHT [get_bd_pins porta0_addr_slice/Dout]]"

# ---- Finalize BD -----------------------------------------------------------
regenerate_bd_layout
validate_bd_design
save_bd_design

# Wrapper
make_wrapper -files [get_files $proj_dir/$proj_name.srcs/sources_1/bd/system/system.bd] -top
add_files -norecurse $proj_dir/$proj_name.srcs/sources_1/bd/system/hdl/system_wrapper.v
set_property top system_wrapper [current_fileset]
update_compile_order -fileset sources_1

puts ""
puts "=========================================="
puts " Block design created: $proj_dir/$proj_name.xpr"
puts "=========================================="

# ---- Bitstream (optional) --------------------------------------------------
if {$run_bitstream} {
    puts ""
    puts ">>> Launching synthesis + implementation + bitstream..."
    launch_runs synth_1 -jobs 4
    wait_on_run synth_1
    launch_runs impl_1 -to_step write_bitstream -jobs 4
    wait_on_run impl_1

    # Copy outputs for PYNQ
    set bit_src "$proj_dir/$proj_name.runs/impl_1/system_wrapper.bit"
    set hwh_src "$proj_dir/$proj_name.gen/sources_1/bd/system/hw_handoff/system.hwh"

    file copy -force $bit_src "$output_dir/hngac_test.bit"
    file copy -force $hwh_src "$output_dir/hngac_test.hwh"

    puts ""
    puts "=========================================="
    puts " Done. Files for PYNQ:"
    puts "   $output_dir/hngac_test.bit"
    puts "   $output_dir/hngac_test.hwh"
    puts " Copy both onto the Zybo SD card next to"
    puts " test_method1_pynq.py and run it there."
    puts "=========================================="
}
