/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
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
 @ Class Description : Class to create Diagnostic Manager Server functionality                           
 */
DCMServer::DCMServer(diag::server::common::property_tree &ptree)
    : DiagnosticManager{},
      uds_transport_protocol_mgr{std::make_unique<uds_transport::UdsTransportProtocolManager>()}
      // conversation_mgr{std::make_unique<conversation_manager::ConversationManager>(GetDCMServerConfig(ptree),
      //                                                                              *uds_transport_protocol_mgr)},
      {
        config = GetDCMServerConfig(ptree);
        ::uds_transport::conversion_manager::ConversionIdentifierType vd_conversation_config;

        vd_conversation_config.p2_server_max = config.conversation_property.p2_server_max;
        vd_conversation_config.p2_star_server_max = config.conversation_property.p2_star_server_max;
        vd_conversation_config.p4_server_max = config.conversation_property.p4_server_max;
        vd_conversation_config.rx_buffer_size = config.conversation_property.rx_buffer_size;
        vd_conversation_config.logical_address = config.conversation_property.logical_address;
        vd_conversation_config.tcp_address = config.conversation_property.network.tcp_ip_address;
        vd_conversation_config.udp_address = config.udp_ip_address;
        vd_conversation_config.udp_broadcast_address = config.udp_broadcast_address;
        vd_conversation_config.port_num = 13400;
        vd_conversation_config.vin_name = config.conversation_property.vin_name;
        vd_conversation_config.eid_name = config.conversation_property.eid_name;
        vd_conversation_config.gid_name = config.conversation_property.gid_name;

        // create the conversation
        diag_server_vehicle_discovery_conversation = std::make_unique<diag::server::conversation::VdConversation>(
          VehicleDiscoveryConversation, vd_conversation_config);
        // vd_conversation = std::make_unique<diag::server::conversation::VdConversation>(it->first, it->second);
        // Register the connection
        diag_server_vehicle_discovery_conversation->RegisterConnection(
          uds_transport_protocol_mgr->doip_transport_handler->FindOrCreateUdpConnection(
          diag_server_vehicle_discovery_conversation->GetConversationHandler(), config.udp_broadcast_address, 13400, config.udp_ip_address, 13400,
            config.conversation_property.logical_address));

        // 

      }      
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
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "Dcm Server Initialized"; });
}

// Run
void DCMServer::Run() {
  // run udsTransportProtocol layer
  uds_transport_protocol_mgr->Run();
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "Dcm Server is ready to serve"; });
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
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "Dcm Server Shutdown completed"; });
}

void DCMServer::RegisterService(uint8_t sid, std::unique_ptr<ServiceBase> service) {
  for (auto &iter : diag_server_conversations_) {
    iter.second->Register(sid, std::move(service));
  }
}


// Function to get the server Conversation
diag::server::conversation::DiagServerConversation &DCMServer::CreateDiagnosticServerConversation(uint16_t logical_address) {
  std::string diag_server_conversation_name{""};
  ::uds_transport::conversion_manager::ConversionIdentifierType conversation_config;

  if (diag_server_conversations_.count(logical_address)) {
    return *diag_server_conversations_[logical_address];
  } else {
    conversation_config.p2_server_max = config.conversation_property.p2_server_max;
    conversation_config.p2_star_server_max = config.conversation_property.p2_star_server_max;
    conversation_config.p4_server_max = config.conversation_property.p4_server_max;
    conversation_config.rx_buffer_size = config.conversation_property.rx_buffer_size;
    conversation_config.logical_address = logical_address;
    conversation_config.tcp_address = config.conversation_property.network.tcp_ip_address;
    conversation_config.udp_address = config.udp_ip_address;
    conversation_config.udp_broadcast_address = config.udp_broadcast_address;
    conversation_config.port_num = 13400;
    diag::server::conversation::DiagServerConversation *ret_conversation{nullptr};
    // std::unique_ptr<diag::server::conversation::DiagServerConversation> conversation{
    //     conversation_mgr->ListenDiagnosticServerConversation(diag_server_conversation_name)};
    diag_server_conversations_[logical_address] = std::make_unique<diag::server::conversation::DmConversation>(logical_address, conversation_config);
    // std::unique_ptr<diag::server::conversation::DiagServerConversation> conversation{nullptr};
    if (diag_server_conversations_[logical_address]) {
        diag_server_conversations_[logical_address]->RegisterConnection(
          uds_transport_protocol_mgr->doip_transport_handler->FindOrCreateTcpConnection(
            diag_server_conversations_[logical_address]->GetConversationHandler(), config.conversation_property.network.tcp_ip_address, 
              13400, logical_address));

      // diag_server_conversations_[logical_address]->Startup();
      // diag_server_vehicle_discovery_conversation.insert(
      //     std::pair<std::string, std::unique_ptr<diag::server::conversation::DiagServerConversation>>{
      //         conversation_name, std::move(conversation)});
      // ret_conversation = diag_server_conversation_map.at(diag_server_conversation_name).get();
      logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
          __FILE__, __LINE__, __func__, [&](std::stringstream &msg) {
            msg << "Diagnostic Server conversation created with name: " << diag_server_conversation_name;
          });
    } else {
      // no conversation found
      logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogFatal(
          __FILE__, __LINE__, __func__, [&](std::stringstream &msg) {
            msg << "Diagnostic Server conversation not found with name: " << diag_server_conversation_name;
          });
      assert(diag_server_conversations_[logical_address]);
    }
    return *diag_server_conversations_[logical_address];
  }
  
}

// Function to get read from json tree and return the config structure
diag::server::config_parser::DCMServerConfig DCMServer::GetDCMServerConfig(diag::server::common::property_tree &ptree) {
  diag::server::config_parser::DCMServerConfig config{};
  // get the udp info for vehicle discovery
  config.udp_ip_address = ptree.get<std::string>("UdpIpAddress");
  config.udp_broadcast_address = ptree.get<std::string>("UdpBroadcastAddress");
  // get the conversation property
  config.conversation_property.p2_server_max = ptree.get<std::uint16_t>("ConversationProperty.p2ServerMax");
  config.conversation_property.p2_star_server_max = ptree.get<std::uint16_t>("ConversationProperty.p2StarServerMax");
  config.conversation_property.p4_server_max = ptree.get<std::uint16_t>("ConversationProperty.p4ServerMax");
  config.conversation_property.rx_buffer_size = ptree.get<std::uint16_t>("ConversationProperty.RxBufferSize");
  config.conversation_property.logical_address = ptree.get<std::uint16_t>("ConversationProperty.LogicalAddress");
  config.conversation_property.vin_name = ptree.get<std::string>("ConversationProperty.VIN");
  config.conversation_property.eid_name = ptree.get<std::string>("ConversationProperty.EID");
  config.conversation_property.gid_name = ptree.get<std::string>("ConversationProperty.GID");
  config.conversation_property.network.protocol_kind = ptree.get<std::string>("ConversationProperty.Network.ProtocolKind");
  config.conversation_property.network.tcp_ip_address = ptree.get<std::string>("ConversationProperty.Network.TcpIpAddress");
  config.conversation_property.network.tls = ptree.get<bool>("ConversationProperty.Network.TLS");

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
