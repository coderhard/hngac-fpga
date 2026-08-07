// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
`timescale 1ns/1ps
(* DowngradeIPIdentifiedWarnings="yes" *) module hngac_authorize_control_s_axi
#(parameter
    C_S_AXI_ADDR_WIDTH = 7,
    C_S_AXI_DATA_WIDTH = 32
)(
    input  wire                          ACLK,
    input  wire                          ARESET,
    input  wire                          ACLK_EN,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0] AWADDR,
    input  wire                          AWVALID,
    output wire                          AWREADY,
    input  wire [C_S_AXI_DATA_WIDTH-1:0] WDATA,
    input  wire [C_S_AXI_DATA_WIDTH/8-1:0] WSTRB,
    input  wire                          WVALID,
    output wire                          WREADY,
    output wire [1:0]                    BRESP,
    output wire                          BVALID,
    input  wire                          BREADY,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0] ARADDR,
    input  wire                          ARVALID,
    output wire                          ARREADY,
    output wire [C_S_AXI_DATA_WIDTH-1:0] RDATA,
    output wire [1:0]                    RRESP,
    output wire                          RVALID,
    input  wire                          RREADY,
    output wire                          interrupt,
    output wire                          ap_start,
    input  wire                          ap_done,
    input  wire                          ap_ready,
    input  wire                          ap_idle,
    input  wire [0:0]                    ap_return,
    output wire [15:0]                   rule_count,
    output wire [383:0]                  request
);
//------------------------Address Info-------------------
// Protocol Used: ap_ctrl_hs
//
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of ap_return
//        bit 0  - ap_return[0] (Read)
//        others - reserved
// 0x18 : Data signal of rule_count
//        bit 15~0 - rule_count[15:0] (Read/Write)
//        others   - reserved
// 0x1c : reserved
// 0x40 : Data signal of request
//        bit 31~0 - request[31:0] (Read/Write)
// 0x44 : Data signal of request
//        bit 31~0 - request[63:32] (Read/Write)
// 0x48 : Data signal of request
//        bit 31~0 - request[95:64] (Read/Write)
// 0x4c : Data signal of request
//        bit 31~0 - request[127:96] (Read/Write)
// 0x50 : Data signal of request
//        bit 31~0 - request[159:128] (Read/Write)
// 0x54 : Data signal of request
//        bit 31~0 - request[191:160] (Read/Write)
// 0x58 : Data signal of request
//        bit 31~0 - request[223:192] (Read/Write)
// 0x5c : Data signal of request
//        bit 31~0 - request[255:224] (Read/Write)
// 0x60 : Data signal of request
//        bit 31~0 - request[287:256] (Read/Write)
// 0x64 : Data signal of request
//        bit 31~0 - request[319:288] (Read/Write)
// 0x68 : Data signal of request
//        bit 31~0 - request[351:320] (Read/Write)
// 0x6c : Data signal of request
//        bit 31~0 - request[383:352] (Read/Write)
// 0x70 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

//------------------------Parameter----------------------
localparam
    ADDR_AP_CTRL           = 7'h00,
    ADDR_GIE               = 7'h04,
    ADDR_IER               = 7'h08,
    ADDR_ISR               = 7'h0c,
    ADDR_AP_RETURN_0       = 7'h10,
    ADDR_RULE_COUNT_DATA_0 = 7'h18,
    ADDR_RULE_COUNT_CTRL   = 7'h1c,
    ADDR_REQUEST_DATA_0    = 7'h40,
    ADDR_REQUEST_DATA_1    = 7'h44,
    ADDR_REQUEST_DATA_2    = 7'h48,
    ADDR_REQUEST_DATA_3    = 7'h4c,
    ADDR_REQUEST_DATA_4    = 7'h50,
    ADDR_REQUEST_DATA_5    = 7'h54,
    ADDR_REQUEST_DATA_6    = 7'h58,
    ADDR_REQUEST_DATA_7    = 7'h5c,
    ADDR_REQUEST_DATA_8    = 7'h60,
    ADDR_REQUEST_DATA_9    = 7'h64,
    ADDR_REQUEST_DATA_10   = 7'h68,
    ADDR_REQUEST_DATA_11   = 7'h6c,
    ADDR_REQUEST_CTRL      = 7'h70,
    WRIDLE                 = 2'd0,
    WRDATA                 = 2'd1,
    WRRESP                 = 2'd2,
    WRRESET                = 2'd3,
    RDIDLE                 = 2'd0,
    RDDATA                 = 2'd1,
    RDRESET                = 2'd2,
    ADDR_BITS                = 7;

//------------------------Local signal-------------------
    reg  [1:0]                    wstate = WRRESET;
    reg  [1:0]                    wnext;
    reg  [ADDR_BITS-1:0]          waddr;
    wire [C_S_AXI_DATA_WIDTH-1:0] wmask;
    wire                          aw_hs;
    wire                          w_hs;
    reg  [1:0]                    rstate = RDRESET;
    reg  [1:0]                    rnext;
    reg  [C_S_AXI_DATA_WIDTH-1:0] rdata;
    wire                          ar_hs;
    wire [ADDR_BITS-1:0]          raddr;
    // internal registers
    reg                           int_ap_idle = 1'b0;
    reg                           int_ap_ready = 1'b0;
    wire                          task_ap_ready;
    reg                           int_ap_done = 1'b0;
    wire                          task_ap_done;
    reg                           int_task_ap_done = 1'b0;
    reg                           int_ap_start = 1'b0;
    reg                           int_interrupt = 1'b0;
    reg                           int_auto_restart = 1'b0;
    reg                           auto_restart_status = 1'b0;
    wire                          auto_restart_done;
    reg                           int_gie = 1'b0;
    reg  [1:0]                    int_ier = 2'b0;
    reg  [1:0]                    int_isr = 2'b0;
    reg  [0:0]                    int_ap_return;
    reg  [15:0]                   int_rule_count = 'b0;
    reg  [383:0]                  int_request = 'b0;

//------------------------Instantiation------------------


//------------------------AXI write fsm------------------
assign AWREADY = (wstate == WRIDLE);
assign WREADY  = (wstate == WRDATA);
assign BVALID  = (wstate == WRRESP);
assign BRESP   = 2'b00;  // OKAY
assign wmask   = { {8{WSTRB[3]}}, {8{WSTRB[2]}}, {8{WSTRB[1]}}, {8{WSTRB[0]}} };
assign aw_hs   = AWVALID & AWREADY;
assign w_hs    = WVALID & WREADY;

// wstate
always @(posedge ACLK) begin
    if (ARESET)
        wstate <= WRRESET;
    else if (ACLK_EN)
        wstate <= wnext;
end

// wnext
always @(*) begin
    case (wstate)
        WRIDLE:
            if (AWVALID)
                wnext = WRDATA;
            else
                wnext = WRIDLE;
        WRDATA:
            if (WVALID)
                wnext = WRRESP;
            else
                wnext = WRDATA;
        WRRESP:
            if (BREADY & BVALID)
                wnext = WRIDLE;
            else
                wnext = WRRESP;
        default:
            wnext = WRIDLE;
    endcase
end

// waddr
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (aw_hs)
            waddr <= {AWADDR[ADDR_BITS-1:2], {2{1'b0}}};
    end
end

//------------------------AXI read fsm-------------------
assign ARREADY = (rstate == RDIDLE);
assign RDATA   = rdata;
assign RRESP   = 2'b00;  // OKAY
assign RVALID  = (rstate == RDDATA);
assign ar_hs   = ARVALID & ARREADY;
assign raddr   = ARADDR[ADDR_BITS-1:0];

// rstate
always @(posedge ACLK) begin
    if (ARESET)
        rstate <= RDRESET;
    else if (ACLK_EN)
        rstate <= rnext;
end

// rnext
always @(*) begin
    case (rstate)
        RDIDLE:
            if (ARVALID)
                rnext = RDDATA;
            else
                rnext = RDIDLE;
        RDDATA:
            if (RREADY & RVALID)
                rnext = RDIDLE;
            else
                rnext = RDDATA;
        default:
            rnext = RDIDLE;
    endcase
end

// rdata
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (ar_hs) begin
            rdata <= 'b0;
            case (raddr)
                ADDR_AP_CTRL: begin
                    rdata[0] <= int_ap_start;
                    rdata[1] <= int_task_ap_done;
                    rdata[2] <= int_ap_idle;
                    rdata[3] <= int_ap_ready;
                    rdata[7] <= int_auto_restart;
                    rdata[9] <= int_interrupt;
                end
                ADDR_GIE: begin
                    rdata <= int_gie;
                end
                ADDR_IER: begin
                    rdata <= int_ier;
                end
                ADDR_ISR: begin
                    rdata <= int_isr;
                end
                ADDR_AP_RETURN_0: begin
                    rdata <= int_ap_return[0:0];
                end
                ADDR_RULE_COUNT_DATA_0: begin
                    rdata <= int_rule_count[15:0];
                end
                ADDR_REQUEST_DATA_0: begin
                    rdata <= int_request[31:0];
                end
                ADDR_REQUEST_DATA_1: begin
                    rdata <= int_request[63:32];
                end
                ADDR_REQUEST_DATA_2: begin
                    rdata <= int_request[95:64];
                end
                ADDR_REQUEST_DATA_3: begin
                    rdata <= int_request[127:96];
                end
                ADDR_REQUEST_DATA_4: begin
                    rdata <= int_request[159:128];
                end
                ADDR_REQUEST_DATA_5: begin
                    rdata <= int_request[191:160];
                end
                ADDR_REQUEST_DATA_6: begin
                    rdata <= int_request[223:192];
                end
                ADDR_REQUEST_DATA_7: begin
                    rdata <= int_request[255:224];
                end
                ADDR_REQUEST_DATA_8: begin
                    rdata <= int_request[287:256];
                end
                ADDR_REQUEST_DATA_9: begin
                    rdata <= int_request[319:288];
                end
                ADDR_REQUEST_DATA_10: begin
                    rdata <= int_request[351:320];
                end
                ADDR_REQUEST_DATA_11: begin
                    rdata <= int_request[383:352];
                end
            endcase
        end
    end
end


//------------------------Register logic-----------------
assign interrupt         = int_interrupt;
assign ap_start          = int_ap_start;
assign task_ap_done      = (ap_done && !auto_restart_status) || auto_restart_done;
assign task_ap_ready     = ap_ready && !int_auto_restart;
assign auto_restart_done = auto_restart_status && (ap_idle && !int_ap_idle);
assign rule_count        = int_rule_count;
assign request           = int_request;
// int_interrupt
always @(posedge ACLK) begin
    if (ARESET)
        int_interrupt <= 1'b0;
    else if (ACLK_EN) begin
        if (int_gie && (|int_isr))
            int_interrupt <= 1'b1;
        else
            int_interrupt <= 1'b0;
    end
end

// int_ap_start
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_start <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_AP_CTRL && WSTRB[0] && WDATA[0])
            int_ap_start <= 1'b1;
        else if (ap_ready)
            int_ap_start <= int_auto_restart; // clear on handshake/auto restart
    end
end

// int_ap_done
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_done <= 1'b0;
    else if (ACLK_EN) begin
            int_ap_done <= ap_done;
    end
end

// int_task_ap_done
always @(posedge ACLK) begin
    if (ARESET)
        int_task_ap_done <= 1'b0;
    else if (ACLK_EN) begin
        if (task_ap_done)
            int_task_ap_done <= 1'b1;
        else if (ar_hs && raddr == ADDR_AP_CTRL)
            int_task_ap_done <= 1'b0; // clear on read
    end
end

// int_ap_idle
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_idle <= 1'b0;
    else if (ACLK_EN) begin
            int_ap_idle <= ap_idle;
    end
end

// int_ap_ready
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_ready <= 1'b0;
    else if (ACLK_EN) begin
        if (task_ap_ready)
            int_ap_ready <= 1'b1;
        else if (ar_hs && raddr == ADDR_AP_CTRL)
            int_ap_ready <= 1'b0;
    end
end

// int_auto_restart
always @(posedge ACLK) begin
    if (ARESET)
        int_auto_restart <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_AP_CTRL && WSTRB[0])
            int_auto_restart <= WDATA[7];
    end
end

// auto_restart_status
always @(posedge ACLK) begin
    if (ARESET)
        auto_restart_status <= 1'b0;
    else if (ACLK_EN) begin
        if (int_auto_restart)
            auto_restart_status <= 1'b1;
        else if (ap_idle)
            auto_restart_status <= 1'b0;
    end
end

// int_gie
always @(posedge ACLK) begin
    if (ARESET)
        int_gie <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_GIE && WSTRB[0])
            int_gie <= WDATA[0];
    end
end

// int_ier
always @(posedge ACLK) begin
    if (ARESET)
        int_ier <= 1'b0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IER && WSTRB[0])
            int_ier <= WDATA[1:0];
    end
end

// int_isr[0]
always @(posedge ACLK) begin
    if (ARESET)
        int_isr[0] <= 1'b0;
    else if (ACLK_EN) begin
        if (int_ier[0] & ap_done)
            int_isr[0] <= 1'b1;
        else if (w_hs && waddr == ADDR_ISR && WSTRB[0])
            int_isr[0] <= int_isr[0] ^ WDATA[0]; // toggle on write
    end
end

// int_isr[1]
always @(posedge ACLK) begin
    if (ARESET)
        int_isr[1] <= 1'b0;
    else if (ACLK_EN) begin
        if (int_ier[1] & ap_ready)
            int_isr[1] <= 1'b1;
        else if (w_hs && waddr == ADDR_ISR && WSTRB[0])
            int_isr[1] <= int_isr[1] ^ WDATA[1]; // toggle on write
    end
end

// int_ap_return
always @(posedge ACLK) begin
    if (ARESET)
        int_ap_return <= 0;
    else if (ACLK_EN) begin
        if (ap_done)
            int_ap_return <= ap_return;
    end
end

// int_rule_count[15:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_rule_count[15:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_RULE_COUNT_DATA_0)
            int_rule_count[15:0] <= (WDATA[31:0] & wmask) | (int_rule_count[15:0] & ~wmask);
    end
end

// int_request[31:0]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[31:0] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_0)
            int_request[31:0] <= (WDATA[31:0] & wmask) | (int_request[31:0] & ~wmask);
    end
end

// int_request[63:32]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[63:32] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_1)
            int_request[63:32] <= (WDATA[31:0] & wmask) | (int_request[63:32] & ~wmask);
    end
end

// int_request[95:64]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[95:64] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_2)
            int_request[95:64] <= (WDATA[31:0] & wmask) | (int_request[95:64] & ~wmask);
    end
end

// int_request[127:96]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[127:96] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_3)
            int_request[127:96] <= (WDATA[31:0] & wmask) | (int_request[127:96] & ~wmask);
    end
end

// int_request[159:128]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[159:128] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_4)
            int_request[159:128] <= (WDATA[31:0] & wmask) | (int_request[159:128] & ~wmask);
    end
end

// int_request[191:160]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[191:160] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_5)
            int_request[191:160] <= (WDATA[31:0] & wmask) | (int_request[191:160] & ~wmask);
    end
end

// int_request[223:192]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[223:192] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_6)
            int_request[223:192] <= (WDATA[31:0] & wmask) | (int_request[223:192] & ~wmask);
    end
end

// int_request[255:224]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[255:224] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_7)
            int_request[255:224] <= (WDATA[31:0] & wmask) | (int_request[255:224] & ~wmask);
    end
end

// int_request[287:256]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[287:256] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_8)
            int_request[287:256] <= (WDATA[31:0] & wmask) | (int_request[287:256] & ~wmask);
    end
end

// int_request[319:288]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[319:288] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_9)
            int_request[319:288] <= (WDATA[31:0] & wmask) | (int_request[319:288] & ~wmask);
    end
end

// int_request[351:320]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[351:320] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_10)
            int_request[351:320] <= (WDATA[31:0] & wmask) | (int_request[351:320] & ~wmask);
    end
end

// int_request[383:352]
always @(posedge ACLK) begin
    if (ARESET)
        int_request[383:352] <= 0;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_REQUEST_DATA_11)
            int_request[383:352] <= (WDATA[31:0] & wmask) | (int_request[383:352] & ~wmask);
    end
end

//synthesis translate_off
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (int_gie & ~int_isr[0] & int_ier[0] & ap_done)
            $display ("// Interrupt Monitor : interrupt for ap_done detected @ \"%0t\"", $time);
        if (int_gie & ~int_isr[1] & int_ier[1] & ap_ready)
            $display ("// Interrupt Monitor : interrupt for ap_ready detected @ \"%0t\"", $time);
    end
end
//synthesis translate_on

//------------------------Memory logic-------------------

endmodule
