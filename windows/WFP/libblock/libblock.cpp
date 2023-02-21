#define INITGUID
#include <cassert>

#include "libblock.h"
#include "../utils/log.hpp"

DWORD libblock_init(DWORD init_type, HANDLE* engine_handle)
{
	Log::SPDLOG::getInstance().init("libblock.txt", "libblock_logger", "Debug", 5*1024*1024, 3, false);
    Log::LOG_DEBUG("libblock init.");

	FWPM_SESSION session = { 0 };
	session.flags = FWPM_SESSION_FLAG_DYNAMIC;	// session结束后自动销毁所有callout和filter
	auto status = FwpmEngineOpen(nullptr, RPC_C_AUTHN_WINNT, nullptr, &session, engine_handle);
	if (status != ERROR_SUCCESS)
	{
		return status;
	}

	status = FwpmTransactionBegin(*engine_handle, 0);
	if (status != ERROR_SUCCESS)
	{
		return status;
	}

	if (init_type & LIBBLOCK_INIT_IPV4)
	{
		FWPM_CALLOUT callout_v4 = { 0 };
		FWPM_DISPLAY_DATA display_data = { 0 };
		wchar_t callout_display_name_v4[] = L"LibBlockCalloutV4";
		wchar_t callout_display_desc_v4[] = L"IPv4 callout for LIBBLOCK";
		display_data.name = callout_display_name_v4;
		display_data.description = callout_display_desc_v4;

		callout_v4.calloutKey = LIBBLOCK_CALLOUT_GUID_V4;
		callout_v4.displayData = display_data;
		callout_v4.applicableLayer = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4;
		callout_v4.flags = 0;
		status = FwpmCalloutAdd(*engine_handle, &callout_v4, nullptr, nullptr);
		if (status != ERROR_SUCCESS)
		{
			FwpmTransactionAbort(*engine_handle);
			return status;
		}
		Log::LOG_DEBUG("libblock init: add ipv4 block callout.");
	}
	if (init_type & LIBBLOCK_INIT_IPV6)
	{
		FWPM_CALLOUT callout_v6 = { 0 };
		FWPM_DISPLAY_DATA display_data_v6 = { 0 };
		wchar_t callout_display_name_v6[] = L"LibBlockCalloutV6";
		wchar_t callout_display_desc_v6[] = L"IPv6 callout for LIBBLOCK";
		display_data_v6.name = callout_display_name_v6;
		display_data_v6.description = callout_display_desc_v6;

		callout_v6.calloutKey = LIBBLOCK_CALLOUT_GUID_V6;
		callout_v6.displayData = display_data_v6;
		callout_v6.applicableLayer = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V6;
		callout_v6.flags = 0;
		status = FwpmCalloutAdd(*engine_handle, &callout_v6, nullptr, nullptr);
		if (status != ERROR_SUCCESS)
		{
			FwpmTransactionAbort(*engine_handle);
			return status;
		}
		Log::LOG_DEBUG("libblock init: add ipv6 block callout.");
	}

	FWPM_SUBLAYER sublayer = { 0 };
	sublayer.subLayerKey = LIBBLOCK_SUBLAYER_GUID;
	wchar_t sublayer_display_name[] = L"LIBBLOCKSublayer";;
	sublayer.displayData.name = sublayer_display_name;
	wchar_t sublayer_display_desc[] = L"Sublayer for LIBBLOCK";
	sublayer.displayData.description = sublayer_display_desc;
	sublayer.flags = 0;
	sublayer.weight = 0x0f;
	status = FwpmSubLayerAdd(*engine_handle, &sublayer, nullptr);
	if (status != ERROR_SUCCESS)
	{
		FwpmTransactionAbort(*engine_handle);
		return status;
	}
	Log::LOG_DEBUG("libblock init: add block sublayer.");

	if (init_type & LIBBLOCK_INIT_IPV4)
	{
		FWPM_FILTER filter_v4 = { 0 };
		UINT64 filter_id_v4 = 0;
		wchar_t filter_display_name_v4[] = L"LIBBLOCKFilterV4";
		filter_v4.displayData.name = filter_display_name_v4;
		wchar_t filter_display_desc_v4[] = L"IPv4 filter for LIBBLOCK";
		filter_v4.displayData.description = filter_display_desc_v4;
		filter_v4.action.type = FWP_ACTION_CALLOUT_TERMINATING;
		filter_v4.subLayerKey = LIBBLOCK_SUBLAYER_GUID;
		filter_v4.weight.type = FWP_UINT8;
		filter_v4.weight.uint8 = 0xf;
		filter_v4.numFilterConditions = 0;
		filter_v4.layerKey = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4;
		filter_v4.action.calloutKey = LIBBLOCK_CALLOUT_GUID_V4;
		status = FwpmFilterAdd(*engine_handle, &filter_v4, nullptr, &filter_id_v4);
		if (status != ERROR_SUCCESS)
		{
			FwpmTransactionAbort(*engine_handle);
			return status;
		}
		Log::LOG_DEBUG("libblock init: add ipv4 block filter.");
	}

	if (init_type & LIBBLOCK_INIT_IPV6)
	{
		FWPM_FILTER filter_v6 = { 0 };
		UINT64 filter_id_v6 = 0;
		wchar_t filter_display_name_v6[] = L"LIBBLOCKFilterV6";
		filter_v6.displayData.name = filter_display_name_v6;
		wchar_t filter_display_desc_v6[] = L"IPv6 filter for LIBBLOCK";
		filter_v6.displayData.description = filter_display_desc_v6;
		filter_v6.action.type = FWP_ACTION_CALLOUT_TERMINATING;
		filter_v6.subLayerKey = LIBBLOCK_SUBLAYER_GUID;
		filter_v6.weight.type = FWP_UINT8;
		filter_v6.weight.uint8 = 0xf;
		filter_v6.numFilterConditions = 0;
		filter_v6.layerKey = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V6;
		filter_v6.action.calloutKey = LIBBLOCK_CALLOUT_GUID_V6;
		status = FwpmFilterAdd(*engine_handle, &filter_v6, nullptr, &filter_id_v6);
		if (status != ERROR_SUCCESS)
		{
			FwpmTransactionAbort(*engine_handle);
			return status;
		}
		Log::LOG_DEBUG("libblock init: add ipv6 block filter.");
	}

	status = FwpmTransactionCommit(*engine_handle);

	return status;
}


void libblock_uninit(HANDLE engine_handle)
{
	Log::LOG_DEBUG("libblock uninit.");
	FwpmEngineClose(engine_handle);
}

HANDLE libblock_open()
{
	HANDLE handle = CreateFileA("\\\\.\\libblock",
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    Log::LOG_DEBUG("libblock opne handle {}.", handle);
    return handle;
}

void libblock_close(HANDLE handle)
{
	Log::LOG_DEBUG("libblock close.");
	CloseHandle(handle);
}



int libblock_get_domain(HANDLE handle, rule_t* conn)
{
	return 1;
}

int libblock_del_domain(HANDLE handle, const rule_t &conn)
{
	DWORD recv_num;
	DeviceIoControl(handle, IOCTL_DEL_DOMAIN, (LPVOID)&conn, sizeof(rule_t), nullptr, 0, &recv_num, nullptr);
	return recv_num;
}

int libblock_set_domain(HANDLE handle, const rule_t &conn)
{
	DWORD recv_num;
	DeviceIoControl(handle, IOCTL_SET_DOMAIN, (LPVOID)&conn, sizeof(rule_t), nullptr, 0, &recv_num, nullptr);
	return recv_num;
}

