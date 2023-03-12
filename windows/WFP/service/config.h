#pragma once

#include <toml++/toml.h>
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <map>

class config {
private:
  std::map<std::string, std::string> m_cfg;
private:
  config() = default;
public:
  config(const config&) = delete;
  config& operator=(const config&) = delete;
  static config& instance();
  void parse_toml(boost::filesystem::path &cfg_path);
  std::string get_str(const std::string name);
  int get_int(const std::string name);
};