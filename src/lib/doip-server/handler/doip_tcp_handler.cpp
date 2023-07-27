/* Diagnostic Server library
* Copyright (C) 2023  Avijit Dey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


#include "handler/doip_tcp_handler.h"
#include "uds_transport/protocol_mgr.h"


#include "connection/connection_manager.h"
#include "common/common_doip_types.h"
#include "common/logger.h"

namespace doip_server {
namespace doip_handler {

DoipTcpHandler::DoipTcpHandler(std::string_view local_tcp_address, uint16_t tcp_port_num)
    : tcp_socket_handler_{std::make_unique<::doip_handler::tcpSocket::DoipTcpSocketHandler>(local_tcp_address, tcp_port_num)} {}

DoipTcpHandler::~DoipTcpHandler() = default;

uds_transport::UdsTransportProtocolMgr::TransmissionResult DoipTcpHandler::Transmit(
    uds_transport::UdsMessageConstPtr message, std::uint16_t logical_address) {
    return (doip_channel_list_[logical_address]->Transmit(std::move(message)));  
}

DoipChannel &DoipTcpHandler::CreateDoipChannel(const std::shared_ptr<uds_transport::ConversionHandler> &conversation, std::uint16_t logical_address) {
  // create new doip channel
  doip_channel_list_.emplace(logical_address,
                             std::make_unique<DoipChannel>(logical_address, *tcp_socket_handler_));
  return *doip_channel_list_[logical_address];
}

DoipChannel::DoipChannel(std::uint16_t logical_address,
                                         ::doip_handler::tcpSocket::DoipTcpSocketHandler &tcp_socket_handler)
    : logical_address_{logical_address},
      tcp_socket_handler_{tcp_socket_handler},
      tcp_connection_{},
      exit_request_{false},
      running_{false},
      routing_activation_res_code_{kDoip_RoutingActivation_ResCode_RoutingSuccessful},
      diag_msg_ack_code_{kDoip_DiagnosticMessage_PosAckCode_Confirm},
      num_of_pending_response_{0u} {
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

DoipChannel::~DoipChannel() {
  exit_request_ = true;
  running_ = false;
  cond_var_.notify_all();
  thread_.join();
}

void DoipChannel::Initialize() {
  {
    std::lock_guard<std::mutex> const lck{mutex_};
    job_queue_.emplace([this]() { this->StartAcceptingConnection(); });
    running_ = true;
  }
  cond_var_.notify_all();
}

void DoipChannel::DeInitialize() {
  if (tcp_connection_) { tcp_connection_->DeInitialize(); }
}

void DoipChannel::StartAcceptingConnection() {
  // Get the tcp connection - this will return after client is connected
  tcp_connection_ = std::move(tcp_socket_handler_.CreateTcpConnection([this](TcpMessagePtr tcp_rx_message) {
    // handle message
    this->HandleMessage(std::move(tcp_rx_message));
  }));
  if (tcp_connection_) {
    tcp_connection_->Initialize();
    running_ = false;
  }
}

// Function to transmit the uds message
uds_transport::UdsTransportProtocolMgr::TransmissionResult DoipChannel::Transmit(
  uds_transport::UdsMessageConstPtr message) {
  // tcp_connection_->Transmit(std::move(message));
  // TODO
  return uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk;
}

void DoipChannel::HandleMessage(TcpMessagePtr tcp_rx_message) {
  received_doip_message_.host_ip_address = tcp_rx_message->host_ip_address_;
  received_doip_message_.port_num = tcp_rx_message->host_port_num_;
  received_doip_message_.protocol_version = tcp_rx_message->rxBuffer_[0];
  received_doip_message_.protocol_version_inv = tcp_rx_message->rxBuffer_[1];
  received_doip_message_.payload_type = GetDoIPPayloadType(tcp_rx_message->rxBuffer_);
  received_doip_message_.payload_length = GetDoIPPayloadLength(tcp_rx_message->rxBuffer_);
  if (received_doip_message_.payload_length > 0U) {
    received_doip_message_.payload.insert(received_doip_message_.payload.begin(),
                                          tcp_rx_message->rxBuffer_.begin() + kDoipheadrSize,
                                          tcp_rx_message->rxBuffer_.end());
  }
  // std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr> ret_val{
  //   doip_connection_.IndicateMessage(static_cast<uds_transport::UdsMessage::Address>(0),
  //                                           static_cast<uds_transport::UdsMessage::Address>(0),
  //                                           uds_transport::UdsMessage::TargetAddressType::kPhysical, 0U,
  //                                           static_cast<std::size_t>(received_doip_message_.payload.size() - 4U), 0U,
  //                                           "DoIPTcp", received_doip_message_.payload)};
  // Trigger async transmission
  {
    std::lock_guard<std::mutex> const lck{mutex_};
    job_queue_.emplace([this]() {
      if (received_doip_message_.payload_type == kDoip_RoutingActivation_ReqType) {
        this->SendRoutingActivationResponse();
      } else if (received_doip_message_.payload_type == kDoip_DiagMessage_Type) {
        this->SendDiagnosticMessageAckResponse();
      }
    });
    running_ = true;
  }
  cond_var_.notify_all();
}

auto DoipChannel::GetDoIPPayloadType(std::vector<uint8_t> payload) noexcept -> uint16_t {
  return ((uint16_t) (((payload[BYTE_POS_TWO] & 0xFF) << 8) | (payload[BYTE_POS_THREE] & 0xFF)));
}

auto DoipChannel::GetDoIPPayloadLength(std::vector<uint8_t> payload) noexcept -> uint32_t {
  return ((uint32_t) ((payload[BYTE_POS_FOUR] << 24U) & 0xFF000000) |
          (uint32_t) ((payload[BYTE_POS_FIVE] << 16U) & 0x00FF0000) |
          (uint32_t) ((payload[BYTE_POS_SIX] << 8U) & 0x0000FF00) |
          (uint32_t) ((payload[BYTE_POS_SEVEN] & 0x000000FF)));
}

void DoipChannel::CreateDoipGenericHeader(std::vector<uint8_t> &doipHeader, std::uint16_t payload_type,
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

void DoipChannel::SendRoutingActivationResponse() {
  TcpMessagePtr routing_activation_response{std::make_unique<TcpMessage>()};
  // create header
  routing_activation_response->txBuffer_.reserve(kDoipheadrSize + kDoip_RoutingActivation_ResMinLen);
  CreateDoipGenericHeader(routing_activation_response->txBuffer_, kDoip_RoutingActivation_ResType,
                          kDoip_RoutingActivation_ResMinLen);

  // logical address of client
  routing_activation_response->txBuffer_.emplace_back(received_doip_message_.payload[0]);
  routing_activation_response->txBuffer_.emplace_back(received_doip_message_.payload[1]);
  // logical address of server
  routing_activation_response->txBuffer_.emplace_back(logical_address_ >> 8U);
  routing_activation_response->txBuffer_.emplace_back(logical_address_ & 0xFFU);
  // activation response code
  routing_activation_response->txBuffer_.emplace_back(routing_activation_res_code_);
  routing_activation_response->txBuffer_.emplace_back(0x00);
  routing_activation_response->txBuffer_.emplace_back(0x00);
  routing_activation_response->txBuffer_.emplace_back(0x00);
  routing_activation_response->txBuffer_.emplace_back(0x00);

  if (tcp_connection_->Transmit(std::move(routing_activation_response))) { running_ = false; }
}

void DoipChannel::SendDiagnosticMessageAckResponse() {
  TcpMessagePtr diag_msg_ack_response{std::make_unique<TcpMessage>()};
  // create header
  diag_msg_ack_response->txBuffer_.reserve(kDoipheadrSize + kDoip_DiagMessageAck_ResMinLen);
  if (diag_msg_ack_code_ == kDoip_DiagnosticMessage_PosAckCode_Confirm) {
    CreateDoipGenericHeader(diag_msg_ack_response->txBuffer_, kDoip_DiagMessagePosAck_Type,
                            kDoip_DiagMessageAck_ResMinLen);
  } else {
    CreateDoipGenericHeader(diag_msg_ack_response->txBuffer_, kDoip_DiagMessageNegAck_Type,
                            kDoip_DiagMessageAck_ResMinLen);
  }
  // logical address of client
  diag_msg_ack_response->txBuffer_.emplace_back(logical_address_ >> 8U);
  diag_msg_ack_response->txBuffer_.emplace_back(logical_address_ & 0xFFU);

  // logical address of target
  diag_msg_ack_response->txBuffer_.emplace_back(received_doip_message_.payload[0]);
  diag_msg_ack_response->txBuffer_.emplace_back(received_doip_message_.payload[1]);

  // activation response code
  diag_msg_ack_response->txBuffer_.emplace_back(diag_msg_ack_code_);

  if (tcp_connection_->Transmit(std::move(diag_msg_ack_response))) {
    // Check for diag message ack code
    if (diag_msg_ack_code_ == kDoip_DiagnosticMessage_PosAckCode_Confirm) {
      ::doip_handler::logger::DoipServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
          __FILE__, __LINE__, "",
          [](std::stringstream &msg) { msg << "Sending of Diagnostic Message Pos Ack Response success"; });

      // Check for pending responses set
      if(!uds_pending_response_payload_.empty()) {
        // emplace pending response jobs based on number of pending response
        for(std::uint8_t pending_count{0}; pending_count < num_of_pending_response_; pending_count++) {
          job_queue_.emplace([this]() {
            // wait so that diag positive ack is processed first before sending diag response
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            this->SendDiagnosticPendingMessageResponse();
          });        
        }
      }
      // emplace a positive response
      job_queue_.emplace([this]() {
        // wait so that diag positive ack is processed first before sending diag response
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        this->SendDiagnosticMessageResponse();
      });
      running_ = true;
      cond_var_.notify_all();
    } else {
      ::doip_handler::logger::DoipServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
          __FILE__, __LINE__, "",
          [](std::stringstream &msg) { msg << "Sending of Diagnostic Message Neg Ack Response success"; });
    }
  }
}

void DoipChannel::SendDiagnosticMessageResponse() {
  TcpMessagePtr diag_uds_message_response{std::make_unique<TcpMessage>()};
  // create header
  diag_uds_message_response->txBuffer_.reserve(kDoipheadrSize + kDoip_DiagMessage_ReqResMinLen +
                                               uds_response_payload_.size());
  CreateDoipGenericHeader(diag_uds_message_response->txBuffer_, kDoip_DiagMessage_Type,
                          kDoip_DiagMessage_ReqResMinLen + uds_response_payload_.size());

  // logical address of client
  diag_uds_message_response->txBuffer_.emplace_back(logical_address_ >> 8U);
  diag_uds_message_response->txBuffer_.emplace_back(logical_address_ & 0xFFU);

  // logical address of target
  diag_uds_message_response->txBuffer_.emplace_back(received_doip_message_.payload[0]);
  diag_uds_message_response->txBuffer_.emplace_back(received_doip_message_.payload[1]);

  // copy the payload
  diag_uds_message_response->txBuffer_.insert(
      diag_uds_message_response->txBuffer_.begin() + kDoipheadrSize + kDoip_DiagMessage_ReqResMinLen,
      uds_response_payload_.begin(), uds_response_payload_.end());

  if (tcp_connection_->Transmit(std::move(diag_uds_message_response))) {
    running_ = false;
    ::doip_handler::logger::DoipServerLogger::GetDiagServerLogger().GetLogger().LogInfo(__FILE__, __LINE__, "", [](std::stringstream &msg) {
      msg << "Sending of Diagnostic Response message success";
    });
  }
}


void DoipChannel::SendDiagnosticPendingMessageResponse() {
  TcpMessagePtr diag_uds_message_response{std::make_unique<TcpMessage>()};
  // create header
  diag_uds_message_response->txBuffer_.reserve(kDoipheadrSize + kDoip_DiagMessage_ReqResMinLen +
                                               uds_pending_response_payload_.size());
  CreateDoipGenericHeader(diag_uds_message_response->txBuffer_, kDoip_DiagMessage_Type,
                          kDoip_DiagMessage_ReqResMinLen + uds_pending_response_payload_.size());

  // logical address of client
  diag_uds_message_response->txBuffer_.emplace_back(logical_address_ >> 8U);
  diag_uds_message_response->txBuffer_.emplace_back(logical_address_ & 0xFFU);

  // logical address of target
  diag_uds_message_response->txBuffer_.emplace_back(received_doip_message_.payload[0]);
  diag_uds_message_response->txBuffer_.emplace_back(received_doip_message_.payload[1]);

  // copy the payload
  diag_uds_message_response->txBuffer_.insert(
      diag_uds_message_response->txBuffer_.begin() + kDoipheadrSize + kDoip_DiagMessage_ReqResMinLen,
      uds_pending_response_payload_.begin(), uds_pending_response_payload_.end());

  if (tcp_connection_->Transmit(std::move(diag_uds_message_response))) {
    running_ = false;
    ::doip_handler::logger::DoipServerLogger::GetDiagServerLogger().GetLogger().LogInfo(__FILE__, __LINE__, "", [](std::stringstream &msg) {
      msg << "Sending of Diagnostic Pending Response message success";
    });
  }
}

void DoipChannel::SetExpectedRoutingActivationResponseToBeSent(
    std::uint8_t routing_activation_res_code) {
  routing_activation_res_code_ = routing_activation_res_code;
}

void DoipChannel::SetExpectedDiagnosticMessageAckResponseToBeSend(std::uint8_t diag_msg_ack_code) {
  diag_msg_ack_code_ = diag_msg_ack_code;
}

void DoipChannel::SetExpectedDiagnosticMessageUdsMessageToBeSend(std::vector<std::uint8_t> payload) {
  uds_response_payload_.clear();
  uds_response_payload_ = std::move(payload);
}

void DoipChannel::SetExpectedDiagnosticMessageWithPendingUdsMessageToBeSend(
  std::vector<std::uint8_t> payload, 
  std::uint8_t num_of_pending_response) {
  uds_pending_response_payload_.clear();
  uds_pending_response_payload_ = std::move(payload);
  num_of_pending_response_ = num_of_pending_response;
}

}  // namespace doip_handler
}  // namespace doip_server