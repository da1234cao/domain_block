#include "service.hpp"

int main(int argc, char* argv[]) {
    boost::filesystem::path cur_path = boost::dll::program_location().parent_path();
    boost::filesystem::path cfg_path = cur_path / "config.toml";
    std::cout << cfg_path.string() << std::endl;
    // 解析配置文件
    config::instance().parse_toml(cfg_path);

    // 初始化日志配置
    std::string log_file_path = config::instance().get_str("log_file_path");
    std::string log_level = config::instance().get_str("log_level");
    int max_log_files = config::instance().get_int("max_log_files");
    int max_log_file_size = config::instance().get_int("max_log_file_size");
		Log::SPDLOG::getInstance().init(log_file_path, "default_logger", log_level, 
                                    max_log_file_size * 1024 * 1024, max_log_files, false);
    LOG_DEBUG("log has inited.");
    service::start();
    LOG_DEBUG("process end.");
    return 0;
}