#pragma once
#include <boost/asio.hpp>

#include "connection.h"

class server {
public:
  server(const std::string &address, const std::string &port);
  void run();
  void stop();

private:
  void do_accept();

private:
  boost::asio::io_context m_io_context;
  boost::asio::ip::tcp::acceptor m_acceptor;
  connection_manager m_connection_manager;
  boost::asio::signal_set m_signals;
};