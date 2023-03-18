#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <set>

class connection;
typedef std::shared_ptr<connection> connection_ptr;

class connection_manager {
public:
  connection_manager() = default;
  connection_manager(const connection_manager &) = delete;
  connection_manager &operator=(const connection_manager &) = delete;

  void start(connection_ptr c);
  void stop(connection_ptr c);
  void stop_all();

private:
  std::set<connection_ptr> m_connections;
};

class connection : public std::enable_shared_from_this<connection> {
public:
  connection(const connection &) = delete;
  connection &operator=(const connection &) = delete;

  connection(boost::asio::ip::tcp::socket socket, connection_manager &manager);

  void start();

  void stop();

protected:
  virtual void do_read();
  virtual void do_write();
  virtual void handle_read(const boost::system::error_code &ec,
                           size_t bytes_transferred);
  virtual void handle_write(const boost::system::error_code &ec,
                            size_t bytes_transferred);

protected:
  boost::asio::ip::tcp::socket m_socket;
  int m_write_size = 0;
  std::array<char, 4096> m_read_buffer;
  std::array<char, 4096> m_write_buffer;
  connection_manager &m_connection_manager;
};