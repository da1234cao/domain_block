#include "./rule.h"
#include "utils.h"
#include <ntstatus.h>

LIST_ENTRY rule_list;
WDFWAITLOCK rule_list_lock;

NTSTATUS init_rule() {
  WdfWaitLockCreate(WDF_NO_OBJECT_ATTRIBUTES, &rule_list_lock);
  InitializeListHead(&rule_list);
  return STATUS_SUCCESS;
}

rule_t &reverse_rule(rule_t &rule) {
  if (rule.remote_addr_info.ip_version = 4) {
    for (int i = 0; i < rule.remote_addr_info.v4.cnt; i++) {
      net2host_ipv4(rule.remote_addr_info.v4.domain_address[i]);
    }
  } else if (rule.remote_addr_info.ip_version = 6) {
    for (int i = 0; i < rule.remote_addr_info.v6.cnt; i++) {
      net2host_ipv6(rule.remote_addr_info.v6.domain_address[i]);
    }
  } else {
    KdPrint(("|LIBBLOCK|reverse_rule|unknow ip version: %d",
             rule.remote_addr_info.ip_version));
  }
  return rule;
}

NTSTATUS add_rule(rule_t &rule) {
  // 先反转内部存储的ip，便于后续比较
  reverse_rule(rule);

  // 没有去重,可能存在两条相同的规则，要处理下
  rule_node_t *node = (rule_node_t *)ExAllocatePool2(
      POOL_FLAG_NON_PAGED, sizeof(rule_node_t), WDF_BLOCK_TAG);
  if (node == NULL) {
    KdPrint(("|LIBBLOCK|add_rule|failed while executing ExAllocatePool2"));
    return STATUS_UNSUCCESSFUL;
  }

  memset(node, 0, sizeof(rule_node_t));
  node->rule = rule;

  KdPrint(("|LIBBLOCK|add_rule|add rule: domain is %s, action is %d",
           node->rule.remote_addr_info.domin_str, node->rule.action));
  WdfWaitLockAcquire(rule_list_lock, nullptr);
  InsertTailList(&rule_list, &node->list_entry);
  WdfWaitLockRelease(rule_list_lock);

  return STATUS_SUCCESS;
}

NTSTATUS del_rule(rule_t &rule) {
  reverse_rule(rule);

  char *domain = rule.remote_addr_info.domin_str;
  WdfWaitLockAcquire(rule_list_lock, nullptr);
  for (PLIST_ENTRY node = rule_list.Flink; node != &rule_list;
       node = node->Flink) {
    rule_node_t *rule_node = CONTAINING_RECORD(node, rule_node_t, list_entry);
    if (strcmp(rule_node->rule.remote_addr_info.domin_str, domain) == 0) {
      KdPrint(("|LIBBLOCK|del_rule|del %s kernel rule.", domain));
      RemoveEntryList(node); // 从链表结构中取下该节点
      ExFreePoolWithTag(rule_node, WDF_BLOCK_TAG); // 删除分配的内存
      break;
    }
  }
  WdfWaitLockRelease(rule_list_lock);
  return STATUS_SUCCESS;
}

NTSTATUS clear_rules() {
  KdPrint(("|LIBBLOCK|clear_rules"));
  WdfWaitLockAcquire(rule_list_lock, nullptr);
  while (IsListEmpty(&rule_list) == FALSE) {
    PLIST_ENTRY node = RemoveTailList(&rule_list);
    rule_node_t *rule_node = CONTAINING_RECORD(node, rule_node_t, list_entry);
    KdPrint(("|LIBBLOCK|clear_rules|clear %s kernel rule.",
             rule_node->rule.remote_addr_info.domin_str));
    ExFreePoolWithTag(rule_node, WDF_BLOCK_TAG);
  }
  WdfWaitLockRelease(rule_list_lock);

  return STATUS_SUCCESS;
}

BOOLEAN is_hint_rule(connect_info_t &connect_info) {
  BOOLEAN result = FALSE;

  WdfWaitLockAcquire(rule_list_lock, nullptr);
  for (PLIST_ENTRY node = rule_list.Flink; node != &rule_list;
       node = node->Flink) {
    rule_node_t *rule_node = CONTAINING_RECORD(node, rule_node_t, list_entry);
    if (connect_info.ip_version == 4) {
      for (int i = 0; i < rule_node->rule.remote_addr_info.v4.cnt; i++) {
        // KdPrint(("|LIBBLOCK|is_hint_rule|look ip is %d.%d.%d.%d",
        // FORMAT_ADDR4(connect_info.v4.remote_address)));
        // KdPrint(("|LIBBLOCK|is_hint_rule|current matching ip is %d.%d.%d.%d",
        // FORMAT_ADDR4(rule_node->rule.remote_addr_info.v4.domain_address[i])));
        if (is_same_ipv4(
                connect_info.v4.remote_address,
                rule_node->rule.remote_addr_info.v4.domain_address[i])) {
          result = TRUE;
          KdPrint(("|LIBBLOCK|is_hint_rule|hint rule: ip is %d.%d.%d.%d, "
                   "action is %d",
                   FORMAT_ADDR4(connect_info.v4.remote_address),
                   rule_node->rule.action));
          goto end;
        }
      }
    } else if (connect_info.ip_version == 6) {
      for (int i = 0; i < rule_node->rule.remote_addr_info.v6.cnt; i++) {
        if (is_same_ipv6(
                connect_info.v6.remote_address,
                rule_node->rule.remote_addr_info.v6.domain_address[i])) {
          result = TRUE;
          KdPrint(("|LIBBLOCK|is_hint_rule|hint rule: ip "
                   "is%x:%x:%x:%x:%x:%x:%x::%x, action is %d",
                   FORMAT_ADDR6(connect_info.v6.remote_address),
                   rule_node->rule.action));
          goto end;
        }
      }
    } else {
      KdPrint(("|LIBBLOCK|is_hint_rule|unknow ip version: %d",
               connect_info.ip_version == 4));
    }
  }

end:
  WdfWaitLockRelease(rule_list_lock);
  return result;
}