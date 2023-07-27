/* Diagnostic Server library
 * Copyright (C) 2023  Avijit Dey
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/** @file create_diagnostic_server.h
 *  @brief Entry function to create Diagnostic Server library
 *  @author Avijit Dey
 */
#ifndef DIAG_SERVER_LIB_APPL_INCLUDE_CREATE_DIAGNOSTIC_SERVER_H_
#define DIAG_SERVER_LIB_APPL_INCLUDE_CREATE_DIAGNOSTIC_SERVER_H_

#include <memory>
#include <string_view>

namespace diag {
namespace server {

// forward declaration
class DiagServer;

/**
 * @brief       Function to get the instance of Diagnostic Client Object.
 *              This instance to be further used for all the functionalities.
 * @param[in]   diag_client_config_path
 *              path to diag client config file
 * @return      std::unique_ptr<diag::server::DiagServer>
 *              Unique pointer to diag client object
 * @remarks     Implemented requirements:
 *              DiagServerLib-Library-Support, DiagServerLib-ComParam-Settings
 */
std::unique_ptr<diag::server::DiagServer> CreateDiagnosticServer(std::string_view diag_client_config_path);

}  // namespace server
}  // namespace diag

#endif  // DIAG_SERVER_LIB_APPL_INCLUDE_CREATE_DIAGNOSTIC_SERVER_H_
