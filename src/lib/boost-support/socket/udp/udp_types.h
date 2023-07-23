/* Diagnostic Server library
 * Copyright (C) 2023  Avijit Dey
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef DIAGNOSTIC_SERVER_LIB_LIB_BOOST_SUPPORT_SOCKET_UDP_UDP_TYPES_H
#define DIAGNOSTIC_SERVER_LIB_LIB_BOOST_SUPPORT_SOCKET_UDP_UDP_TYPES_H

namespace boost_support {
namespace socket {
namespace udp {

class UdpMessageType {
public:
  // buffer type
  using buffType = std::vector<uint8_t>;
  // ip address type
  using ipAddressType = std::string;

public:
  // ctor
  UdpMessageType() = default;

  // dtor
  virtual ~UdpMessageType() = default;

  // Receive buffer
  buffType rx_buffer_;
  // Transmit buffer
  buffType tx_buffer_;
  // host ipaddress
  ipAddressType host_ip_address_;
  // host port num
  uint16_t host_port_num_;
};

// unique pointer to const UdpMessage
using UdpMessageConstPtr = std::unique_ptr<const UdpMessageType>;
// unique pointer to UdpMessage
using UdpMessagePtr = std::unique_ptr<UdpMessageType>;
// Response size udp
constexpr uint8_t kDoipUdpResSize = 40U;
}  // namespace udp
}  // namespace socket
}  // namespace boost_support
#endif  // DIAGNOSTIC_SERVER_LIB_LIB_BOOST_SUPPORT_SOCKET_UDP_UDP_TYPES_H
