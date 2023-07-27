/* Diagnostic Server library
 * Copyright (C) 2023  Avijit Dey
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* includes */
#include "src/dcm/dcm_server.h"

#include "src/common/logger.h"

namespace diag {
namespace server {
namespace dcm {

// string representing vehicle discovery conversation name
constexpr std::string_view VehicleDiscoveryConversation{"VehicleDiscovery"};

/*
 @ Class Name        : DCM
 @ Class Description : Class to create Diagnostic Manager Client functionality                           
 */
DCMServer::DCMServer(diag::server::common::property_tree &ptree)
    : DiagnosticManager{},
      uds_transport_protocol_mgr{std::make_unique<uds_transport::UdsTransportProtocolManager>()}
      // conversation_mgr{std::make_unique<conversation_manager::ConversationManager>(GetDCMServerConfig(ptree),
      //                                                                              *uds_transport_protocol_mgr)},
      // diag_server_vehicle_discovery_conversation{
      //     conversation_mgr->GetDiagnosticServerVehicleDiscoveryConversation(VehicleDiscoveryConversation)} {}
      {}
// dtor
DCMServer::~DCMServer() = default;

// Initialize
void DCMServer::Initialize() {
  // start Vehicle Discovery
  diag_server_vehicle_discovery_conversation->Startup();
  // start Conversation Manager
  // conversation_mgr->Startup();
  // start all the udsTransportProtocol Layer
  uds_transport_protocol_mgr->Startup();
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "Dcm Client Initialized"; });
}

// Run
void DCMServer::Run() {
  // run udsTransportProtocol layer
  uds_transport_protocol_mgr->Run();
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "Dcm Client is ready to serve"; });
}

// shutdown DCM
void DCMServer::Shutdown() {
  // shutdown Vehicle Discovery
  diag_server_vehicle_discovery_conversation->Shutdown();
  // shutdown Conversation Manager
  // conversation_mgr->Shutdown();
  // shutdown udsTransportProtocol layer
  uds_transport_protocol_mgr->Shutdown();
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "Dcm Client Shutdown completed"; });
}

// Function to get the client Conversation
diag::server::conversation::DiagServerConversation &DCMServer::ListenDiagnosticServerConversation(
    std::string_view conversation_name) {
  std::string diag_client_conversation_name{conversation_name};
  diag::server::conversation::DiagServerConversation *ret_conversation{nullptr};
  // std::unique_ptr<diag::server::conversation::DiagServerConversation> conversation{
  //     conversation_mgr->ListenDiagnosticServerConversation(diag_client_conversation_name)};
  std::unique_ptr<diag::server::conversation::DiagServerConversation> conversation{nullptr};
  if (conversation) {
    // diag_server_vehicle_discovery_conversation.insert(
    //     std::pair<std::string, std::unique_ptr<diag::server::conversation::DiagServerConversation>>{
    //         conversation_name, std::move(conversation)});
    // ret_conversation = diag_client_conversation_map.at(diag_client_conversation_name).get();
    logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
        __FILE__, __LINE__, __func__, [&](std::stringstream &msg) {
          msg << "Diagnostic Client conversation created with name: " << conversation_name;
        });
  } else {
    // no conversation found
    logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogFatal(
        __FILE__, __LINE__, __func__, [&](std::stringstream &msg) {
          msg << "Diagnostic Client conversation not found with name: " << conversation_name;
        });
    assert(ret_conversation);
  }
  return *ret_conversation;
}

// Function to get read from json tree and return the config structure
diag::server::config_parser::DCMServerConfig DCMServer::GetDCMServerConfig(diag::server::common::property_tree &ptree) {
  diag::server::config_parser::DCMServerConfig config{};
  // get the udp info for vehicle discovery
  config.udp_ip_address = ptree.get<std::string>("UdpIpAddress");
  config.udp_broadcast_address = ptree.get<std::string>("UdpBroadcastAddress");
  // get the conversation property
  // config.p2_server_max = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // config.p2_star_server_max = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // config.p4_server_max = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // config.rx_buffer_size = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // config.logical_address = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // config.vin = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // config.eid = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // config.gid = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // config.network.tcp_ip_address = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // config.network.tcp_ip_address = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // config.network.tcp_ip_address = ptree.get<std::string>("ConversationProperty.p2ServerMax");
  // for (diag::server::common::property_tree::value_type &conversation_ptr:
  //      ptree.get_child("Conversation.ConversationProperty")) {
  //   diag::server::config_parser::ConversationType conversation{};
  //   conversation.conversation_name = conversation_ptr.second.get<std::string>("ConversationName");
  //   conversation.p2_client_max = conversation_ptr.second.get<std::uint16_t>("p2ClientMax");
  //   conversation.p2_star_client_max = conversation_ptr.second.get<std::uint16_t>("p2StarClientMax");
  //   conversation.rx_buffer_size = conversation_ptr.second.get<std::uint16_t>("RxBufferSize");
  //   conversation.source_address = conversation_ptr.second.get<std::uint16_t>("SourceAddress");
  //   conversation.network.tcp_ip_address = conversation_ptr.second.get<std::string>("Network.TcpIpAddress");
  //   config.conversations.emplace_back(conversation);
  // }
  return config;
}

std::pair<diag::server::DiagServer::VehicleResponseResult,
          diag::server::vehicle_info::VehicleInfoMessageResponseUniquePtr>
DCMServer::SendVehicleIdentificationRequest(
    diag::server::vehicle_info::VehicleInfoListRequestType vehicle_info_request) {
  return diag_server_vehicle_discovery_conversation->SendVehicleIdentificationRequest(vehicle_info_request);
}

}  // namespace dcm
}  // namespace server
}  // namespace diag
