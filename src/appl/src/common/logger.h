/* Diagnostic Server library
 * Copyright (C) 2023  Avijit Dey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef DIAGNOSTIC_SERVER_LIB_APPL_SRC_COMMON_LOGGER_H
#define DIAGNOSTIC_SERVER_LIB_APPL_SRC_COMMON_LOGGER_H

#include "utility/logger.h"

namespace diag {
namespace server {
namespace logger {
using Logger = utility::logger::Logger;

class DiagServerLogger {
public:
  auto static GetDiagServerLogger() noexcept -> DiagServerLogger& {
    static DiagServerLogger diag_client_logger_;
    return diag_client_logger_;
  }

  auto GetLogger() noexcept -> Logger& { return logger_; }

private:
  DiagServerLogger() = default;

  // actual logger context
  Logger logger_{"dcap"};
};
}  // namespace logger
}  // namespace server
}  // namespace diag
#endif
