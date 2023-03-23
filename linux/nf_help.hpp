#pragma once
#include "nlohmann/json.hpp"
#include "utils/common.hpp"
#include "utils/log.h"
#include <boost/process.hpp>
#include <set>
#include <vector>

class nf_help {
public:
  static nf_help &instance() {
    static nf_help inst;
    return inst;
  }

  void parse_rules(std::string rules) {
    // 没有出现过的规则; 出现过&&现进行修改的规则 ->进行保存
    nlohmann::json json = nlohmann::json::parse(rules);
    for (nlohmann::json &json_rule : json["rules"]) {
      std::string domain = json_rule["domain"].get<std::string>();
      std::string action = json_rule["action"].get<std::string>();
      nlohmann::json &ips = json_rule["ip"];
      for (int i = 0; i < ips.size(); i++) {
        std::string ip = ips[i].get<std::string>();
        if (action == "block") {
          if (have_blocked_ips.count(ip)) {
            continue;
          } else {
            need_block_ips.emplace(ip);
          }
        } else if (action == "allow") {
          if (have_blocked_ips.count(ip)) {
            need_allow_ips.emplace(ip);
          } else {
            continue;
          }
        }
      }
    }
    LOG_TRACE("parse rule end;");
  }

  void send_all_rules() {
    // 需要阻塞的ip。阻塞成功后，放入已阻塞集合
    std::vector<std::string> nf_rules;
    for (auto it = need_block_ips.begin(); it != need_block_ips.end();) {
      std::string block_ip_rule =
          "nft add rule inet block OUTPUT ip daddr " + *it + " drop";
      nf_rules.push_back(block_ip_rule);
      // 假定之后可以添加规则成功。这是个智障的假定。>_<
      have_blocked_ips.insert(*it);
      it = need_block_ips.erase(it);
    }
    exec_rules(nf_rules);

    // 放行已经阻塞的ip
    nf_rules.clear();
    for (auto it = need_allow_ips.begin(); it != need_allow_ips.end();) {
      // sudo nft list chain inet block OUTPUT  --handle |  grep
      // '202.89.233.101' | awk '{print $NF}'
      std::string allow_ip_rule =
          "nft delete rule inet block OUTPUT ip daddr " + *it + " drop";
      nf_rules.push_back(allow_ip_rule);
      // 假定之后可以添加规则成功。这是个智障的假定。>_<
      have_blocked_ips.erase(*it);
      it = need_allow_ips.erase(it);
    }
    exec_rules(nf_rules);
  }

  ~nf_help() {
    std::vector<std::string> nf_exit_cmds;
    // 清空链
    nf_exit_cmds.emplace_back("nft flush chain inet block OUTPUT");
    // 删除链
    nf_exit_cmds.emplace_back("nft delete chain inet block OUTPUT");
    // 清空表
    nf_exit_cmds.emplace_back("nft flush table inet block");
    // 删除表
    nf_exit_cmds.emplace_back("nft delete table inet block");

    exec_rules(nf_exit_cmds);
  }

private:
  nf_help() {
    std::vector<std::string> nf_init_cmds;
    // 创建ipv4/ipv6还用的inet表；表名为block
    nf_init_cmds.emplace_back("nft add table inet block");
    // 在inet类型的block表中，添加一个名为OUTOUT的基础链
    // 这个基础链是filter类型，挂在在output钩子上，优先级是filter类型(0),默认的策略是放行
    nf_init_cmds.emplace_back(
        "nft add chain inet block OUTPUT { type filter hook output "
        "priority filter; policy accept; }");

    exec_rules(nf_init_cmds);
  }

  void exec_rules(std::vector<std::string> &cmds) {
    // 同步执行，不要执行耗时任务
    for (auto &cmd : cmds) {
      // 使用同步IO读取输出
      boost::process::ipstream is;
      LOG_TRACE("{}", cmd);
      boost::process::child c(cmd, boost::process::std_err > is);
      std::ostringstream oss;
      std::string line;
      while (c.running() && std::getline(is, line) && !line.empty()) {
        oss << line << std::endl;
      }
      c.wait();                      // 等待结束
      int exit_code = c.exit_code(); // 获取返回值
      if (exit_code != 0) { // 并没有规定程序执行成功返回0. echo $?
        LOG_WARN("cmd: {} exit code is {}. out message is: {}", cmd, exit_code,
                 oss.str());
      }
    }
  }

private:
  std::set<std::string> have_blocked_ips;
  std::set<std::string> need_block_ips;
  std::set<std::string> need_allow_ips;
};