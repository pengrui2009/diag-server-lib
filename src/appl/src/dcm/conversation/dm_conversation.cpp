/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* includes */

#include "src/dcm/conversation/dm_conversation.h"

#include "src/common/logger.h"
#include "src/dcm/service/dm_uds_message.h"
#include "common/common_doip_types.h"
#include "src/dcm/service/service_0x10.h"

namespace diag {
namespace server {
namespace conversation {
//ctor
DmConversation::DmConversation(uint16_t logical_address,
                               ::uds_transport::conversion_manager::ConversionIdentifierType &conversion_identifier)
    : diag::server::conversation::DiagServerConversation(),
      activity_status_{ActivityStatusType::kInactive},
      active_session_{SessionControlType::kDefaultSession},
      active_security_{SecurityLevelType::kLocked},
      rx_buffer_size_{conversion_identifier.rx_buffer_size},
      p2_server_max_{conversion_identifier.p2_server_max},
      p2_star_server_max_{conversion_identifier.p2_star_server_max},
      source_address_{conversion_identifier.logical_address},
      target_address_{},
      logical_address_{logical_address},
      exit_request_{false},
      running_{false},
      dm_conversion_handler_{std::make_shared<DmConversationHandler>(conversion_identifier.handler_id, *this)} {
  
  uds_services_[0x10] = std::make_unique<service::Service0x10>(*this);

  // Start thread to receive messages
  thread_ = std::thread([this]() {
    std::unique_lock<std::mutex> lck(mutex_);
    while (!exit_request_) {
      if (!running_) {
        cond_var_.wait(lck, [this]() { return exit_request_ || running_; });
      }
      if (!exit_request_.load()) {
        if (running_) {
          while (!job_queue_.empty()) {
            std::function<void(void)> const job{job_queue_.front()};
            job();
            job_queue_.pop();
          }
        }
      }
    }
  });
}

//dtor
DmConversation::~DmConversation() = default;


// startup
void DmConversation::Startup() {

  // initialize the connection
  connection_ptr_->Initialize();
  // start the connection
  connection_ptr_->Start();
  // Change the state to Active
  activity_status_ = ActivityStatusType::kActive;
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(__FILE__, __LINE__, __func__,
                                                                      [&](std::stringstream &msg) {
                                                                        msg << "'" << conversation_name_ << "'"
                                                                            << "-> "
                                                                            << "Startup completed";
                                                                      });
}

// shutdown
void DmConversation::Shutdown() {
  // shutdown connection
  connection_ptr_->Stop();
  // Change the state to InActive
  activity_status_ = ActivityStatusType::kInactive;
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(__FILE__, __LINE__, __func__,
                                                                      [&](std::stringstream &msg) {
                                                                        msg << "'" << conversation_name_ << "'"
                                                                            << "-> "
                                                                            << "Shutdown completed";
                                                                      });
}

DiagServerConversation::ConnectResult DmConversation::ConnectToDiagServer(std::uint16_t target_address,
                                                                          IpAddress host_ip_addr) {
  // create an uds message just to get the port number
  // source address required for Routing Activation
  uds_transport::ByteVector payload{};  // empty payload
  // Send Connect request to doip layer
  DiagServerConversation::ConnectResult const connection_result{static_cast<DiagServerConversation::ConnectResult>(
      /*connection_ptr_->ConnectToHost(std::move(std::make_unique<diag::server::uds_message::DmUdsMessage>(
          source_address_, target_address, host_ip_addr, payload)))*/0)};
  remote_address_ = host_ip_addr;
  target_address_ = target_address;
  if (connection_result == DiagServerConversation::ConnectResult::kConnectSuccess) {
    logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
        __FILE__, __LINE__, __func__, [this](std::stringstream &msg) {
          msg << "'" << conversation_name_ << "'"
              << "-> "
              << "Successfully connected to Server with IP <" << remote_address_ << ">"
              << " and LA= 0x" << std::hex << target_address_;
        });
  } else {
    logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogError(
        __FILE__, __LINE__, __func__, [this](std::stringstream &msg) {
          msg << "'" << conversation_name_ << "'"
              << "-> "
              << "Failed connecting to Server with IP <" << remote_address_ << ">"
              << " and LA= 0x" << std::hex << target_address_;
        });
  }
  return connection_result;
}

DiagServerConversation::DisconnectResult DmConversation::DisconnectFromDiagServer() {
  DiagServerConversation::DisconnectResult ret_val{DiagServerConversation::DisconnectResult::kDisconnectFailed};
  // Check if already connected before disconnecting
  if (/*connection_ptr_->IsConnectToHost()*/1) {
    // Send disconnect request to doip layer
    // ret_val = static_cast<DiagServerConversation::DisconnectResult>(connection_ptr_->DisconnectFromHost());
    if (ret_val == DiagServerConversation::DisconnectResult::kDisconnectSuccess) {
      logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
          __FILE__, __LINE__, __func__, [this](std::stringstream &msg) {
            msg << "'" << conversation_name_ << "'"
                << "-> "
                << "Successfully disconnected from Server with IP <" << remote_address_ << ">"
                << " and LA= 0x" << std::hex << target_address_;
          });
    } else {
      logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogWarn(
          __FILE__, __LINE__, __func__, [&](std::stringstream &msg) {
            msg << "'" << conversation_name_ << "'"
                << "-> "
                << "Failed to disconnect from Server with IP <" << remote_address_ << ">";
          });
    }
  } else {
    ret_val = DiagServerConversation::DisconnectResult::kAlreadyDisconnected;
  }
  return ret_val;
}

std::pair<DiagServerConversation::DiagResult, uds_message::UdsResponseMessagePtr> DmConversation::SendDiagnosticRequest(
    uds_message::UdsRequestMessageConstPtr message) {
  std::pair<DiagServerConversation::DiagResult, uds_message::UdsResponseMessagePtr> ret_val{
      DiagServerConversation::DiagResult::kDiagGenericFailure, nullptr};
  if (message) {
    // fill the data
    uds_transport::ByteVector payload{message->GetPayload()};
    // Initiate Sending of diagnostic request
    uds_transport::UdsTransportProtocolMgr::TransmissionResult transmission_result{
        connection_ptr_->Transmit(std::move(std::make_unique<diag::server::uds_message::DmUdsMessage>(
            source_address_, target_address_, message->GetHostIpAddress(), payload)))};
    if (transmission_result == uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk) {
      // Diagnostic Request Sent successful
      logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
          __FILE__, __LINE__, __func__, [&](std::stringstream &msg) {
            msg << "'" << conversation_name_ << "'"
                << "-> "
                << "Diagnostic Request Sent & Positive Ack received";
          });
      conversation_state_.GetConversationStateContext().TransitionTo(ConversationState::kDiagWaitForRes);
      // Wait P6Max / P2ClientMax
      WaitForResponse(
          [&]() {
            ret_val.first = DiagServerConversation::DiagResult::kDiagResponseTimeout;
            conversation_state_.GetConversationStateContext().TransitionTo(ConversationState::kIdle);
            logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
                __FILE__, __LINE__, "", [&](std::stringstream &msg) {
                  msg << "'" << conversation_name_ << "'"
                      << "-> "
                      << "Diagnostic Response P2 Timeout happened after " << p2_server_max_ << " milliseconds";
                });
          },
          [&]() {
            // pending or pos/neg response
            if (conversation_state_.GetConversationStateContext().GetActiveState().GetState() ==
                ConversationState::kDiagRecvdFinalRes) {
              // pos/neg response received
            } else if (conversation_state_.GetConversationStateContext().GetActiveState().GetState() ==
                       ConversationState::kDiagRecvdPendingRes) {
              // first pending received
              conversation_state_.GetConversationStateContext().TransitionTo(ConversationState::kDiagStartP2StarTimer);
            }
          },
          p2_server_max_);

      // Wait until final response or timeout
      while (conversation_state_.GetConversationStateContext().GetActiveState().GetState() !=
             ConversationState::kIdle) {
        // Check the active state
        switch (conversation_state_.GetConversationStateContext().GetActiveState().GetState()) {
          case ConversationState::kDiagRecvdPendingRes:
            conversation_state_.GetConversationStateContext().TransitionTo(ConversationState::kDiagStartP2StarTimer);
            break;
          case ConversationState::kDiagRecvdFinalRes:
            // do nothing
            break;
          case ConversationState::kDiagStartP2StarTimer:
            // wait P6Star/ P2 star client time
            WaitForResponse(
                [&]() {
                  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
                      __FILE__, __LINE__, "", [&](std::stringstream &msg) {
                        msg << "'" << conversation_name_ << "'"
                            << "-> "
                            << "Diagnostic Response P2 Star Timeout happened after " << p2_star_server_max_
                            << " milliseconds";
                        ;
                      });
                  ret_val.first = DiagServerConversation::DiagResult::kDiagResponseTimeout;
                  conversation_state_.GetConversationStateContext().TransitionTo(ConversationState::kIdle);
                },
                [&]() {
                  // pending or pos/neg response
                  if (conversation_state_.GetConversationStateContext().GetActiveState().GetState() ==
                      ConversationState::kDiagRecvdFinalRes) {
                    // pos/neg response received
                  } else if (conversation_state_.GetConversationStateContext().GetActiveState().GetState() ==
                             ConversationState::kDiagRecvdPendingRes) {
                    // pending received again
                    conversation_state_.GetConversationStateContext().TransitionTo(
                        ConversationState::kDiagStartP2StarTimer);
                  }
                },
                p2_star_server_max_);
            break;
          case ConversationState::kDiagSuccess:
            // change state to idle, form the uds response and return
            ret_val.second = std::move(std::make_unique<diag::server::uds_message::DmUdsResponse>(payload_rx_buffer));
            ret_val.first = DiagServerConversation::DiagResult::kDiagSuccess;
            conversation_state_.GetConversationStateContext().TransitionTo(ConversationState::kIdle);
            break;
          default:
            // nothing
            break;
        }
      }
    } else {
      // failure
      ret_val.first = ConvertResponseType(transmission_result);
    }
  } else {
    ret_val.first = DiagServerConversation::DiagResult::kDiagInvalidParameter;
    logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogWarn(__FILE__, __LINE__, "",
                                                                        [&](std::stringstream &msg) {
                                                                          msg << "'" << conversation_name_ << "'"
                                                                              << "-> "
                                                                              << "Diagnostic Request message is empty";
                                                                        });
  }
  return ret_val;
}

