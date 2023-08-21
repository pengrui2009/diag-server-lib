/* Diagnostic Server library
* Copyright (C) 2023  Rui Peng
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_SOCKETS_TCP_SOCKET_HANDLER_H_
#define DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_SOCKETS_TCP_SOCKET_HANDLER_H_

#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "socket/tcp/tcp_server.h"

namespace doip_handler {

namespace tcpSocket {

using TcpMessage = boost_support::socket::tcp::TcpMessageType;
using TcpMessagePtr = boost_support::socket::tcp::TcpMessagePtr;
using TcpMessageConstPtr = boost_support::socket::tcp::TcpMessageConstPtr;

class DoipTcpSocketHandler {
public:
  using TcpSocket = boost_support::socket::tcp::CreateTcpServerSocket;
  using TcpConnection = boost_support::socket::tcp::CreateTcpServerSocket::TcpServerConnection;
  using TcpHandlerRead = boost_support::socket::tcp::CreateTcpServerSocket::TcpHandlerRead;

  class TcpConnectionHandler {
  public:
    explicit TcpConnectionHandler(std::unique_ptr<TcpConnection> tcp_connection);

    ~TcpConnectionHandler();

    // start the reception thread
    void Initialize();

    // stop the reception thread
    void DeInitialize();

    bool GetConnectionState() {
      return connection_state_.load();
    }

    // function to trigger transmission
    // true on success else false
    bool Transmit(TcpMessageConstPtr tcp_tx_message);

    // get connection state: 1:connect or 0: disconnect
    bool GetConnectionState();
    
  private:
    // read handler
    TcpHandlerRead tcp_handler_read_;

    // store connection
    std::unique_ptr<TcpConnection> tcp_connection_;

    // flag to the connection of tcp client
    std::atomic_bool connection_state_;

    // flag to terminate the thread
    std::atomic_bool exit_request_;

    // flag th start the thread
    std::atomic_bool running_;

    // conditional variable to block the thread
    std::condition_variable cond_var_;

    // threading var
    std::thread thread_;

    // locking critical section
    std::mutex mutex_;
  };

public:
  // ctor
  DoipTcpSocketHandler(std::string_view local_ip_address, uint16_t port_num);

  // dtor
  ~DoipTcpSocketHandler() = default;

  // function to create tcp connection
  std::unique_ptr<TcpConnectionHandler> CreateTcpConnection(TcpHandlerRead &&tcp_handler_read);

private:
  // local Ip address
  std::string local_ip_address_;

  // local port number
  uint16_t port_num_;

  // tcp socket
  std::unique_ptr<TcpSocket> tcp_socket_;

};

}  // namespace tcpSocket
}  // namespace doip_handler

#endif  //DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_SOCKETS_TCP_SOCKET_HANDLER_H_
