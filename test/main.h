/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <string>
#include <thread>

#include "doip_handler/doip_udp_handler.h"
#include "doip_handler/logger.h"
#include "include/create_diagnostic_server.h"
#include "include/diagnostic_server.h"

namespace doip_server {

// Diag Test Server Udp Ip Address
const std::string DiagUdpIpAddress{"172.16.25.128"};

// Port number
constexpr std::uint16_t DiagUdpPortNum{13400u};

// Path to json file
const std::string DiagServerJsonPath{"../../diag-client-lib/appl/etc/diag_client_config.json"};

class DoipClientFixture : public ::testing::Test {
protected:
  DoipClientFixture()
      : diag_client_{diag::client::CreateDiagnosticServer(DiagServerJsonPath)},
        doip_udp_handler_{DiagUdpIpAddress, DiagUdpPortNum} {
    // Initialize logger
    doip_handler::logger::DoipServerLogger::GetDiagServerLogger();
    // Initialize doip test handler
    doip_udp_handler_.Initialize();
    // Initialize diag client library
    diag_client_->Initialize();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  ~DoipClientFixture() override {
    // De-initialize diag client library
    diag_client_->DeInitialize();
    // De-initialize doip test handler
    doip_udp_handler_.DeInitialize();
  }

  void SetUp() override {}

  void TearDown() override {}

  // Function to get Diag client library reference
  auto GetDiagServerRef() noexcept -> diag::client::DiagServer& { return *diag_server_; }

  // Function to get Doip Test Handler reference
  auto GetDoipTestUdpHandlerRef() noexcept -> doip_handler::DoipUdpHandler& { return doip_udp_handler_; }

private:
  // diag server library
  std::unique_ptr<diag::server::DiagServer> diag_server_;

  // doip test handler
  doip_handler::DoipUdpHandler doip_udp_handler_;
};

}  // namespace doip_server