// 
bool DmConversation::GetClientDiagState() {
  return connection_ptr_->GetConnectionState();
}

// Function to add register Connection to conversion
void DmConversation::RegisterConnection(std::shared_ptr<uds_transport::Connection> connection) {
  connection_ptr_ = std::move(connection);
}

std::shared_ptr<::uds_transport::ConversionHandler> &DmConversation::GetConversationHandler() {
  return dm_conversion_handler_;
}


auto DmConversation::GetDoIPPayloadType(std::vector<uint8_t> payload) noexcept -> uint16_t {
  return ((uint16_t) (((payload[BYTE_POS_TWO] & 0xFF) << 8) | (payload[BYTE_POS_THREE] & 0xFF)));
}

auto DmConversation::GetDoIPPayloadLength(std::vector<uint8_t> payload) noexcept -> uint32_t {
  return ((uint32_t) ((payload[BYTE_POS_FOUR] << 24U) & 0xFF000000) |
          (uint32_t) ((payload[BYTE_POS_FIVE] << 16U) & 0x00FF0000) |
          (uint32_t) ((payload[BYTE_POS_SIX] << 8U) & 0x0000FF00) |
          (uint32_t) ((payload[BYTE_POS_SEVEN] & 0x000000FF)));
}

void DmConversation::CreateDoipGenericHeader(std::vector<uint8_t> &doipHeader, std::uint16_t payload_type,
                                                          std::uint32_t payload_len) {
  doipHeader.push_back(kDoip_ProtocolVersion);
  doipHeader.push_back(~((uint8_t) kDoip_ProtocolVersion));
  doipHeader.push_back((uint8_t) ((payload_type & 0xFF00) >> 8));
  doipHeader.push_back((uint8_t) (payload_type & 0x00FF));
  doipHeader.push_back((uint8_t) ((payload_len & 0xFF000000) >> 24));
  doipHeader.push_back((uint8_t) ((payload_len & 0x00FF0000) >> 16));
  doipHeader.push_back((uint8_t) ((payload_len & 0x0000FF00) >> 8));
  doipHeader.push_back((uint8_t) (payload_len & 0x000000FF));
}


