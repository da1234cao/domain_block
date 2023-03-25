#pragma once
#include "connection.h"

#include <boost/asio.hpp>

template <typename T> class server;

typedef std::shared_ptr<server<connection>> server_ptr;

template <typename T = connection> class server {
public:
  server(const std::string &address, const std::string &port)
      : m_io_context(1), m_acceptor(m_io_context), m_connection_manager(),
        m_signals(m_io_context) {
    // 在win下，使用taskkill发送信号，会让进程直接退出，并没有执行这里的信号处理。
    // 目前不清楚，可参考：https://stackoverflow.com/questions/26404907/gracefully-terminate-a-boost-asio-based-windows-console-application
    m_signals.add(SIGINT);
    m_signals.add(SIGTERM);
    m_signals.async_wait([this](boost::system::error_code ec, int signo) {
      if (signo == SIGINT || signo == SIGTERM) {
        LOG_DEBUG("server receive signo: {}", signo);
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

  void run() { m_io_context.run(); }
  void stop() {
    // 服务器停止是通过取消所有未完成的异步操作来实现的。
    // 一旦所有操作都完成，io_context::run() 函数将退出。
    m_acceptor.close();
    m_connection_manager.stop_all();
  }

private:
  void do_accept() {
    // Move accept handler requirements
    m_acceptor.async_accept([this](boost::system::error_code ec,
                                   boost::asio::ip::tcp::socket socket) {
      // Check whether the server was stopped by a signal before this
      // completion handler had a chance to run.
      if (!m_acceptor.is_open()) {
        return;
      }
      if (!ec) {
        m_connection_manager.start(
            std::make_shared<T>(std::move(socket), m_connection_manager));
      }

      do_accept();
    });
  }

private:
  boost::asio::io_context m_io_context;
  boost::asio::ip::tcp::acceptor m_acceptor;
  connection_manager m_connection_manager;
  boost::asio::signal_set m_signals;
};