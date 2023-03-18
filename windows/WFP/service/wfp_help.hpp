#pragma once
#include "libblock_common_help.hpp"
#include "nlohmann/json.hpp"
#include "utils/common.hpp"
#include "utils/log.h"
#include <map>
#include <string>
#include <tuple>
#include <ws2tcpip.h>

struct rule_wrapper_t {
  rule_wrapper_t() {
    rule = {0};
    // 规则没有被应用的时候(当出现新规则,或者修改规则)，设置为false
    used = false;
  }
  rule_wrapper_t(rule_t _rule, bool _used = false) : rule(_rule), used(_used) {}
  rule_t rule;
  bool used;
};

class wfp_help {
public:
  static wfp_help &instance() {
    static wfp_help inst;
    return inst;
  }

  void parse_rules(std::string rules) {
    // 没有出现过的规则; 出现过&&现进行修改的规则 ->进行保存
    nlohmann::json json = nlohmann::json::parse(rules);
    for (nlohmann::json &json_rule : json["rules"]) {
      // 从json中提取出一条规则
      std::string domain = json_rule["domain"].get<std::string>();
      std::string action = json_rule["action"].get<std::string>();
      rule_t rule = {0};
      rule_t rule_v6 = {0};
      if (action == "block") {
        rule.action = BLOCK;
      } else {
        rule.action = ALLOW;
      }
      rule_v6 = rule; // 拷贝下规则的相同部分
      nlohmann::json &ips = json_rule["ip"];
      for (int i = 0; i < ips.size(); i++) {
        std::string ip = ips[i].get<std::string>();
        add_ip(rule, rule_v6, ip);
      }

      // 将提取的ipv4规则存储
      if (rule.remote_addr_info.v4.cnt > 0) {
        std::string domain_v4 = domain + "_v4";
        std::strcpy(rule.remote_addr_info.domin_str, domain_v4.c_str());
        if (!rules_wrapper.count(domain_v4)) {
          // 没有出现过的规则直接添加
          rules_wrapper[domain_v4] = rule_wrapper_t(rule);
        } else if (rules_wrapper[domain_v4].rule.action != rule.action) {
          // 修改规则(目前只考虑修改放行/允许)
          //// 直接覆盖
          rules_wrapper[domain_v4] = rule_wrapper_t(rule);
        }
      }

      // 将提取的ipv6规则存储
      if (rule_v6.remote_addr_info.v6.cnt > 0) {
        std::string domain_v6 = domain + "_v6";
        std::strcpy(rule_v6.remote_addr_info.domin_str, domain_v6.c_str());
        if (!rules_wrapper.count(domain_v6)) {
          rules_wrapper[domain_v6] = rule_wrapper_t(rule_v6);
        } else if (rules_wrapper[domain_v6].rule.action != rule.action) {
          rules_wrapper[domain_v6] = rule_wrapper_t(rule_v6);
        }
      }
    }
    LOG_TRACE("parse rule end;");
    log_now_rules();
  }

  // 函数功能：应用所有规则后，删除不需要的规则
  // 缺点，重复遍历规则
  // 改进方法：略
  void send_all_rules() {
    // 将规则发送到内核
    for (auto &r : rules_wrapper) {
      if (r.second.used == true)
        continue; // 已经出现并发送过给驱动的规则，不要重新发送
      if (r.second.rule.action == BLOCK) {
        libblock_set_domain(handle, r.second.rule);
        LOG_TRACE("block this rule in kernel: {}", r.first);
      } else if (r.second.rule.action == ALLOW) {
        libblock_del_domain(handle, r.second.rule);
        LOG_TRACE("allow this rule in kernel: {}", r.first);
      }
      r.second.used = true;
    }

    // 一边遍历，一边删除是危险的动作
    // 这里去除那些used==false的规则
    for (auto it = rules_wrapper.begin(); it != rules_wrapper.end();) {
      if (it->second.used == false) {
        it = rules_wrapper.erase(it);
      } else {
        it++;
      }
    }
  }

  void remove_all_rules() {
    for (auto &r : rules_wrapper) {
      if (r.second.used == false)
        continue;
      if (r.second.rule.action == BLOCK) {
        libblock_del_domain(handle, r.second.rule);
        LOG_TRACE("delete this rule in kernel: {}", r.first);
      }
    }
    rules_wrapper.clear();
  }

  HANDLE get_engine_handle() const { return engine_handle; }

  ~wfp_help() {
    remove_all_rules();
    libblock_uninit(engine_handle);
    libblock_close(handle);
  }

private:
  wfp_help() {
    libblock_init(LIBBLOCK_INIT_IPV4 | LIBBLOCK_INIT_IPV6, &engine_handle);
    handle = libblock_open();
  }

  void log_now_rules() {
    for (auto &rule_wrapper : rules_wrapper) {
      LOG_TRACE("struct {}", rule_wrapper.first);
      LOG_TRACE("used: {}", rule_wrapper.second.used);
      LOG_TRACE("rule.action: {}", rule_wrapper.second.rule.action);
      LOG_TRACE("rule.domain: {}",
                rule_wrapper.second.rule.remote_addr_info.domin_str);
      if (rule_wrapper.second.rule.remote_addr_info.ip_version == 4) {
        for (int i = 0; i < rule_wrapper.second.rule.remote_addr_info.v4.cnt;
             i++) {
          LOG_TRACE("rule.ip: {}", common::addr_to_string(
                                       rule_wrapper.second.rule.remote_addr_info
                                           .v4.domain_address[i]));
        }
      } else if (rule_wrapper.second.rule.remote_addr_info.ip_version == 6) {
        for (int i = 0; i < rule_wrapper.second.rule.remote_addr_info.v4.cnt;
             i++) {
          LOG_TRACE("rule.ip: {}", common::addr_to_string(
                                       rule_wrapper.second.rule.remote_addr_info
                                           .v6.domain_address[i]));
        }
      } else {
        LOG_WARN("unknow ip version.");
      }
    }
  }

private:
  HANDLE handle;
  HANDLE engine_handle;
  std::map<std::string, rule_wrapper_t> rules_wrapper;
};