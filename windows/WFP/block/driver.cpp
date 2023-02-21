
#define NDIS61 1				// Need to declare this to compile WFP stuff on Win7, I'm not sure why

#include "Ntifs.h"
#include <ntddk.h>				// Windows Driver Development Kit
#include <wdf.h>				// Windows Driver Foundation

#include "device.h"
#include "rule.h"
#include "wfp.h"


VOID libblock_unload(IN WDFDRIVER driver)
{
	UNREFERENCED_PARAMETER(driver);
	KdPrint(("|LIBBLOCK|libblock_unload"));
	clear_rules();
	// wfp_uninit();
	// device_uninit();
}


extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT driver_obj, IN PUNICODE_STRING reg_path)
{
	KdPrint(("|LIBBLOCK|DriverEntry"));
	// Configure ourself as a non-PnP driver:
	WDF_DRIVER_CONFIG config;
	WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
	config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
	config.EvtDriverUnload = libblock_unload;
	WDFDRIVER driver;
	NTSTATUS status = WdfDriverCreate(driver_obj, reg_path, WDF_NO_OBJECT_ATTRIBUTES, &config, &driver);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("|LIBBLOCK|DriverEntry|failed to create WDF driver: %d", status));
		return status;
	}

	WDFDEVICE device;
	status = device_init(driver, device);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("|LIBBLOCK|DriverEntry|failed in device init: %d", status));
		return status;
	}

	// 初始化存储结构
	init_rule();

	PDEVICE_OBJECT pdev_obj = WdfDeviceWdmGetDeviceObject(device);
	status = wfp_init(pdev_obj);

	KdPrint(("|LIBBLOCK|DriverEntry|end."));
	return status;
}