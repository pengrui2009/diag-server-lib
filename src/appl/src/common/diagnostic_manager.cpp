/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "src/common/diagnostic_manager.h"

namespace diag {
namespace server {
namespace common {
/*
 @ Class Name        : Diagnostic_manager
 @ Class Description : Parent class to create DCM and DEM class                            
 */
DiagnosticManager::DiagnosticManager() : exit_requested_{false} {}

DiagnosticManager::~DiagnosticManager() {
  {
    std::lock_guard<std::mutex> lock{mutex_};
    exit_requested_ = true;
  }
  cond_var.notify_all();
}

// Main function which keeps DCM alive
void DiagnosticManager::Main() {
  // Initialize the module
  Initialize();
  // Run the module
  Run();
  // Entering infinite loop
  while (!exit_requested_) {
    std::unique_lock<std::mutex> lck(mutex_);
    cond_var.wait(lck, [this]() { return exit_requested_; });
    // Thread exited
  }
  // Shutdown Module
  Shutdown();
}

// Function to shut down the component
void DiagnosticManager::SignalShutdown() {
  {
    std::lock_guard<std::mutex> lock{mutex_};
    exit_requested_ = true;
  }
  cond_var.notify_all();
}
}  // namespace common
}  // namespace server
}  // namespace diag
