#pragma once

#include <WinSock2.h>
#include <windows.h>
#include <WS2tcpip.h>
#include <fwpmu.h>
#include <fwpstypes.h>


// 生成动态库
#ifdef LIBBLOCK_EXPORTS
#define LIBBLOCK_API __declspec(dllexport)
#else
#define LIBBLOCK_API __declspec(dllimport)
#endif

#include "libblock_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LIBBLOCK_INIT_IPV4	1
#define LIBBLOCK_INIT_IPV6	2

	LIBBLOCK_API DWORD libblock_init(DWORD init_type, HANDLE* engine_handle);
	LIBBLOCK_API void libblock_uninit(HANDLE engine_handle);

	LIBBLOCK_API HANDLE libblock_open();
	LIBBLOCK_API void libblock_close(HANDLE handle);

	LIBBLOCK_API int libblock_set_domain(HANDLE handle, const rule_t &conn);
	LIBBLOCK_API int libblock_get_domain(HANDLE handle, rule_t* conn);
	LIBBLOCK_API int libblock_del_domain(HANDLE handle, const rule_t &conn);

#ifdef __cplusplus
}
#endif
