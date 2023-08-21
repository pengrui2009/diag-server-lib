/* Diagnostic Server library
* Copyright (C) 2023  Rui Peng
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <iostream>

#include <unistd.h>

#include "uds_message.h"

// includes from diag-client library
#include "include/create_diagnostic_server.h"
#include "include/diagnostic_server.h"
#include "include/diagnostic_server_uds_message_type.h"
#include "include/diagnostic_server_vehicle_info_message_type.h"

/*
 * Main Entry point of diag client example 
 * Example to find available ECU in a network through vehicle discovery and connecting to it.
 */
int main() {
    uint16_t physical_address = 0x0100;
    uint16_t functional_address = 0x0101;

    // Create the Diagnostic client and pass the config for creating internal properties
    std::unique_ptr<diag::server::DiagServer> diag_server{
        diag::server::CreateDiagnosticServer("etc/diag_server_config.json")};

    // Initialize the Diagnostic Server library
    diag_server->Initialize();

    // Get conversation for tester one by providing the conversation name configured
    // in diag_client_config file passed while creating the diag client
    diag::server::conversation::DiagServerConversation &physical_diag_server_conversation {
        diag_server->CreateDiagnosticServerConversation(physical_address)};

    // diag::server::conversation::DiagServerConversation &funcitonal_diag_server_conversation {
    //     diag_server->CreateDiagnosticServerConversation(functional_address)};

    // sleep(1000000);
    // Start the conversation for tester one
    

    // funcitonal_diag_server_conversation.Startup();

    while(1) {
        if (!physical_diag_server_conversation.GetClientDiagState()) {
            physical_diag_server_conversation.Startup();
        }
        usleep(100000);
    }
    // Create a vehicle info request for finding ECU with matching VIN - ABCDEFGH123456789
    // diag::server::vehicle_info::VehicleInfoListRequestType
        // vehicle_info_request{1U, "ABCDEFGH123456789"};
    // Send Vehicle Identification request and get the response with available ECU information
    // std::pair<diag::client::DiagServer::VehicleResponseResult,
    //           diag::client::vehicle_info::VehicleInfoMessageResponseUniquePtr>
    //     vehicle_response_result{diag_client->SendVehicleIdentificationRequest(vehicle_info_request)};


    // Valid vehicle discovery response must be received before connecting to ECU
    // if(vehicle_response_result.first == diag::client::DiagServer::VehicleResponseResult::kStatusOk &&
    //     (vehicle_response_result.second)) {
    //   // Get the list of all vehicle available with matching VIN
    //   diag::client::vehicle_info::VehicleInfoMessage::VehicleInfoListResponseType vehicle_response_collection{
    //       vehicle_response_result.second->GetVehicleList()};

    //   // Create an uds message using first vehicle available in the list of response collection
    //   std::string remote_ecu_ip_address{vehicle_response_collection[0].ip_address}; // Remote ECU ip address
    //   std::uint16_t remote_ecu_server_logical_address{vehicle_response_collection[0].logical_address}; // Remote ECU logical address

    //   diag::client::uds_message::UdsRequestMessagePtr uds_message{
    //       std::make_unique<UdsMessage>(remote_ecu_ip_address, UdsMessage::ByteVector{0x10, 0x01})};

    //   // Connect Tester One to remote ECU
    //   diag::client::conversation::DiagServerConversation::ConnectResult connect_result{
    //       diag_client_conversation.ConnectToDiagServer(remote_ecu_server_logical_address,
    //                                                    uds_message->GetHostIpAddress())};

    //   if(connect_result == diag::client::conversation::DiagServerConversation::ConnectResult::kConnectSuccess) {
    //     // Use Tester One to send the diagnostic message to remote ECU
    //     std::pair<diag::client::conversation::DiagServerConversation::DiagResult,
    //               diag::client::uds_message::UdsResponseMessagePtr>
    //         diag_response{diag_client_conversation.SendDiagnosticRequest(std::move(uds_message))};

    //     if (diag_response.first == diag::client::conversation::DiagServerConversation::DiagResult::kDiagSuccess) {
    //       std::cout << "diag_client_conversation Total size: " << diag_response.second->GetPayload().size() << std::endl;
    //       // Print the payload
    //       for (auto const byte: diag_response.second->GetPayload()) {
    //         std::cout << "diag_client_conversation byte: " << std::hex << static_cast<int>(byte) << std::endl;
    //       }
    //     }

    //     // Disconnect Tester One from ECU1 with remote ip address "172.16.25.128"
    //     diag_client_conversation.DisconnectFromDiagServer();
    //   }

    // }

    // shutdown the conversation
    physical_diag_server_conversation.Shutdown();

    // funcitonal_diag_server_conversation.Shutdown();

    // important to release all the resources by calling de-initialize
    diag_server->DeInitialize();
    return (0);
}
