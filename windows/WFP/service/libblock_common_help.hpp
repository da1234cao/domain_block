#include "../libblock/libblock.h"
#include "../libblock/libblock_common.h"
#include "utils/common.hpp"

inline bool add_ip(rule_t &rule, rule_t &rule_v6, std::string &ip) {
  bool flag = true;
  if (common::is_ip(ip) == false) {
    flag = false;
  } else if (common::is_ipv4(ip)) {
    rule.remote_addr_info.ip_version = 4;
    auto &cnt = rule.remote_addr_info.v4.cnt;
    if (cnt >= MAX_DOMAIN_ADDRESS_CNT) {
      flag = false;
    } else {
      inet_pton(AF_INET, ip.c_str(),
                &rule.remote_addr_info.v4.domain_address[cnt]);
      cnt++;
    }
  } else {
    rule_v6.remote_addr_info.ip_version = 6;
    auto &cnt = rule_v6.remote_addr_info.v6.cnt;
    if (cnt >= MAX_DOMAIN_ADDRESS_CNT) {
      flag = false;
    } else {
      inet_pton(AF_INET6, ip.c_str(),
                &rule_v6.remote_addr_info.v6.domain_address[cnt]);
      cnt++;
    }
  }
  return flag;
}