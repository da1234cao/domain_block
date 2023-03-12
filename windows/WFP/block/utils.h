#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <inaddr.h>
#include <in6addr.h>

// 将主机序的地址，以网路序的方式显示

#define FORMAT_ADDR4(x)                                                        \
  x.S_un.S_un_b.s_b4, x.S_un.S_un_b.s_b3, x.S_un.S_un_b.s_b2, x.S_un.S_un_b.s_b1

#define FORMAT_ADDR6(x)                                                        \
  RtlUshortByteSwap(x.u.Word[7]), RtlUshortByteSwap(x.u.Word[6]),              \
      RtlUshortByteSwap(x.u.Word[5]), RtlUshortByteSwap(x.u.Word[4]),          \
      RtlUshortByteSwap(x.u.Word[3]), RtlUshortByteSwap(x.u.Word[2]),          \
      RtlUshortByteSwap(x.u.Word[1]), RtlUshortByteSwap(x.u.Word[0])

BOOLEAN is_same_ipv4(const IN_ADDR &first, const IN_ADDR &second);
BOOLEAN is_same_ipv6(const IN6_ADDR &first, const IN6_ADDR &second);

// 存储上-主机序转换成网络序列
VOID host2net_ipv4(IN_ADDR &ip);
VOID host2net_ipv6(IN6_ADDR &ip);

// 存储上-网络序列转主机序列
VOID net2host_ipv4(IN_ADDR &ip);
VOID net2host_ipv6(IN6_ADDR &ip);