void DmConversation::SendRoutingActivationResponse(const DoipMessage &doip_message) {
  std::vector<uint8_t> payload;
  uds_transport::UdsMessagePtr message = std::make_unique<diag::server::uds_message::DmUdsMessage>();
  // TcpMessagePtr routing_activation_response{std::make_unique<TcpMessage>()};
  // create header
  // routing_activation_response->txBuffer_.reserve(kDoipheadrSize + kDoip_RoutingActivation_ResMinLen);
  // CreateDoipGenericHeader(routing_activation_response->txBuffer_, kDoip_RoutingActivation_ResType,
  //                         kDoip_RoutingActivation_ResMinLen);

  // logical address of client
  payload.emplace_back(doip_message.payload[0]);
  payload.emplace_back(doip_message.payload[1]);
  // logical address of server
  payload.emplace_back(logical_address_ >> 8U);
  payload.emplace_back(logical_address_ & 0xFFU);
  // activation response code
  payload.emplace_back(kDoip_RoutingActivation_ResCode_ConfirmtnRequired);
  payload.emplace_back(0x00);
  payload.emplace_back(0x00);
  payload.emplace_back(0x00);
  payload.emplace_back(0x00);
  
  message->SetPayload(payload);
  
  uds_transport::UdsTransportProtocolMgr::TransmissionResult ret;
  ret = connection_ptr_->Transmit(std::move(message));
  if (uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk == ret) {

  }
  // if (connection_ptr_->Transmit(std::move(routing_activation_response))) { 
  //   running_ = false; 
  // }
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
        __FILE__, __LINE__, "", [&](std::stringstream &msg) {
          msg << "DmConversation::SendRoutingActivationResponse";          
        });
}

