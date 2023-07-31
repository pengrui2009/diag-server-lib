/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "src/diagnostic_server_impl.h"

#include <pthread.h>

#include <memory>
#include <string>

#include "common/logger.h"
#include "include/diagnostic_server.h"
#include "parser/json_parser.h"

namespace diag {
namespace server {
// ctor
DiagServerImpl::DiagServerImpl(std::string_view dm_server_config)
    : diag::server::DiagServer{},
      dcm_instance_ptr{},
      dcm_thread_{} {
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "DiagServer instance creation started"; });
  // start parsing the config json file
  boost_support::parser::boostTree ptree{boost_support::parser::JsonParser{}.operator()(dm_server_config)};
  // create single dcm instance and pass the config tree
  dcm_instance_ptr = std::make_unique<diag::server::dcm::DCMServer>(ptree);
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "DiagServer instance creation completed"; });
}

// Initialize all the resources and load the configs
void DiagServerImpl::Initialize() {
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "DiagServer Initialization started"; });
  // start DCM thread here
  dcm_thread_ = std::thread([this]() noexcept { this->dcm_instance_ptr->Main(); });
  pthread_setname_np(dcm_thread_.native_handle(), "DCMServer_Main");
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "DiagServer Initialization completed"; });
}

// De-initialize all the resource and free memory
void DiagServerImpl::DeInitialize() {
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "DiagServer De-Initialization started"; });
  // shutdown DCM module here
  dcm_instance_ptr->SignalShutdown();
  if (dcm_thread_.joinable()) { dcm_thread_.join(); }
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
      __FILE__, __LINE__, __func__, [](std::stringstream &msg) { msg << "DiagServer De-Initialization completed"; });
}

diag::server::conversation::DiagServerConversation &DiagServerImpl::CreateDiagnosticServerConversation(
  uint16_t logical_address) {
  return (dcm_instance_ptr->CreateDiagnosticServerConversation(logical_address));
}

std::pair<diag::server::DiagServer::VehicleResponseResult,
          diag::server::vehicle_info::VehicleInfoMessageResponseUniquePtr>
DiagServerImpl::SendVehicleIdentificationRequest(
    diag::server::vehicle_info::VehicleInfoListRequestType vehicle_info_request) {
  return (dcm_instance_ptr->SendVehicleIdentificationRequest(vehicle_info_request));
}

}  // namespace server
}  // namespace diag
