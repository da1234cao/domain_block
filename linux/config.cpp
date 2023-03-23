#include "config.h"
#include <iostream>

config &config::instance() {
  static config inst;
  return inst;
}

void config::parse_toml(boost::filesystem::path &cfg_path) {
  toml::table tlbs = toml::parse_file(cfg_path.string());
  // 日志文件
  m_cfg["log_file_path"] = tlbs["log"]["path"].value<std::string>().value();
  m_cfg["log_level"] = tlbs["log"]["level"].value<std::string>().value();
  m_cfg["max_log_file_size"] =
      tlbs["log"]["max_file_size"].value<std::string>().value();
  m_cfg["max_log_files"] =
      tlbs["log"]["max_files"].value<std::string>().value();
  // 服务器配置
  m_cfg["ip"] = tlbs["server"]["ip"].value<std::string>().value();
  m_cfg["port"] = tlbs["server"]["port"].value<std::string>().value();
}

std::string config::get_str(const std::string name) { return m_cfg[name]; }

int config::get_int(const std::string name) { return std::stoi(m_cfg[name]); }