void DmConversation::SendDiagnosticMessageAckResponse(const DoipMessage &doip_message) {
  // TcpMessagePtr diag_msg_ack_response{std::make_unique<TcpMessage>()};
  // // create header
  // diag_msg_ack_response->txBuffer_.reserve(kDoipheadrSize + kDoip_DiagMessageAck_ResMinLen);
  // if (diag_msg_ack_code_ == kDoip_DiagnosticMessage_PosAckCode_Confirm) {
  //   CreateDoipGenericHeader(diag_msg_ack_response->txBuffer_, kDoip_DiagMessagePosAck_Type,
  //                           kDoip_DiagMessageAck_ResMinLen);
  // } else {
  //   CreateDoipGenericHeader(diag_msg_ack_response->txBuffer_, kDoip_DiagMessageNegAck_Type,
  //                           kDoip_DiagMessageAck_ResMinLen);
  // }
  // // logical address of client
  // diag_msg_ack_response->txBuffer_.emplace_back(logical_address_ >> 8U);
  // diag_msg_ack_response->txBuffer_.emplace_back(logical_address_ & 0xFFU);

  // // logical address of target
  // diag_msg_ack_response->txBuffer_.emplace_back(received_doip_message_.payload[0]);
  // diag_msg_ack_response->txBuffer_.emplace_back(received_doip_message_.payload[1]);

  // // activation response code
  // diag_msg_ack_response->txBuffer_.emplace_back(diag_msg_ack_code_);

  // if (tcp_connection_->Transmit(std::move(diag_msg_ack_response))) {
  //   // Check for diag message ack code
  //   if (diag_msg_ack_code_ == kDoip_DiagnosticMessage_PosAckCode_Confirm) {
  //     logger::LibGtestLogger::GetLibGtestLogger().GetLogger().LogInfo(
  //         __FILE__, __LINE__, "",
  //         [](std::stringstream &msg) { msg << "Sending of Diagnostic Message Pos Ack Response success"; });

  //     // Check for pending responses set
  //     if(!uds_pending_response_payload_.empty()) {
  //       // emplace pending response jobs based on number of pending response
  //       for(std::uint8_t pending_count{0}; pending_count < num_of_pending_response_; pending_count++) {
  //         job_queue_.emplace([this]() {
  //           // wait so that diag positive ack is processed first before sending diag response
  //           std::this_thread::sleep_for(std::chrono::milliseconds(25));
  //           this->SendDiagnosticPendingMessageResponse();
  //         });        
  //       }
  //     }
  //     // emplace a positive response
  //     job_queue_.emplace([this]() {
  //       // wait so that diag positive ack is processed first before sending diag response
  //       std::this_thread::sleep_for(std::chrono::milliseconds(25));
  //       this->SendDiagnosticMessageResponse();
  //     });
  //     running_ = true;
  //     cond_var_.notify_all();
  //   } else {
  //     logger::LibGtestLogger::GetLibGtestLogger().GetLogger().LogInfo(
  //         __FILE__, __LINE__, "",
  //         [](std::stringstream &msg) { msg << "Sending of Diagnostic Message Neg Ack Response success"; });
  //   }
  // }
}

