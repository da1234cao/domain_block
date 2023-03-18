#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#ifdef WIN32
#include <Ws2tcpip.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

namespace common {
inline bool is_ipv4(const std::string &ip) {
  boost::asio::ip::address addr = boost::asio::ip::address::from_string(ip);
  return addr.is_v4();
}

inline bool is_ipv6(const std::string &ip) {
  boost::asio::ip::address addr = boost::asio::ip::address::from_string(ip);
  return addr.is_v6();
}

inline bool is_ip(const std::string &ip) { return is_ipv4(ip) || is_ipv6(ip); }

inline std::string addr_to_string(const in_addr &addr) {
  char addr_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &addr, addr_str, INET_ADDRSTRLEN);
  return std::string(addr_str);
}

inline std::string addr_to_string(const in6_addr &addr) {
  char addr_str[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &addr, addr_str, INET6_ADDRSTRLEN);
  return std::string(addr_str);
}
} // namespace common