#pragma once
#include "nlohmann/json.hpp"
#include "utils/common.hpp"
#include "utils/log.h"
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
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
      std::string handle_info_cmd =
          "nft list chain inet block OUTPUT  --handle";
      nlohmann::json ret_json = exec_rule(handle_info_cmd);
      std::string handle_info_str = ret_json["out_str"].get<std::string>();
      int ip_handle_int = get_handle(handle_info_str, *it);
      if (ip_handle_int < 0) { // 没有找到指定ip的handle
        it = need_allow_ips.erase(it);
        continue;
      }
      std::string allow_ip_rule = "nft delete rule inet block OUTPUT handle " +
                                  std::to_string(ip_handle_int);
      nf_rules.push_back(allow_ip_rule);
      // 假定之后可以添加规则成功。这是个智障的假定。>_<
      have_blocked_ips.erase(*it);
      it = need_allow_ips.erase(it);
    }
    exec_rules(nf_rules);
  }

  ~nf_help() { flush_all(); }

private:
  nf_help() {
    // flush_all();
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

  void flush_all() {
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

  void exec_rules(std::vector<std::string> &cmds) {
    // 同步执行，不要执行耗时任务
    for (auto &cmd : cmds) {
      exec_rule(cmd);
    }
  }

  nlohmann::json pipe_exec_rule(std::string cmd) {
    // boost::process::system无法直接执行包含管道符号的命令
    // 将命令拆分开，前者的输出作为后者的输入

    // 这个函数有问题，当输入"nft list chain inet block OUTPUT  --handle |
    // grep 10.0.0.1 | awk '{print $NF}'" 在执行到awk的时候的时候报错。提示略。

    // 目前不使用这个个函数
    namespace bp = boost::process;

    std::vector<std::string> commands;
    std::string command = "";
    for (const auto &c : cmd) {
      if (c == '|') {
        commands.push_back(command);
        command = "";
      } else {
        command += c;
      }
    }
    commands.push_back(command);

    std::vector<bp::pipe> streams(commands.size());
    std::vector<bp::child> processes(commands.size());

    for (std::size_t i = 0; i < commands.size(); ++i) {
      if (i == 0) {
        processes[i] = bp::child(commands[i], bp::std_out > streams[i]);
      } else if (i == commands.size() - 1) {
        processes[i] = bp::child(commands[i], bp::std_in < streams[i - 1]);
      } else {
        processes[i] = bp::child(
            commands[i], bp::std_in<streams[i - 1], bp::std_out> streams[i]);
      }
    }

    int exit_code;
    for (std::size_t i = 0; i < processes.size(); ++i) {
      LOG_TRACE("wait {}", commands[i]);
      processes[i].wait();
      exit_code = processes[i].exit_code();
    }

    std::string output;
    char buffer[4096];
    while (streams[streams.size() - 1].read(buffer, sizeof(buffer))) {
      output.append(buffer, sizeof(buffer));
      LOG_TRACE("output {}", output);
    }
    nlohmann::json ret;
    ret["exit_code"] = exit_code;
    ret["out_str"] = output;
    return ret;
  }

  nlohmann::json exec_rule(std::string cmd) {
    // 使用同步IO读取输出
    boost::process::ipstream out_is;
    boost::process::ipstream err_is;
    LOG_TRACE("{}", cmd);
    int exit_code =
        boost::process::system(cmd, boost::process::std_err > err_is,
                               boost::process::std_out > out_is);
    if (exit_code != 0) { // 并没有规定程序执行成功返回0. echo $?
      LOG_WARN("cmd: {} exit code is {}. out message is: {};{}", cmd, exit_code,
               get_pipe_stream_str(err_is), get_pipe_stream_str(out_is));
    }

    nlohmann::json ret;
    ret["exit_code"] = exit_code;
    ret["out_str"] = get_pipe_stream_str(out_is);
    return ret;
  }

  std::string get_pipe_stream_str(boost::process::ipstream &is) {
    std::stringstream buffer;
    buffer << is.rdbuf();
    return buffer.str();
  }

  int get_handle(const std::string &info, std::string ip) {
    //     table inet block {
    //         chain OUTPUT { # handle 1
    //                 type filter hook output priority filter; policy accept;
    //                 ip daddr 180.101.50.188 drop # handle 2
    //                 ip daddr 180.101.50.242 drop # handle 3
    //                 ip daddr 202.89.233.100 drop # handle 4
    //                 ip daddr 202.89.233.101 drop # handle 5
    //         }
    // }

    // 从上面这样的结构中，根据info，使用C++，提取出handle后面的数字
    int handle_num = -1;
    std::istringstream iss(info);
    std::string line;
    while (std::getline(iss, line)) {
      boost::algorithm::trim(line);
      LOG_TRACE("get handle function line: {}", line);
      if (line.find("ip") != 0)
        continue;
      if (line.find(ip) == std::string::npos)
        continue;
      std::size_t handle_pos = line.find("handle");
      if (handle_pos != std::string::npos) {
        std::string handle_str = line.substr(handle_pos + 7);
        handle_num = std::stoi(handle_str);
        break;
      }
    }
    return handle_num;
  }

private:
  std::set<std::string> have_blocked_ips;
  std::set<std::string> need_block_ips;
  std::set<std::string> need_allow_ips;
};