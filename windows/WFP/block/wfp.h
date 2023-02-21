#pragma once

#include <ntddk.h>

extern HANDLE wfp_engine_handle;

NTSTATUS wfp_init(PDEVICE_OBJECT dev_obj);
void wfp_uninit();