void DmConversation::SendDiagnosticMessageResponse(const DoipMessage &doip_message) {
  // TcpMessagePtr diag_uds_message_response{std::make_unique<TcpMessage>()};
  // // create header
  // diag_uds_message_response->txBuffer_.reserve(kDoipheadrSize + kDoip_DiagMessage_ReqResMinLen +
  //                                              uds_response_payload_.size());
  // CreateDoipGenericHeader(diag_uds_message_response->txBuffer_, kDoip_DiagMessage_Type,
  //                         kDoip_DiagMessage_ReqResMinLen + uds_response_payload_.size());

  // // logical address of client
  // diag_uds_message_response->txBuffer_.emplace_back(logical_address_ >> 8U);
  // diag_uds_message_response->txBuffer_.emplace_back(logical_address_ & 0xFFU);

  // // logical address of target
  // diag_uds_message_response->txBuffer_.emplace_back(received_doip_message_.payload[0]);
  // diag_uds_message_response->txBuffer_.emplace_back(received_doip_message_.payload[1]);

  // // copy the payload
  // diag_uds_message_response->txBuffer_.insert(
  //     diag_uds_message_response->txBuffer_.begin() + kDoipheadrSize + kDoip_DiagMessage_ReqResMinLen,
  //     uds_response_payload_.begin(), uds_response_payload_.end());

  // if (tcp_connection_->Transmit(std::move(diag_uds_message_response))) {
  //   running_ = false;
  //   logger::LibGtestLogger::GetLibGtestLogger().GetLogger().LogInfo(__FILE__, __LINE__, "", [](std::stringstream &msg) {
  //     msg << "Sending of Diagnostic Response message success";
  //   });
  // }
}


void DmConversation::SendDiagnosticPendingMessageResponse(const DoipMessage &doip_message) {
  // TcpMessagePtr diag_uds_message_response{std::make_unique<TcpMessage>()};
  // // create header
  // diag_uds_message_response->txBuffer_.reserve(kDoipheadrSize + kDoip_DiagMessage_ReqResMinLen +
  //                                              uds_pending_response_payload_.size());
  // CreateDoipGenericHeader(diag_uds_message_response->txBuffer_, kDoip_DiagMessage_Type,
  //                         kDoip_DiagMessage_ReqResMinLen + uds_pending_response_payload_.size());

  // // logical address of client
  // diag_uds_message_response->txBuffer_.emplace_back(logical_address_ >> 8U);
  // diag_uds_message_response->txBuffer_.emplace_back(logical_address_ & 0xFFU);

  // // logical address of target
  // diag_uds_message_response->txBuffer_.emplace_back(received_doip_message_.payload[0]);
  // diag_uds_message_response->txBuffer_.emplace_back(received_doip_message_.payload[1]);

  // // copy the payload
  // diag_uds_message_response->txBuffer_.insert(
  //     diag_uds_message_response->txBuffer_.begin() + kDoipheadrSize + kDoip_DiagMessage_ReqResMinLen,
  //     uds_pending_response_payload_.begin(), uds_pending_response_payload_.end());

  // if (tcp_connection_->Transmit(std::move(diag_uds_message_response))) {
  //   running_ = false;
  //   // logger::LibGtestLogger::GetLibGtestLogger().GetLogger().LogInfo(__FILE__, __LINE__, "", [](std::stringstream &msg) {
  //   //   msg << "Sending of Diagnostic Pending Response message success";
  //   // });
  // }
}

