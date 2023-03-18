#pragma once

#include "server/connection.h"
#include "server/server.hpp"
#include "wfp_help.hpp"

class wfp_connection;

typedef std::shared_ptr<server<wfp_connection>> wfp_server_ptr;

class wfp_connection : public connection {
public:
  wfp_connection(boost::asio::ip::tcp::socket socket,
                 connection_manager &manager)
      : connection(std::move(socket), manager) {}

protected:
  void handle_read(const boost::system::error_code &ec,
                   size_t bytes_transferred) override {
    if (!ec) {
      // 检查是否是一个完整的json
      try {
        std::string recv(m_read_buffer.begin(),
                         m_read_buffer.begin() + bytes_transferred);
        nlohmann::json json = nlohmann::json::parse(recv);
        std::string s = json.dump();
        LOG_TRACE("recv: {}", s);
        wfp_help::instance().parse_rules(s);
        wfp_help::instance().send_all_rules();
        std::copy(s.c_str(), s.c_str() + s.size(), m_write_buffer.begin());
        m_write_size = s.size();
        do_write();
      } catch (nlohmann::json::exception &e) {
        LOG_WARN("warning: {}", e.what());
      }
    } else if (ec != boost::asio::error::operation_aborted) {
      m_connection_manager.stop(shared_from_this());
    }
  }
};