#include "server.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind/bind.hpp>
#include <fstream>
#include <iostream>

server::server(const std::string &address, const std::string &port)
    : m_io_context(1), m_acceptor(m_io_context), m_connection_manager(),
      m_signals(m_io_context) {
  // 在win下，使用taskkill发送信号，会让进程直接退出，并没有执行这里的信号处理。
  // 目前不清楚，可参考：https://stackoverflow.com/questions/26404907/gracefully-terminate-a-boost-asio-based-windows-console-application
  m_signals.add(SIGINT);
  m_signals.add(SIGTERM);
  m_signals.async_wait([this](boost::system::error_code ec, int signo) {
    if (signo == SIGINT || signo == SIGTERM) {
      stop();
    }
  });

  boost::asio::ip::tcp::resolver resolver(m_io_context);
  boost::asio::ip::tcp::endpoint endpoint =
      *resolver.resolve(address, port).begin();
  m_acceptor.open(endpoint.protocol());
  m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  m_acceptor.bind(endpoint);
  m_acceptor.listen();

  do_accept();
}

void server::do_accept() {
  // Move accept handler requirements
  m_acceptor.async_accept([this](boost::system::error_code ec,
                                 boost::asio::ip::tcp::socket socket) {
    // Check whether the server was stopped by a signal before this
    // completion handler had a chance to run.
    if (!m_acceptor.is_open()) {
      return;
    }
    if (!ec) {
      m_connection_manager.start(std::make_shared<connection>(
          std::move(socket), m_connection_manager));
    }

    do_accept();
  });
}

void server::run() { m_io_context.run(); }

void server::stop() {
  // 服务器停止是通过取消所有未完成的异步操作来实现的。
  // 一旦所有操作都完成，io_context::run() 函数将退出。
  m_acceptor.close();
  m_connection_manager.stop_all();
}