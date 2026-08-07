// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2025.2 (64-bit)
// Tool Version Limit: 2025.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#ifdef SDT
#include "xparameters.h"
#endif
#include "xhngac_authorize.h"

extern XHngac_authorize_Config XHngac_authorize_ConfigTable[];

#ifdef SDT
XHngac_authorize_Config *XHngac_authorize_LookupConfig(UINTPTR BaseAddress) {
	XHngac_authorize_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XHngac_authorize_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XHngac_authorize_ConfigTable[Index].Control_BaseAddress == BaseAddress) {
			ConfigPtr = &XHngac_authorize_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XHngac_authorize_Initialize(XHngac_authorize *InstancePtr, UINTPTR BaseAddress) {
	XHngac_authorize_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XHngac_authorize_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XHngac_authorize_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XHngac_authorize_Config *XHngac_authorize_LookupConfig(u16 DeviceId) {
	XHngac_authorize_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XHNGAC_AUTHORIZE_NUM_INSTANCES; Index++) {
		if (XHngac_authorize_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XHngac_authorize_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XHngac_authorize_Initialize(XHngac_authorize *InstancePtr, u16 DeviceId) {
	XHngac_authorize_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XHngac_authorize_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XHngac_authorize_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

