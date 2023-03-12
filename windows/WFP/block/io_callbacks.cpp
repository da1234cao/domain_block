#include <ntifs.h>
#include <ntddk.h> // 包含大量 Windows 内核 API 的定义
#include "io_callbacks.h"
#include "../libblock/libblock_common.h"
#include "rule.h"
#include "utils.h"

VOID file_create(IN WDFDEVICE device, IN WDFREQUEST request,
                 IN WDFFILEOBJECT object) {
  UNREFERENCED_PARAMETER(device);
  UNREFERENCED_PARAMETER(object);
  KdPrint(("|LIBBLOCK|file_create|"));
  WdfRequestComplete(request, STATUS_SUCCESS);
}

VOID file_close(IN WDFFILEOBJECT object) {
  KdPrint(("|LIBBLOCK|file_close|"));
  UNREFERENCED_PARAMETER(object);
}

VOID file_cleanup(IN WDFFILEOBJECT object) {
  KdPrint(("|LIBBLOCK|file_cleanup|"));
  UNREFERENCED_PARAMETER(object);
}

VOID device_ioctl(IN WDFQUEUE queue, IN WDFREQUEST request,
                  IN size_t out_length, IN size_t in_length, IN ULONG code) {
  UNREFERENCED_PARAMETER(queue);
  UNREFERENCED_PARAMETER(out_length);
  UNREFERENCED_PARAMETER(in_length);

  if (code == IOCTL_GET_DOMAIN) {
    KdPrint(("|LIBBLOCK|device_ioctl|IOCTL_GET_DOMAIN"));
    WdfRequestComplete(request, STATUS_SUCCESS);
  } else if (code == IOCTL_SET_DOMAIN) {
    KdPrint(("|LIBBLOCK|device_ioctl|IOCTL_SET_DOMAIN"));
    void *inbuf;
    size_t inlen;
    NTSTATUS status =
        WdfRequestRetrieveInputBuffer(request, sizeof(rule_t), &inbuf, &inlen);
    if (!NT_SUCCESS(status)) {
      KdPrint(("|LIBBLOCK|device_ioctl|get input buffer error: %d", status));
      WdfRequestComplete(request, status);
      return;
    }
    if (inlen != sizeof(rule_t)) {
      KdPrint(
          ("|LIBBLOCK|device_ioctl|get input buffer len error: %zu", inlen));
      WdfRequestComplete(request, STATUS_INVALID_PARAMETER);
      return;
    }

    status = add_rule(*reinterpret_cast<rule_t *>(inbuf));
    WdfRequestComplete(request, status);
  } else if (code == IOCTL_DEL_DOMAIN) {
    // 和上面条件的代码相似度极高，可考虑合并，通过一个函数实现
    KdPrint(("|LIBBLOCK|device_ioctl|IOCTL_DEL_DOMAIN"));
    void *inbuf;
    size_t inlen;
    NTSTATUS status =
        WdfRequestRetrieveInputBuffer(request, sizeof(rule_t), &inbuf, &inlen);
    if (!NT_SUCCESS(status)) {
      KdPrint(("|LIBBLOCK|device_ioctl|get input buffer error: %d", status));
      WdfRequestComplete(request, status);
      return;
    }
    if (inlen != sizeof(rule_t)) {
      KdPrint(
          ("|LIBBLOCK|device_ioctl|get input buffer len error: %zu", inlen));
      WdfRequestComplete(request, STATUS_INVALID_PARAMETER);
      return;
    }

    status = del_rule(*reinterpret_cast<rule_t *>(inbuf));
    WdfRequestComplete(request, status);
  } else {
    KdPrint(("|LIBBLOCK|device_ioctl|UNKNOWN code"));
    WdfRequestComplete(request, STATUS_UNSUCCESSFUL);
  }
}
