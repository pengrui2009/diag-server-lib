/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <chrono>

#include "include/create_diagnostic_server.h"
#include "main.h"

namespace doip_client {

TEST_F(DoipClientFixture, StartupAndTermination) {
  // Get conversation for tester one
  diag::client::conversation::DiagServerConversation &diag_client_conversation{
      GetDiagServerRef().GetDiagnosticClientConversation("DiagTesterOne")};

  diag_client_conversation.Startup();
  diag_client_conversation.Shutdown();
}

}  // namespace doip_client