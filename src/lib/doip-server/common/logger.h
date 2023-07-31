/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_LOGGER_H_
#define DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_LOGGER_H_

#include "utility/logger.h"

namespace doip_handler {
namespace logger {
using Logger = utility::logger::Logger;

class DoipServerLogger {
public:
  auto static GetDiagServerLogger() noexcept -> DoipServerLogger& {
    static DoipServerLogger lib_boost_logger_;
    return lib_boost_logger_;
  }

  auto GetLogger() noexcept -> Logger& { return logger_; }

private:
  DoipServerLogger() = default;

  // actual logger context
  Logger logger_{"CTST", "gtst"};
};
}  // namespace logger
}  // namespace doip_handler
#endif  // DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_LOGGER_H_
