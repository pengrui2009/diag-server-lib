 
#include <boost/asio.hpp>
#include <iostream>
 
const std::string c_multicast_addr = "239.255.0.1";
const std::string c_listen_addr = "0.0.0.0";
const int c_multicast_port = 13400;
 
class UDPMulticastSocket
{
public:
  UDPMulticastSocket(boost::asio::io_service& io_service)
      : socket_(io_service), timer_(io_service), endpoint_(boost::asio::ip::address_v4::from_string(c_multicast_addr), c_multicast_port)
  {
    socket_.open(endpoint_.protocol());
    socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket_.bind(endpoint_);
    socket_.set_option(boost::asio::ip::multicast::join_group(
        boost::asio::ip::address_v4::from_string(c_multicast_addr)
        , boost::asio::ip::address_v4::from_string(c_listen_addr)));
    do_receive();
    // do_send();
  }
 
  ~UDPMulticastSocket()
  {
    socket_.close();
  }
 
private:
  void do_receive()
  {
    boost::asio::ip::udp::endpoint sender_endpoint;
    socket_.async_receive_from(
        boost::asio::buffer(buff_), sender_endpoint,
        [this](const boost::system::error_code ec, std::size_t length) {
          if (!ec)
          {
            std::cout <<"receive:";
            std::cout.write(buff_.data(), length);
            std::cout << std::endl;
 
            do_receive();
          }
        });
  }
  void do_send()
  {
    static int count = 0;
    std::string msg = "msg " + std::to_string(count++);
    std::cout << "send: " << msg << std::endl;
    boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address_v4::from_string("239.255.0.1"), 13400);
    socket_.async_send_to(
        boost::asio::buffer(msg), endpoint,
        [this](const boost::system::error_code &ec, std::size_t /*length*/) {
          timer_.expires_from_now(boost::posix_time::seconds(1));
          timer_.async_wait(
              [this](const boost::system::error_code &error) {
                if (error)
                {
                  std::cout << "error:" << error.message() << std::endl;
                }
                else
                {
                  this->do_send();
                }
              });
        });
  }
 
private:
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint endpoint_;
  std::array<char, 1024> buff_;
  boost::asio::deadline_timer timer_;
};
 
int main(int argc, char *argv[])
{
  try
  {
    boost::asio::io_service io_service;
    UDPMulticastSocket sock(io_service);
    io_service.run();
  }
  catch (...)
  {
  std:;
    std::cout << "error occurred!!!" << std::endl;
  }
  return 0;
}