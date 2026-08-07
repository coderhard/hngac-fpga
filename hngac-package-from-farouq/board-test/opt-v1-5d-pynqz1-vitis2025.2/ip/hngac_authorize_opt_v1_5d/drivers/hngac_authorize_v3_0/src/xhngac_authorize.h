// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XHNGAC_AUTHORIZE_H
#define XHNGAC_AUTHORIZE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xhngac_authorize_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Control_BaseAddress;
} XHngac_authorize_Config;
#endif

typedef struct {
    u64 Control_BaseAddress;
    u32 IsReady;
} XHngac_authorize;

typedef u32 word_type;

typedef struct {
    u32 word_0;
    u32 word_1;
    u32 word_2;
    u32 word_3;
    u32 word_4;
    u32 word_5;
    u32 word_6;
    u32 word_7;
    u32 word_8;
    u32 word_9;
    u32 word_10;
    u32 word_11;
} XHngac_authorize_Request;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XHngac_authorize_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XHngac_authorize_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XHngac_authorize_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XHngac_authorize_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifdef SDT
int XHngac_authorize_Initialize(XHngac_authorize *InstancePtr, UINTPTR BaseAddress);
XHngac_authorize_Config* XHngac_authorize_LookupConfig(UINTPTR BaseAddress);
#else
int XHngac_authorize_Initialize(XHngac_authorize *InstancePtr, u16 DeviceId);
XHngac_authorize_Config* XHngac_authorize_LookupConfig(u16 DeviceId);
#endif
int XHngac_authorize_CfgInitialize(XHngac_authorize *InstancePtr, XHngac_authorize_Config *ConfigPtr);
#else
int XHngac_authorize_Initialize(XHngac_authorize *InstancePtr, const char* InstanceName);
int XHngac_authorize_Release(XHngac_authorize *InstancePtr);
#endif

void XHngac_authorize_Start(XHngac_authorize *InstancePtr);
u32 XHngac_authorize_IsDone(XHngac_authorize *InstancePtr);
u32 XHngac_authorize_IsIdle(XHngac_authorize *InstancePtr);
u32 XHngac_authorize_IsReady(XHngac_authorize *InstancePtr);
void XHngac_authorize_EnableAutoRestart(XHngac_authorize *InstancePtr);
void XHngac_authorize_DisableAutoRestart(XHngac_authorize *InstancePtr);
u32 XHngac_authorize_Get_return(XHngac_authorize *InstancePtr);

void XHngac_authorize_Set_rule_count(XHngac_authorize *InstancePtr, u32 Data);
u32 XHngac_authorize_Get_rule_count(XHngac_authorize *InstancePtr);
void XHngac_authorize_Set_request(XHngac_authorize *InstancePtr, XHngac_authorize_Request Data);
XHngac_authorize_Request XHngac_authorize_Get_request(XHngac_authorize *InstancePtr);

void XHngac_authorize_InterruptGlobalEnable(XHngac_authorize *InstancePtr);
void XHngac_authorize_InterruptGlobalDisable(XHngac_authorize *InstancePtr);
void XHngac_authorize_InterruptEnable(XHngac_authorize *InstancePtr, u32 Mask);
void XHngac_authorize_InterruptDisable(XHngac_authorize *InstancePtr, u32 Mask);
void XHngac_authorize_InterruptClear(XHngac_authorize *InstancePtr, u32 Mask);
u32 XHngac_authorize_InterruptGetEnabled(XHngac_authorize *InstancePtr);
u32 XHngac_authorize_InterruptGetStatus(XHngac_authorize *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
