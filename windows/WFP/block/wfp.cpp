#include "wfp.h"
#include "wfp_callbacks.h"
#include "common.h"

HANDLE wfp_engine_handle = nullptr;
static UINT32 callout_id_v4;
static UINT32 callout_id_v6;

static NTSTATUS register_callout(DEVICE_OBJECT* wdm_device)
{
	if (!wfp_engine_handle)
		return STATUS_INVALID_HANDLE;

	// callout的注册必须在内核态，但是可以在用户态添加到引擎中
	NTSTATUS status = STATUS_SUCCESS;
	FWPS_CALLOUT callout_v4 = { 0 };
	callout_v4.calloutKey = LIBBLOCK_CALLOUT_GUID_V4;
	callout_v4.classifyFn = &callout_classify;
	callout_v4.notifyFn = &callout_notify;
	callout_v4.flowDeleteFn = &callout_flow_delete;
	status = FwpsCalloutRegister((void*)wdm_device, &callout_v4, &callout_id_v4);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("|LIBREDIRECT|register_callout|Failed to register IPv4 callout, status 0x%08x", status));
		return status;
	}

	FWPS_CALLOUT callout_v6 = { 0 };
	callout_v6.calloutKey = LIBBLOCK_CALLOUT_GUID_V6;
	callout_v6.classifyFn = &callout_classify;
	callout_v6.notifyFn = &callout_notify;
	callout_v6.flowDeleteFn = &callout_flow_delete;
	status = FwpsCalloutRegister((void*)wdm_device, &callout_v6, &callout_id_v6);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("|LIBBLOCK|register_callout|Failed to register IPv4 callout, status 0x%08x", status));
		return status;
	}

	return status;
}

static void unregister_callout()
{
	auto status = FwpsCalloutUnregisterById(callout_id_v4);
	if (!NT_SUCCESS(status)) {
		KdPrint(("|LIBBLOCK|unregister_callout|Failed to unregister IPv4 callout, status: 0x%08x", status));
	}

	status = FwpsCalloutUnregisterById(callout_id_v6);
	if (!NT_SUCCESS(status)) {
		KdPrint(("|LIBBLOCK|unregister_callout|Failed to unregister IPv6 callout, status: 0x%08x", status));
	}
}


NTSTATUS wfp_init(PDEVICE_OBJECT dev_obj)
{
	FWPM_SESSION session = { 0 };
	session.flags = FWPM_SESSION_FLAG_DYNAMIC;	// session结束后自动销毁所有callout和filter
	auto status = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, &session, &wfp_engine_handle);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	status = FwpmTransactionBegin(wfp_engine_handle, 0);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	status = register_callout(dev_obj);
	if (!NT_SUCCESS(status))
	{
		FwpmTransactionAbort(wfp_engine_handle);
		return status;
	}

	status = FwpmTransactionCommit(wfp_engine_handle);
	if (!NT_SUCCESS(status))
	{
		unregister_callout();
	}
	return status;
}

void wfp_uninit()
{
	KdPrint(("|LIBBLOCK|wfp_uninit"));
	unregister_callout();
	if (wfp_engine_handle)
	{
		FwpmEngineClose(wfp_engine_handle);
	}
}