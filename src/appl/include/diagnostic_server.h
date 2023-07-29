/* Diagnostic Server library
 * Copyright (C) 2023  Avijit Dey
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/** @file diagnostic_server.h
 *  @brief Header file of Diag Server Class
 *  @author Avijit Dey
 */
#ifndef DIAGNOSTIC_SERVER_LIB_APPL_INCLUDE_DIAGNOSTIC_SERVER_H
#define DIAGNOSTIC_SERVER_LIB_APPL_INCLUDE_DIAGNOSTIC_SERVER_H

#include <string_view>

#include "diagnostic_server_conversation.h"
#include "diagnostic_server_vehicle_info_message_type.h"

namespace diag {
namespace server {

/**
 * @brief    Class to manage Diagnostic Client
 */
class DiagServer {
public:
  /**
   * @brief  Definitions of Vehicle Identification response result
   */
  enum class VehicleResponseResult : std::uint8_t {
    kTransmitFailed = 0U,     /**< Failure on Transmissions */
    kInvalidParameters = 1U,  /**< Invalid Parameter passed */
    kNoResponseReceived = 2U, /**< No vehicle identification response received */
    kStatusOk = 3U            /**< Vehicle identification response received success */
  };

public:
  /**
   * @brief         Constructs an instance of DiagServer
   * @remarks       Implemented requirements:
   *                DiagServerLib-Construction
   */
  DiagServer() = default;

  /**
   * @brief         Destruct an instance of DiagServer
   * @remarks       Implemented requirements:
   *                DiagServerLib-Destruction
   */
  virtual ~DiagServer() = default;

  /**
   * @brief        Function to initialize the already created instance of DiagServer
   * @details      Must be called once and before using any other functions of DiagServer
   * @remarks      Implemented requirements:
   *               DiagServerLib-Initialization
   */
  virtual void Initialize() = 0;

  /**
   * @brief        Function to de-initialize the already initialized instance of DiagServer
   * @details      Must be called during shutdown phase, no further processing of any
   *               function will be allowed after this call
   * @remarks      Implemented requirements:
   *               DiagServerLib-DeInitialization
   */
  virtual void DeInitialize() = 0;

  /**
   * @brief       Function to send vehicle identification request and get the Diagnostic Server list
   * @param[in]   vehicle_info_request
   *              Vehicle information sent along with request
   * @return      std::pair<VehicleResponseResult, diag::server::vehicle_info::VehicleInfoMessageResponsePtr>
   *              Pair consisting the result & response, contains valid response when result = kStatusOk
   * @remarks     Implemented requirements:
   *              DiagServerLib-VehicleDiscovery
   */
  virtual std::pair<VehicleResponseResult, diag::server::vehicle_info::VehicleInfoMessageResponseUniquePtr>
  SendVehicleIdentificationRequest(diag::server::vehicle_info::VehicleInfoListRequestType vehicle_info_request) = 0;

  /**
   * @brief       Function to get required diag server conversation object based on conversation name
   * @param[in]   conversation_name
   *              Name of conversation configured as json parameter "ConversationName"
   * @return      DiagServerConversation&
   *              Reference to diag client conversation
   * @remarks     Implemented requirements:
   *              DiagServerLib-MultipleTester-Connection, DiagServerLib-Conversation-Construction
   */
  virtual diag::server::conversation::DiagServerConversation& StartDiagnosticServerConversation() = 0;
};

}  // namespace server
}  // namespace diag
#endif  // DIAGNOSTIC_SERVER_LIB_APPL_INCLUDE_DIAGNOSTIC_SERVER_H
