/* Diagnostic Server library
 * Copyright (C) 2023  Avijit Dey
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef DIAGNOSTIC_SERVER_LIB_APPL_SRC_DCM_CONVERSATION_CONVERSATION_MANAGER_H
#define DIAGNOSTIC_SERVER_LIB_APPL_SRC_DCM_CONVERSATION_CONVERSATION_MANAGER_H
/* includes */
#include <string_view>

#include "common_header.h"
#include "src/dcm/config_parser/config_parser_type.h"
#include "src/dcm/connection/uds_transport_protocol_manager.h"
#include "src/dcm/conversation/dm_conversation.h"
#include "src/dcm/conversation/vd_conversation.h"

namespace diag {
namespace server {
namespace conversation_manager {
/*
 @ Class Name        : ConversationManager
 @ Class Description : Class to manage all the conversion created from usr request                           
 */
class ConversationManager {
public:
  // ctor
  ConversationManager(diag::server::config_parser::DcmClientConfig config,
                      diag::server::uds_transport::UdsTransportProtocolManager &uds_transport_mgr);

  // dtor
  ~ConversationManager() = default;

  // startup
  void Startup();

  // shutdown
  void Shutdown();

  // Get the required conversion
  std::unique_ptr<diag::server::conversation::DmConversation> GetDiagnosticClientConversation(
      std::string_view conversion_name);

  // Get the required conversion
  std::unique_ptr<diag::server::conversation::VdConversation> GetDiagnosticClientVehicleDiscoveryConversation(
      std::string_view conversion_name);

private:
  // store uds transport manager
  uds_transport::UdsTransportProtocolManager &uds_transport_mgr_;

  // store the conversion name with conversion configurations
  std::map<std::string, ::uds_transport::conversion_manager::ConversionIdentifierType> conversation_config_;

  // store the vehicle discovery with conversation configuration
  std::map<std::string, ::uds_transport::conversion_manager::ConversionIdentifierType> vd_conversation_config_;

  // function to create or find new conversion
  void CreateConversationConfig(diag::server::config_parser::DcmClientConfig &config);
};
}  // namespace conversation_manager
}  // namespace server
}  // namespace diag
#endif  // DIAGNOSTIC_SERVER_LIB_APPL_SRC_DCM_CONVERSATION_CONVERSATION_MANAGER_H
