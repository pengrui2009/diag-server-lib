/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef DIAGNOSTIC_SERVER_LIB_APPL_SRC_DIAGNOSTIC_COMMUNICATION_MANAGER_H
#define DIAGNOSTIC_SERVER_LIB_APPL_SRC_DIAGNOSTIC_COMMUNICATION_MANAGER_H
/* includes */
#include <string_view>

#include "src/common/diagnostic_manager.h"
#include "src/dcm/config_parser/config_parser_type.h"
#include "src/dcm/connection/uds_transport_protocol_manager.h"
#include "src/dcm/conversation/dm_conversation.h"
#include "src/dcm/conversation/vd_conversation.h"
// #include "src/dcm/conversation/conversation_manager.h"

namespace diag {
namespace server {
namespace dcm {
/*
 @ Class Name        : DCM server
 @ Class Description : Class to create Diagnostic Manager server functionality                           
 */
class DCMServer final : public diag::server::common::DiagnosticManager {
public:
  //ctor
  explicit DCMServer(diag::server::common::property_tree &ptree);

  //dtor
  ~DCMServer() override;

  // Initialize
  void Initialize() override;

  // Run
  void Run() override;

  // Shutdown
  void Shutdown() override;

  void RegisterService(uint8_t sid, std::unique_ptr<ServiceBase> );

  // Function to get the diagnostic server conversation
  diag::server::conversation::DiagServerConversation &CreateDiagnosticServerConversation(uint16_t logical_address) override;

  // Send Vehicle Identification Request and get response
  std::pair<diag::server::DiagServer::VehicleResponseResult,
            diag::server::vehicle_info::VehicleInfoMessageResponseUniquePtr>
  SendVehicleIdentificationRequest(
      diag::server::vehicle_info::VehicleInfoListRequestType vehicle_info_request) override;

private:
  diag::server::config_parser::DCMServerConfig config;
  // uds transport protocol Manager
  std::unique_ptr<uds_transport::UdsTransportProtocolManager> uds_transport_protocol_mgr;

  // conversation manager
  // std::unique_ptr<conversation_manager::ConversationManager> conversation_mgr;

  // map to store conversation pointer along with conversation name
  // std::unordered_map<std::string, std::unique_ptr<diag::server::conversation::DiagServerConversation>>
  //     diag_server_conversation_map;
  std::unordered_map<uint16_t, std::unique_ptr<diag::server::conversation::DiagServerConversation>> diag_server_conversations_;

  // store the diag server conversation for vehicle discovery
  std::unique_ptr<diag::server::conversation::VdConversation> diag_server_vehicle_discovery_conversation;

  // function to read from property tree to config structure
  static diag::server::config_parser::DCMServerConfig GetDCMServerConfig(diag::server::common::property_tree &ptree);
};
}  // namespace dcm
}  // namespace server
}  // namespace diag
#endif  // DIAGNOSTIC_SERVER_LIB_APPL_SRC_DIAGNOSTIC_COMMUNICATION_MANAGER_H