// Indicate message Diagnostic message reception over TCP to user
std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr>
DmConversation::IndicateMessage(uds_transport::UdsMessage::Address source_addr,
                                uds_transport::UdsMessage::Address target_addr,
                                uds_transport::UdsMessage::TargetAddressType type, uds_transport::ChannelID channel_id,
                                std::size_t size, uds_transport::Priority priority,
                                uds_transport::ProtocolKind protocol_kind, std::vector<uint8_t> payload_info) {
  std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr> ret_val{
      uds_transport::UdsTransportProtocolMgr::IndicationResult::kIndicationNOk, nullptr};
  DoipMessage received_doip_message;


  // received_doip_message.host_ip_address = tcp_rx_message->host_ip_address_;
  // received_doip_message.port_num = tcp_rx_message->host_port_num_;
  received_doip_message.protocol_version = payload_info[0];
  received_doip_message.protocol_version_inv = payload_info[1];
  received_doip_message.payload_type = GetDoIPPayloadType(payload_info);
  received_doip_message.payload_length = GetDoIPPayloadLength(payload_info);
  if (received_doip_message.payload_length > 0U) {
    received_doip_message.payload.insert(received_doip_message.payload.begin(),
                                          payload_info.begin() + kDoipheadrSize,
                                          payload_info.end());
  }
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
          __FILE__, __LINE__, "", [&](std::stringstream &msg) {
            msg << "DmConversation::IndicateMessage receive data:";
            for (int i=0; i<payload_info.size(); i++) {
              msg << static_cast<int>(payload_info[i]) << " ";
            }
          });
  // Trigger async transmission
  {
    std::lock_guard<std::mutex> const lck{mutex_};
    job_queue_.emplace([this, received_doip_message]() {
      if (received_doip_message.payload_type == kDoip_RoutingActivation_ReqType) {
        this->SendRoutingActivationResponse(received_doip_message);
      } else if (received_doip_message.payload_type == kDoip_DiagMessage_Type) {
        this->SendDiagnosticMessageAckResponse(received_doip_message);
      }
    });
    running_ = true;
    cond_var_.notify_all();
  }
#if 0  
  // Verify the payload received :-
  if (!payload_info.empty()) {
    // Check for size, else kIndicationOverflow
    if (size <= rx_buffer_size_) {
      // Check for pending response
      // payload = 0x7F XX 0x78
      if (payload_info[2U] == 0x78) {
        logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
            __FILE__, __LINE__, "", [&](std::stringstream &msg) {
              msg << "'" << conversation_name_ << "'"
                  << "-> "
                  << "Diagnostic pending response received in Conversation";
            });
        ret_val.first = uds_transport::UdsTransportProtocolMgr::IndicationResult::kIndicationPending;
        conversation_state_.GetConversationStateContext().TransitionTo(ConversationState::kDiagRecvdPendingRes);
      } else {
        logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogDebug(
            __FILE__, __LINE__, "", [this](std::stringstream &msg) {
              msg << "'" << conversation_name_ << "'"
                  << "-> "
                  << "Diagnostic final response received in Conversation";
            });
        // positive or negative response, provide valid buffer
        // resize the global rx buffer
        payload_rx_buffer.resize(size);
        ret_val.first = uds_transport::UdsTransportProtocolMgr::IndicationResult::kIndicationOk;
        ret_val.second = std::move(std::make_unique<diag::server::uds_message::DmUdsMessage>(
            source_address_, target_address_, "", payload_rx_buffer));
        conversation_state_.GetConversationStateContext().TransitionTo(ConversationState::kDiagRecvdFinalRes);
      }
      WaitCancel();
    } else {
      logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogError(
          __FILE__, __LINE__, "", [&](std::stringstream &msg) {
            msg << "'" << conversation_name_ << "'"
                << "-> "
                << "Diagnostic Conversation Error Indication Overflow";
          });
      ret_val.first = uds_transport::UdsTransportProtocolMgr::IndicationResult::kIndicationOverflow;
    }
  } else {
    logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogError(
        __FILE__, __LINE__, "", [&](std::stringstream &msg) {
          msg << "'" << conversation_name_ << "'"
              << "-> "
              << "Diagnostic Conversation Rx Payload size 0 received";
        });
  }

  uint8_t sid = 0x10;

  if (uds_services_[sid]->GetSessionType()) {
    if (uds_services_[sid]->GetSecurityLevel() == 0) {

    }
  }
  job_queue_.emplace([this, sid]() { this->uds_services_[sid]->Service(); });
  
  running_ = true;
  cond_var_.notify_all();
