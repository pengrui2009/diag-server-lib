/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_DOIP_PAYLOAD_TYPES_H_
#define DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_DOIP_PAYLOAD_TYPES_H_

#include <cstdint>
#include <string>
#include <vector>

/* Magic numbers */
constexpr uint8_t BYTE_POS_ZERO = 0x00;
constexpr uint8_t BYTE_POS_ONE = 0x01;
constexpr uint8_t BYTE_POS_TWO = 0x02;
constexpr uint8_t BYTE_POS_THREE = 0x03;
constexpr uint8_t BYTE_POS_FOUR = 0x04;
constexpr uint8_t BYTE_POS_FIVE = 0x05;
constexpr uint8_t BYTE_POS_SIX = 0x06;
constexpr uint8_t BYTE_POS_SEVEN = 0x07;

class DoipMessage {
public:
  enum class rx_socket_type : std::uint8_t { kBroadcast, kUnicast };
  // ip address type
  using IpAddressType = std::string;

public:
  // ctor
  DoipMessage() = default;

  DoipMessage(const DoipMessage &other) = default;

  DoipMessage(DoipMessage &&other) noexcept = default;

  DoipMessage &operator=(const DoipMessage &other) = default;

  DoipMessage &operator=(DoipMessage &&other) noexcept = default;

  // rx type -> broadcast, unicast
  rx_socket_type rx_socket{rx_socket_type::kUnicast};
  // remote ip address;
  IpAddressType host_ip_address;
  // remote port
  std::uint16_t port_num;
  // doip protocol version
  std::uint8_t protocol_version{};
  // doip protocol inverse version
  std::uint8_t protocol_version_inv{};
  // doip payload type
  std::uint16_t payload_type{};
  // doip payload length
  std::uint32_t payload_length{};
  // doip payload
  std::vector<std::uint8_t> payload;
};

#endif  // DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_DOIP_PAYLOAD_TYPES_H_
