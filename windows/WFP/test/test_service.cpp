#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

void write_log(char *format, ...) {
  va_list va;
  va_start(va, format);
  char buffer[256]; // 小心缓冲区溢出
  vsprintf(buffer, format, va);
  va_end(va);
  std::cout << buffer << std::endl;
  std::ofstream outfile;
  outfile.open("C:\\test_service.txt", std::ios::out | std::ios::app);
  outfile << std::string(buffer) << std::endl;
  return;
}

int main(int argc, char *argv) {
  try {
    write_log("begin service test");

    nlohmann::json rules = nlohmann::json::array();
    nlohmann::json rule_one;
    rule_one["action"] = "block";
    // rule_one["action"] = "allow";
    rule_one["domain"] = "www.baidu.com";
    rule_one["ip"] = {"180.101.50.188", "180.101.50.242"};
    rules.push_back(rule_one);

    nlohmann::json json;
    json["name"] = "dacao";
    json["description"] = "Block the following networks";
    json["rules"] = rules;

    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket(io_context);
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::endpoint endpoint =
        *resolver.resolve("127.0.0.1", "6666").begin();

    socket.connect(endpoint);

    std::string send_rules = json.dump();
    write_log("send rules: %s", send_rules.c_str());

    socket.send(boost::asio::buffer(send_rules));
    socket.close();
  } catch (std::exception &e) {
    write_log("exception: %s", e.what());
  }
  return 0;
}