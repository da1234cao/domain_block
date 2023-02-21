#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <stddef.h>
#include "../libblock/libblock_common.h" // 这个需要放在neddk.h头文件下面。因为里面使用了IN_ADDR/IN6_AAADR,需要ntddk.h这个头文件，但是neddk.h不能包含在用户层的代码中
