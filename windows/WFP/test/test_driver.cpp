#include "../libblock/libblock.h"
#include <ws2tcpip.h>

int main(int argc, char *argv[]) {
  HANDLE engine_handle;
  libblock_init(LIBBLOCK_INIT_IPV4 | LIBBLOCK_INIT_IPV6, &engine_handle);
  HANDLE handle = libblock_open();

  rule_t baidu_forbid_rule;
  memset(&baidu_forbid_rule, 0, sizeof(rule_t));
  baidu_forbid_rule.action = BLOCK;
  strcpy(baidu_forbid_rule.remote_addr_info.domin_str, "www.baidu.com");
  baidu_forbid_rule.remote_addr_info.ip_version = 4;
  baidu_forbid_rule.remote_addr_info.v4.cnt = 2;
  inet_pton(AF_INET, "180.101.50.188",
            &baidu_forbid_rule.remote_addr_info.v4.domain_address[0]);
  inet_pton(AF_INET, "180.101.50.242",
            &baidu_forbid_rule.remote_addr_info.v4.domain_address[1]);

  libblock_set_domain(handle, baidu_forbid_rule);

  system("pause");

  libblock_del_domain(handle, baidu_forbid_rule);
  libblock_uninit(engine_handle);
  libblock_close(handle);
}