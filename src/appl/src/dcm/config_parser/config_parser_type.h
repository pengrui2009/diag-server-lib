/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef DIAGNOSTIC_SERVER_LIB_APPL_SRC_DCM_CONFIG_PARSER_CONFIG_PARSER_TYPE_H
#define DIAGNOSTIC_SERVER_LIB_APPL_SRC_DCM_CONFIG_PARSER_CONFIG_PARSER_TYPE_H
/* includes */
#include "common_header.h"

namespace diag {
namespace server {
namespace config_parser {

// Doip network property type
struct DoipNetworkType {
  // protocol kind
  std::string protocol_kind;
  // local tcp address
  std::string tcp_ip_address;
  // tls
  bool tls;
};

// Properties of a single conversation
struct ConversationType {
  // store p2 server timeout
  std::uint16_t p2_server_max;
  // store p2 star server timeout
  std::uint16_t p2_star_server_max;
  // store p4 server timeout
  std::uint16_t p4_server_max;
  // store receive buffer size
  std::uint16_t rx_buffer_size;
  // store logical address of server
  std::uint16_t logical_address;
  // store the server VIN name
  std::string vin_name;
  // store the server eid name
  std::string eid_name;  
  // store the server gid name
  std::string gid_name;
  // store the doip network item
  DoipNetworkType network;
};

// Properties of diag server configuration
struct DCMServerConfig {
  // local udp address
  std::string udp_ip_address;
  // broadcast address
  std::string udp_broadcast_address;
  // conversation property
  ConversationType conversation_property;
};
}  // namespace config_parser
}  // namespace server
}  // namespace diag
#endif  //  DIAGNOSTIC_SERVER_LIB_APPL_SRC_DCM_CONFIG_PARSER_CONFIG_PARSER_TYPE_H