#endif
  return ret_val;
}

void DmConversation::Service() {

}
// Hands over a valid message to conversion
void DmConversation::HandleMessage(uds_transport::UdsMessagePtr message) {
  if (message != nullptr) {
    conversation_state_.GetConversationStateContext().TransitionTo(ConversationState::kDiagSuccess);
  }
}

void DmConversation::WaitForResponse(std::function<void()> &&timeout_func, std::function<void()> &&cancel_func,
                                     int msec) {
  if (sync_timer_.Start(std::chrono::milliseconds(msec)) == SyncTimerState::kTimeout) {
    timeout_func();
  } else {
    cancel_func();
  }
}

void DmConversation::WaitCancel() { sync_timer_.Stop(); }

DiagServerConversation::DiagResult DmConversation::ConvertResponseType(
    uds_transport::UdsTransportProtocolMgr::TransmissionResult result_type) {
  DiagServerConversation::DiagResult ret_result{DiagServerConversation::DiagResult::kDiagGenericFailure};
  switch (result_type) {
    case uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitFailed:
      ret_result = DiagServerConversation::DiagResult::kDiagRequestSendFailed;
      break;
    case uds_transport::UdsTransportProtocolMgr::TransmissionResult::kNoTransmitAckReceived:
      ret_result = DiagServerConversation::DiagResult::kDiagAckTimeout;
      break;
    case uds_transport::UdsTransportProtocolMgr::TransmissionResult::kNegTransmitAckReceived:
      ret_result = DiagServerConversation::DiagResult::kDiagNegAckReceived;
      break;
    case uds_transport::UdsTransportProtocolMgr::TransmissionResult::kBusyProcessing:
      ret_result = DiagServerConversation::DiagResult::kDiagBusyProcessing;
      break;
    default:
      ret_result = DiagServerConversation::DiagResult::kDiagGenericFailure;
      break;
  }
  return ret_result;
}

// ctor
DmConversationHandler::DmConversationHandler(uds_transport::conversion_manager::ConversionHandlerID handler_id,
                                             DmConversation &dm_conversion)
    : uds_transport::ConversionHandler{handler_id},
      dm_conversation_{dm_conversion} {}

// Indicate message Diagnostic message reception over TCP to user
std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr>
DmConversationHandler::IndicateMessage(uds_transport::UdsMessage::Address source_addr,
                                       uds_transport::UdsMessage::Address target_addr,
                                       uds_transport::UdsMessage::TargetAddressType type,
                                       uds_transport::ChannelID channel_id, std::size_t size,
                                       uds_transport::Priority priority, uds_transport::ProtocolKind protocol_kind,
                                       std::vector<uint8_t> payload_info) {
  return (dm_conversation_.IndicateMessage(source_addr, target_addr, type, channel_id, size, priority, protocol_kind,
                                           payload_info));
}

// Hands over a valid message to conversion
void DmConversationHandler::HandleMessage(uds_transport::UdsMessagePtr message) {
  dm_conversation_.HandleMessage(std::move(message));
}
}  // namespace conversation
}  // namespace server
}  // namespace diag