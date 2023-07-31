/* Diagnostic Server library
* Copyright (C) 2023  Rui Peng
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <gtest/gtest.h>

#include "include/create_diagnostic_server.h"
#include "include/diagnostic_server.h"
#include "main.h"

namespace doip_client {

TEST_F(DoipClientFixture, VerifyPreselectionModeEmpty) {
  doip_handler::DoipUdpHandler::VehicleAddrInfo vehicle_addr_response{0xFA25U, "ABCDEFGH123456789", "00:02:36:31:00:1c",
                                                                      "0a:0b:0c:0d:0e:0f"};
  // Create an expected vehicle identification response
  GetDoipTestUdpHandlerRef().SetExpectedVehicleIdentificationResponseToBeSent(vehicle_addr_response);

  // Send Vehicle Identification request and expect response
  diag::client::vehicle_info::VehicleInfoListRequestType vehicle_info_request{0u, ""};
  std::pair<diag::client::DiagServer::VehicleResponseResult,
            diag::client::vehicle_info::VehicleInfoMessageResponseUniquePtr>
      response_result{GetDiagServerRef().SendVehicleIdentificationRequest(vehicle_info_request)};

  // Verify Vehicle identification request with no payload
  EXPECT_TRUE(GetDoipTestUdpHandlerRef().VerifyVehicleIdentificationRequestWithExpectedVIN(""));

  // Verify Vehicle identification responses received successfully
  ASSERT_EQ(response_result.first, diag::client::DiagServer::VehicleResponseResult::kStatusOk);
  ASSERT_TRUE(response_result.second);

  // Get the list of all vehicle available
  diag::client::vehicle_info::VehicleInfoMessage::VehicleInfoListResponseType response_collection{
      response_result.second->GetVehicleList()};

  EXPECT_EQ(response_collection.size(), 1U);
  EXPECT_EQ(response_collection[0].ip_address, DiagUdpIpAddress);
  EXPECT_EQ(response_collection[0].logical_address, vehicle_addr_response.logical_address);
  EXPECT_EQ(response_collection[0].vin, vehicle_addr_response.vin);
  EXPECT_EQ(response_collection[0].eid, vehicle_addr_response.eid);
  EXPECT_EQ(response_collection[0].gid, vehicle_addr_response.gid);
}

TEST_F(DoipClientFixture, VerifyPreselectionModeVin) {
  doip_handler::DoipUdpHandler::VehicleAddrInfo vehicle_addr_response{0xFA25U, "ABCDEFGH123456789", "00:02:36:31:00:1c",
                                                                      "0a:0b:0c:0d:0e:0f"};
  // Create an expected vehicle identification response
  GetDoipTestUdpHandlerRef().SetExpectedVehicleIdentificationResponseToBeSent(vehicle_addr_response);

  // Send Vehicle Identification request with VIN and expect response
  diag::client::vehicle_info::VehicleInfoListRequestType vehicle_info_request{1U, "ABCDEFGH123456789"};
  std::pair<diag::client::DiagServer::VehicleResponseResult,
            diag::client::vehicle_info::VehicleInfoMessageResponseUniquePtr>
      response_result{GetDiagServerRef().SendVehicleIdentificationRequest(vehicle_info_request)};

  // Verify Vehicle identification request payload matches
  EXPECT_TRUE(GetDoipTestUdpHandlerRef().VerifyVehicleIdentificationRequestWithExpectedVIN("ABCDEFGH123456789"));

  // Verify Vehicle identification responses
  ASSERT_EQ(response_result.first, diag::client::DiagServer::VehicleResponseResult::kStatusOk);
  ASSERT_TRUE(response_result.second);

  // Get the list of all vehicle available
  diag::client::vehicle_info::VehicleInfoMessage::VehicleInfoListResponseType response_collection{
      response_result.second->GetVehicleList()};

  // Verify the received response
  EXPECT_EQ(response_collection.size(), 1U);
  EXPECT_EQ(response_collection[0].ip_address, DiagUdpIpAddress);
  EXPECT_EQ(response_collection[0].logical_address, vehicle_addr_response.logical_address);
  EXPECT_EQ(response_collection[0].vin, vehicle_addr_response.vin);
  EXPECT_EQ(response_collection[0].eid, vehicle_addr_response.eid);
  EXPECT_EQ(response_collection[0].gid, vehicle_addr_response.gid);
}

TEST_F(DoipClientFixture, VerifyPreselectionModeEID) {
  doip_handler::DoipUdpHandler::VehicleAddrInfo vehicle_addr_response{0xFA25U, "ABCDEFGH123456789", "00:02:36:31:00:1c",
                                                                      "0a:0b:0c:0d:0e:0f"};
  // Create an expected vehicle identification response
  GetDoipTestUdpHandlerRef().SetExpectedVehicleIdentificationResponseToBeSent(vehicle_addr_response);

  // Send Vehicle Identification request with EID and expect response
  diag::client::vehicle_info::VehicleInfoListRequestType vehicle_info_request{2U, "00:02:36:31:00:1c"};
  std::pair<diag::client::DiagServer::VehicleResponseResult,
            diag::client::vehicle_info::VehicleInfoMessageResponseUniquePtr>
      response_result{GetDiagServerRef().SendVehicleIdentificationRequest(vehicle_info_request)};

  // Verify Vehicle identification request payload matches
  EXPECT_TRUE(GetDoipTestUdpHandlerRef().VerifyVehicleIdentificationRequestWithExpectedEID("00:02:36:31:00:1c"));

  // Verify Vehicle identification responses
  ASSERT_EQ(response_result.first, diag::client::DiagServer::VehicleResponseResult::kStatusOk);
  ASSERT_TRUE(response_result.second);

  // Get the list of all vehicle available
  diag::client::vehicle_info::VehicleInfoMessage::VehicleInfoListResponseType response_collection{
      response_result.second->GetVehicleList()};

  // Verify the received response
  EXPECT_EQ(response_collection.size(), 1U);
  EXPECT_EQ(response_collection[0].ip_address, DiagUdpIpAddress);
  EXPECT_EQ(response_collection[0].logical_address, vehicle_addr_response.logical_address);
  EXPECT_EQ(response_collection[0].vin, vehicle_addr_response.vin);
  EXPECT_EQ(response_collection[0].eid, vehicle_addr_response.eid);
  EXPECT_EQ(response_collection[0].gid, vehicle_addr_response.gid);
}

}  // namespace doip_client