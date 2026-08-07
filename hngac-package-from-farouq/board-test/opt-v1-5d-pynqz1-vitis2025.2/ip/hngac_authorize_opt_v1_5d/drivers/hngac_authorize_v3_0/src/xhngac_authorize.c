// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xhngac_authorize.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XHngac_authorize_CfgInitialize(XHngac_authorize *InstancePtr, XHngac_authorize_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XHngac_authorize_Start(XHngac_authorize *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_AP_CTRL) & 0x80;
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XHngac_authorize_IsDone(XHngac_authorize *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XHngac_authorize_IsIdle(XHngac_authorize *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XHngac_authorize_IsReady(XHngac_authorize *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XHngac_authorize_EnableAutoRestart(XHngac_authorize *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XHngac_authorize_DisableAutoRestart(XHngac_authorize *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_AP_CTRL, 0);
}

u32 XHngac_authorize_Get_return(XHngac_authorize *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_AP_RETURN);
    return Data;
}
void XHngac_authorize_Set_rule_count(XHngac_authorize *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_RULE_COUNT_DATA, Data);
}

u32 XHngac_authorize_Get_rule_count(XHngac_authorize *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_RULE_COUNT_DATA);
    return Data;
}

void XHngac_authorize_Set_request(XHngac_authorize *InstancePtr, XHngac_authorize_Request Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 0, Data.word_0);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 4, Data.word_1);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 8, Data.word_2);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 12, Data.word_3);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 16, Data.word_4);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 20, Data.word_5);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 24, Data.word_6);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 28, Data.word_7);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 32, Data.word_8);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 36, Data.word_9);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 40, Data.word_10);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 44, Data.word_11);
}

XHngac_authorize_Request XHngac_authorize_Get_request(XHngac_authorize *InstancePtr) {
    XHngac_authorize_Request Data;

    Data.word_0 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 0);
    Data.word_1 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 4);
    Data.word_2 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 8);
    Data.word_3 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 12);
    Data.word_4 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 16);
    Data.word_5 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 20);
    Data.word_6 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 24);
    Data.word_7 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 28);
    Data.word_8 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 32);
    Data.word_9 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 36);
    Data.word_10 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 40);
    Data.word_11 = XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_REQUEST_DATA + 44);
    return Data;
}

void XHngac_authorize_InterruptGlobalEnable(XHngac_authorize *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_GIE, 1);
}

void XHngac_authorize_InterruptGlobalDisable(XHngac_authorize *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_GIE, 0);
}

void XHngac_authorize_InterruptEnable(XHngac_authorize *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_IER);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_IER, Register | Mask);
}

void XHngac_authorize_InterruptDisable(XHngac_authorize *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_IER);
    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_IER, Register & (~Mask));
}

void XHngac_authorize_InterruptClear(XHngac_authorize *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHngac_authorize_WriteReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_ISR, Mask);
}

u32 XHngac_authorize_InterruptGetEnabled(XHngac_authorize *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_IER);
}

u32 XHngac_authorize_InterruptGetStatus(XHngac_authorize *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHngac_authorize_ReadReg(InstancePtr->Control_BaseAddress, XHNGAC_AUTHORIZE_CONTROL_ADDR_ISR);
}

