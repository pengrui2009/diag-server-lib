/* Diagnostic Server library
 * Copyright (C) 2023  Avijit Dey
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "include/diagnostic_server.h"

#include "src/diagnostic_server_impl.h"

namespace diag {
namespace server {
std::unique_ptr<diag::server::DiagServer> CreateDiagnosticServer(std::string_view diag_client_config_path) {
  return (std::make_unique<diag::server::DiagServerImpl>(diag_client_config_path));
}
}  // namespace server
}  // namespace diag
