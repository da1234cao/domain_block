#pragma once

#include <in6addr.h>
#include <inaddr.h>

#define MAX_DOMAIN_ADDRESS_CNT 10 // 如果域名对应的ip4(ipv6)超过10个，取前10个

// {0CF72951-F078-4854-9AFD-0E870B0ABEBA}
DEFINE_GUID(LIBBLOCK_CALLOUT_GUID_V4, 0xcf72951, 0xf078, 0x4854, 0x9a, 0xfd,
            0xe, 0x87, 0xb, 0xa, 0xbe, 0xba);

// {EAD094BF-4EC4-4CA6-A343-E1170FF90DB6}
DEFINE_GUID(LIBBLOCK_CALLOUT_GUID_V6, 0xead094bf, 0x4ec4, 0x4ca6, 0xa3, 0x43,
            0xe1, 0x17, 0xf, 0xf9, 0xd, 0xb6);

// {AE1E820A-C60A-42A8-B4A2-9ACFB050387F}
DEFINE_GUID(LIBBLOCK_SUBLAYER_GUID, 0xae1e820a, 0xc60a, 0x42a8, 0xb4, 0xa2,
            0x9a, 0xcf, 0xb0, 0x50, 0x38, 0x7f);

#define IOCTL_GET_DOMAIN                                                       \
  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_DOMAIN                                                       \
  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DEL_DOMAIN                                                       \
  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

enum action_t { BLOCK, ALLOW };

typedef struct _addr_info_t {
  int ip_version;
  char domin_str[260]; // 域名的最大长度为253
  union {
    struct {
      int cnt;
      // 为了方便使用这里不使用柔性数组
      IN_ADDR domain_address[MAX_DOMAIN_ADDRESS_CNT];
    } v4;
    struct {
      int cnt;
      IN6_ADDR domain_address[MAX_DOMAIN_ADDRESS_CNT];
    } v6;
  };
} addr_info_t;

typedef struct _rule_t {
  addr_info_t remote_addr_info;
  action_t action = BLOCK;
} rule_t;
