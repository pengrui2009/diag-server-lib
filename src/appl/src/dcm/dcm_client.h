/* Diagnostic Server library
 * Copyright (C) 2023  Avijit Dey
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
#include "src/dcm/conversation/conversation_manager.h"

namespace diag {
namespace server {
namespace dcm {
/*
 @ Class Name        : DCM Client
 @ Class Description : Class to create Diagnostic Manager Client functionality                           
 */
class DCMClient final : public diag::server::common::DiagnosticManager {
public:
  //ctor
  explicit DCMClient(diag::server::common::property_tree &ptree);

  //dtor
  ~DCMClient() override;

  // Initialize
  void Initialize() override;

  // Run
  void Run() override;

  // Shutdown
  void Shutdown() override;

  // Function to get the diagnostic client conversation
  diag::server::conversation::DiagClientConversation &GetDiagnosticClientConversation(
      std::string_view conversation_name) override;

  // Send Vehicle Identification Request and get response
  std::pair<diag::server::DiagClient::VehicleResponseResult,
            diag::server::vehicle_info::VehicleInfoMessageResponseUniquePtr>
  SendVehicleIdentificationRequest(
      diag::server::vehicle_info::VehicleInfoListRequestType vehicle_info_request) override;

private:
  // uds transport protocol Manager
  std::unique_ptr<uds_transport::UdsTransportProtocolManager> uds_transport_protocol_mgr;

  // conversation manager
  std::unique_ptr<conversation_manager::ConversationManager> conversation_mgr;

  // map to store conversation pointer along with conversation name
  std::unordered_map<std::string, std::unique_ptr<diag::server::conversation::DiagClientConversation>>
      diag_client_conversation_map;

  // store the diag client conversation for vehicle discovery
  std::unique_ptr<conversation::VdConversation> diag_client_vehicle_discovery_conversation;

  // function to read from property tree to config structure
  static diag::server::config_parser::DcmClientConfig GetDcmClientConfig(diag::server::common::property_tree &ptree);
};
}  // namespace dcm
}  // namespace server
}  // namespace diag
#endif  // DIAGNOSTIC_SERVER_LIB_APPL_SRC_DIAGNOSTIC_COMMUNICATION_MANAGER_H
