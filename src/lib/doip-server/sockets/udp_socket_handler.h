/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_SOCKETS_UDP_SOCKET_HANDLER_H_
#define DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_SOCKETS_UDP_SOCKET_HANDLER_H_

//includes
#include <functional>
#include <string>
#include <string_view>

#include "socket/udp/udp_client.h"

namespace doip_handler {
namespace udpSocket {

// typedefs
using UdpSocket = boost_support::socket::udp::createUdpClientSocket;
using UdpMessage = boost_support::socket::udp::UdpMessageType;
using UdpMessagePtr = boost_support::socket::udp::UdpMessagePtr;
using UdpMessageConstPtr = boost_support::socket::udp::UdpMessageConstPtr;
using kDoip_String = std::string_view;
using UdpMessageFunctor = std::function<void(UdpMessagePtr)>;

/*
 @ Class Name        : DoipUdpSocketHandler
 @ Class Description : Class used to create a tcp socket for handling transmission
                       and reception of tcp message from driver
 */
class DoipUdpSocketHandler {
public:
  // Port Type
  using PortType = boost_support::socket::udp::createUdpClientSocket::PortType;

public:
  //ctor
  DoipUdpSocketHandler(kDoip_String &udp_ip_address, uint16_t udp_port_num, PortType port_type,
                       UdpMessageFunctor udp_handler);

  //dtor
  ~DoipUdpSocketHandler() = default;

  //start
  void Start();

  //stop
  void Stop();

  // function to trigger transmission
  bool Transmit(UdpMessageConstPtr udp_tx_message);

private:
  // local Ip address
  kDoip_String local_ip_address_;

  // local port number
  uint16_t local_port_num_;

  // Port type
  UdpSocket::PortType port_type_;

  // udp socket
  std::unique_ptr<UdpSocket> udp_socket_;
};

}  // namespace udpSocket
}  // namespace doip_handler

#endif  // DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_SOCKETS_TCP_SOCKET_HANDLER_H_
