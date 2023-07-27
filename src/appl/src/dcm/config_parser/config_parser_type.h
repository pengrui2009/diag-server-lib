/* Diagnostic Server library
 * Copyright (C) 2023  Avijit Dey
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
  // local tcp address
  std::string tcp_ip_address;
};

// Properties of a single conversation
struct ConversationType {
  // store p2 client timeout
  std::uint16_t p2_client_max;
  // store p2 star client timeout
  std::uint16_t p2_star_client_max;
  // store receive buffer size
  std::uint16_t rx_buffer_size;
  // store source address of client
  std::uint16_t source_address;
  // store the client conversation name
  std::string conversation_name;
  // store the doip network item
  DoipNetworkType network;
};

// Properties of diag client configuration
struct DCMServerConfig {
  // local udp address
  std::string udp_ip_address;
  // broadcast address
  std::string udp_broadcast_address;
  // conversation property
  // store all conversations
  std::vector<ConversationType> conversations;
};
}  // namespace config_parser
}  // namespace server
}  // namespace diag
#endif  //  DIAGNOSTIC_SERVER_LIB_APPL_SRC_DCM_CONFIG_PARSER_CONFIG_PARSER_TYPE_H