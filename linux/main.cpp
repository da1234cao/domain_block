#include "config.h"
#include "nf_connection.hpp"
#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

int main(int argc, char *agrv[]) {
  boost::filesystem::path cur_path =
      boost::dll::program_location().parent_path();
  boost::filesystem::path cfg_path = cur_path / "domain_block_service.toml";

  // 解析配置文件
  config::instance().parse_toml(cfg_path);

  // 初始化日志配置
  std::string log_file_path = config::instance().get_str("log_file_path");
  std::string log_level = config::instance().get_str("log_level");
  int max_log_files = config::instance().get_int("max_log_files");
  int max_log_file_size = config::instance().get_int("max_log_file_size");
  Log::SPDLOG::getInstance().init(log_file_path, "default_logger", log_level,
                                  max_log_file_size * 1024 * 1024,
                                  max_log_files, false);
  LOG_DEBUG("log has inited.");

  // 创建server对象
  server<nf_connection> s(config::instance().get_str("ip"),
                          std::to_string(config::instance().get_int("port")));
  nf_help::instance(); // 显示的在server启动前创建nftalbe的表和链
  s.run();

  nf_help::instance().flush_all();
  LOG_DEBUG("process end.");
  return 0;
}