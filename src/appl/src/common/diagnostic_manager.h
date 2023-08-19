/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef DIAGNOSTIC_SERVER_LIB_APPL_SRC_COMMON_DIAGNOSTIC_MANAGER_H
#define DIAGNOSTIC_SERVER_LIB_APPL_SRC_COMMON_DIAGNOSTIC_MANAGER_H
/* includes */
#include <string_view>

#include "common_header.h"
#include "include/diagnostic_server.h"
#include "include/diagnostic_server_uds_message_type.h"
#include "include/diagnostic_server_vehicle_info_message_type.h"
#include "parser/json_parser.h"

namespace diag {
namespace server {
// forward declaration
namespace conversation {
class DiagServerConversation;
}

namespace common {
using property_tree = boost_support::parser::boostTree;

/*
 @ Class Name        : DiagnosticManager
 @ Class Description : Parent class to create DCM and DEM class                            
 */
class DiagnosticManager {
public:
  //ctor
  DiagnosticManager();

  // dtor
  virtual ~DiagnosticManager();

  // main function
  virtual void Main();

  // signal shutdown
  virtual void SignalShutdown();

  // Initialize
  virtual void Initialize() = 0;

  // Run
  virtual void Run() = 0;

  // Shutdown
  virtual void Shutdown() = 0;

  // Function to get the diagnostic server conversation
  virtual diag::server::conversation::DiagServerConversation &CreateDiagnosticServerConversation(uint16_t logical_address) = 0;

  // Send Vehicle Identification Request and get response
  virtual std::pair<diag::server::DiagServer::VehicleResponseResult,
                    diag::server::vehicle_info::VehicleInfoMessageResponseUniquePtr>
  SendVehicleIdentificationRequest(diag::server::vehicle_info::VehicleInfoListRequestType vehicle_info_request) = 0;

private:
  // flag to terminate the main thread
  bool exit_requested_;
  // conditional variable to block the thread
  std::condition_variable cond_var;
  // For locking critical section of code
  std::mutex mutex_;
};
}  // namespace common
}  // namespace server
}  // namespace diag
#endif  // DIAGNOSTIC_SERVER_LIB_APPL_SRC_COMMON_DIAGNOSTIC_MANAGER_H