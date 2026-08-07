// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// control
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

#define XHNGAC_AUTHORIZE_CONTROL_ADDR_AP_CTRL         0x00
#define XHNGAC_AUTHORIZE_CONTROL_ADDR_GIE             0x04
#define XHNGAC_AUTHORIZE_CONTROL_ADDR_IER             0x08
#define XHNGAC_AUTHORIZE_CONTROL_ADDR_ISR             0x0c
#define XHNGAC_AUTHORIZE_CONTROL_ADDR_AP_RETURN       0x10
#define XHNGAC_AUTHORIZE_CONTROL_BITS_AP_RETURN       1
#define XHNGAC_AUTHORIZE_CONTROL_ADDR_RULE_COUNT_DATA 0x18
#define XHNGAC_AUTHORIZE_CONTROL_BITS_RULE_COUNT_DATA 16
#define XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA    0x40
#define XHNGAC_AUTHORIZE_CONTROL_BITS_REQUEST_DATA    384
#define XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA_   0x68
#define XHNGAC_AUTHORIZE_CONTROL_BITS_REQUEST_DATA    384

