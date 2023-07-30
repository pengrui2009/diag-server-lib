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
// #include "common/common_header.h"
#include "common/common_doip_types.h"
#include "common/logger.h"

namespace doip_server {
namespace doip_handler {


auto RoutingActivationHandler::ProcessDoIPRoutingActivationRequest(DoipMessage &doip_payload) noexcept -> void {

}

auto RoutingActivationHandler::SendRoutingActivationResponse(uds_transport::UdsMessageConstPtr &message) noexcept
      -> uds_transport::UdsTransportProtocolMgr::TransmissionResult {
  return uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk;
}

void RoutingActivationHandler::CreateDoipGenericHeader(std::vector<uint8_t> &doipHeader, uint16_t payloadType, 
  uint32_t payloadLen) {
    doipHeader.push_back(kDoip_ProtocolVersion);
    doipHeader.push_back(~((uint8_t) kDoip_ProtocolVersion));
    doipHeader.push_back((uint8_t) ((payloadType & 0xFF00) >> 8));
    doipHeader.push_back((uint8_t) (payloadType & 0x00FF));
    doipHeader.push_back((uint8_t) ((payloadLen & 0xFF000000) >> 24));
    doipHeader.push_back((uint8_t) ((payloadLen & 0x00FF0000) >> 16));
    doipHeader.push_back((uint8_t) ((payloadLen & 0x0000FF00) >> 8));
    doipHeader.push_back((uint8_t) (payloadLen & 0x000000FF));
}

// auto DiagnosticMessageHandler::ProcessDoIPDiagnosticAckMessageResponse(DoipMessage &doip_payload) 
//     noexcept -> void {

// }

auto DiagnosticMessageHandler::ProcessDoIPDiagnosticMessageRequest(DoipMessage &doip_payload) 
    noexcept -> void {
    
    std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr> ret_val{
            tcp_transport_handler_.IndicateMessage(static_cast<uds_transport::UdsMessage::Address>(0),
                            static_cast<uds_transport::UdsMessage::Address>(0),
                            uds_transport::UdsMessage::TargetAddressType::kPhysical, 0U,
                            static_cast<std::size_t>(doip_payload.payload.size() - 4U), 0U,
                            "DoIPTcp", doip_payload.payload)};
}

auto DiagnosticMessageHandler::SendDiagnosticResponse(uds_transport::UdsMessageConstPtr &message) 
    noexcept -> uds_transport::UdsTransportProtocolMgr::TransmissionResult {
    return uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk;
}

auto DiagnosticMessageHandler::SendDiagnosticAckResponse(uds_transport::UdsMessageConstPtr &message) noexcept
    -> uds_transport::UdsTransportProtocolMgr::TransmissionResult {
    
    return uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk;
}

auto DiagnosticMessageHandler::CreateDoipGenericHeader(std::vector<uint8_t> &doipHeader, 
    uint16_t payloadType, uint32_t payloadLen) noexcept -> void {

}


DoipChannelHandlerImpl::DoipChannelHandlerImpl(::doip_handler::tcpSocket::DoipTcpSocketHandler &tcp_socket_handler, 
  DoipChannel &tcp_channle, DoipTcpHandler &tcp_transport_handler) :
    doip_channle_{tcp_channle},
    routing_activation_handler_(tcp_socket_handler, *this),
    diagnostic_message_handler_(tcp_socket_handler, tcp_transport_handler, *this) {

}

DoipChannelHandlerImpl::~DoipChannelHandlerImpl() {

}

auto DoipChannelHandlerImpl::HandleMessage(TcpMessagePtr tcp_rx_message) noexcept -> void {

}

auto DoipChannelHandlerImpl::SendRoutingActivationResponse(uds_transport::UdsMessageConstPtr &message) noexcept
    -> uds_transport::UdsTransportProtocolMgr::TransmissionResult {
      return uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk;
}

// Function to send Diagnostic request
auto DoipChannelHandlerImpl::SendDiagnosticResponse(uds_transport::UdsMessageConstPtr &message) noexcept
    -> uds_transport::UdsTransportProtocolMgr::TransmissionResult {
      return uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk;
}

// Function to process DoIP Header
auto DoipChannelHandlerImpl::ProcessDoIPHeader(DoipMessage &doip_rx_message, uint8_t &nackCode) 
  noexcept -> bool {
    return true;
}

// Function to verify payload length of various payload type
auto DoipChannelHandlerImpl::ProcessDoIPPayloadLength(uint32_t payloadLen, uint16_t payloadType) 
  noexcept -> bool {
    return true;
}

// Function to get payload type
auto DoipChannelHandlerImpl::GetDoIPPayloadType(std::vector<uint8_t> payload) 
  noexcept -> uint16_t {
    return 0;
}

// Function to get payload length
auto DoipChannelHandlerImpl::GetDoIPPayloadLength(std::vector<uint8_t> payload) 
  noexcept -> uint32_t {
    return 0;
}

// Function to process DoIP payload responses
auto DoipChannelHandlerImpl::ProcessDoIPPayload(DoipMessage &doip_payload) 
  noexcept -> void {

}

DoipChannel &DoipTcpHandler::CreateDoipChannel(std::uint16_t logical_address, connection::DoipTcpConnection &tcp_connection) {
  // create new doip channel
  doip_channel_list_.emplace(logical_address,
                             std::make_unique<DoipChannel>(tcp_connection, logical_address, *tcp_socket_handler_, *this));
  return *doip_channel_list_[logical_address];
}

DoipChannel::DoipChannel(connection::DoipTcpConnection &tcp_connection, std::uint16_t logical_address, 
                         ::doip_handler::tcpSocket::DoipTcpSocketHandler &tcp_socket_handler,
                         DoipTcpHandler &tcp_transport_handler)
    : logical_address_{logical_address},
      tcp_socket_handler_{tcp_socket_handler},
      tcp_connection_{tcp_connection},
      tcp_connection_handler_{},
      doip_channel_handle_impl_{std::make_unique<DoipChannelHandlerImpl>(tcp_socket_handler_, *this, tcp_transport_handler)},
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
  if (tcp_connection_handler_) { 
    tcp_connection_handler_->DeInitialize(); 
  }
}

void DoipChannel::StartAcceptingConnection() {
  // Get the tcp connection - this will return after client is connected
  tcp_connection_handler_ = std::move(tcp_socket_handler_.CreateTcpConnection(
    [this](TcpMessagePtr tcp_rx_message) {
      // handle message
      this->HandleMessage(std::move(tcp_rx_message));
  }));
  if (tcp_connection_handler_) {
    tcp_connection_handler_->Initialize();
    running_ = false;
  }
}

// Function to transmit the uds message
uds_transport::UdsTransportProtocolMgr::TransmissionResult DoipChannel::Transmit(
  uds_transport::UdsMessageConstPtr message) {
  // TODO
  return tcp_connection_.Transmit(std::move(message));
  // return uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk;
}

std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr>
      DoipChannel::IndicateMessage(uds_transport::UdsMessage::Address source_addr,
                                   uds_transport::UdsMessage::Address target_addr,
                                   uds_transport::UdsMessage::TargetAddressType type,
                                   uds_transport::ChannelID channel_id, std::size_t size,
                                   uds_transport::Priority priority, uds_transport::ProtocolKind protocol_kind,
                                   std::vector<uint8_t> payloadInfo) {
  std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr> ret = {
    uds_transport::UdsTransportProtocolMgr::IndicationResult::kIndicationNOk, nullptr};
  
  tcp_connection_.IndicateMessage(static_cast<uds_transport::UdsMessage::Address>(0),
                                    static_cast<uds_transport::UdsMessage::Address>(0),
                                    uds_transport::UdsMessage::TargetAddressType::kPhysical, 0U,
                                    static_cast<std::size_t>(received_doip_message_.payload.size() - 4U), 0U,
                                    "DoIPTcp", received_doip_message_.payload);

  return ret;
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
  
  doip_channel_handle_impl_->HandleMessage(std::move(tcp_rx_message));
  // Trigger async transmission
  // {
  //   std::lock_guard<std::mutex> const lck{mutex_};
  //   job_queue_.emplace([this]() {
  //     if (received_doip_message_.payload_type == kDoip_RoutingActivation_ReqType) {
        
  //       this->SendRoutingActivationResponse(received_doip_message_);
  //     } else if (received_doip_message_.payload_type == kDoip_DiagMessage_Type) {
  //       this->SendDiagnosticMessageAckResponse();        
  //       std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr> ret_val{
  //           IndicateMessage(static_cast<uds_transport::UdsMessage::Address>(0),
  //                           static_cast<uds_transport::UdsMessage::Address>(0),
  //                           uds_transport::UdsMessage::TargetAddressType::kPhysical, 0U,
  //                           static_cast<std::size_t>(received_doip_message_.payload.size() - 4U), 0U,
  //                           "DoIPTcp", received_doip_message_.payload)};
  //     }
  //   });
  //   running_ = true;
  // }
  // cond_var_.notify_all();
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

void DoipChannel::SendRoutingActivationResponse(const DoipMessage &msg) {
  // Routing activation response code
  std::uint8_t routing_activation_res_code;
  uint16_t target_logical_address = (msg.payload[0] << 8) + msg.payload[0];
  TcpMessagePtr routing_activation_response{std::make_unique<TcpMessage>()};

  if (msg.payload.size() < kDoip_RoutingActivation_ReqMinLen) {
    routing_activation_res_code = kDoip_RoutingActivation_ResCode_ConfirmtnRejectd;
  } else if (msg.payload.size() > kDoip_RoutingActivation_ReqMaxLen) {
    routing_activation_res_code = kDoip_RoutingActivation_ResCode_ConfirmtnRejectd;
  } else if (logical_address_ == target_logical_address) {
    routing_activation_res_code = kDoip_RoutingActivation_ResCode_ConfirmtnRequired;
  }

  // create header
  routing_activation_response->txBuffer_.reserve(kDoipheadrSize + kDoip_RoutingActivation_ResMinLen);
  CreateDoipGenericHeader(routing_activation_response->txBuffer_, kDoip_RoutingActivation_ResType,
                          kDoip_RoutingActivation_ResMinLen);

  // logical address of client
  routing_activation_response->txBuffer_.emplace_back(msg.payload[0]);
  routing_activation_response->txBuffer_.emplace_back(msg.payload[1]);
  // logical address of server
  routing_activation_response->txBuffer_.emplace_back(logical_address_ >> 8U);
  routing_activation_response->txBuffer_.emplace_back(logical_address_ & 0xFFU);
  // activation response code
  routing_activation_response->txBuffer_.emplace_back(routing_activation_res_code_);
  routing_activation_response->txBuffer_.emplace_back(0x00);
  routing_activation_response->txBuffer_.emplace_back(0x00);
  routing_activation_response->txBuffer_.emplace_back(0x00);
  routing_activation_response->txBuffer_.emplace_back(0x00);

  if (tcp_connection_handler_->Transmit(std::move(routing_activation_response))) { 
    running_ = false; 
  }
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

  if (tcp_connection_handler_->Transmit(std::move(diag_msg_ack_response))) {
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

  if (tcp_connection_handler_->Transmit(std::move(diag_uds_message_response))) {
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

  if (tcp_connection_handler_->Transmit(std::move(diag_uds_message_response))) {
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

DoipTcpHandler::DoipTcpHandler(std::string_view local_tcp_address, uint16_t tcp_port_num)
    : tcp_socket_handler_{std::make_unique<::doip_handler::tcpSocket::DoipTcpSocketHandler>(local_tcp_address, tcp_port_num)} {

}

DoipTcpHandler::~DoipTcpHandler() = default;

auto DoipTcpHandler::HandleMessage(TcpMessagePtr tcp_rx_message) noexcept -> void {
  DoipMessage received_doip_message_;
  received_doip_message_.host_ip_address = tcp_rx_message->host_ip_address_;
  received_doip_message_.port_num = tcp_rx_message->host_port_num_;
  received_doip_message_.protocol_version = tcp_rx_message->rxBuffer_[0];
  received_doip_message_.protocol_version_inv = tcp_rx_message->rxBuffer_[1];
  // received_doip_message_.payload_type = GetDoIPPayloadType(tcp_rx_message->rxBuffer_);
  // received_doip_message_.payload_length = GetDoIPPayloadLength(tcp_rx_message->rxBuffer_);
  if (received_doip_message_.payload_length > 0U) {
    received_doip_message_.payload.insert(received_doip_message_.payload.begin(),
                                          tcp_rx_message->rxBuffer_.begin() + kDoipheadrSize,
                                          tcp_rx_message->rxBuffer_.end());
  }
  // Trigger async transmission
  {
    // std::lock_guard<std::mutex> const lck{mutex_};
    // job_queue_.emplace([this]() {
    //   if (received_doip_message_.payload_type == kDoip_RoutingActivation_ReqType) {
    //     routing_activation_handler_.ProcessDoIPRoutingActivationRequest();
    //     // this->SendRoutingActivationResponse(received_doip_message_);
    //   } else if (received_doip_message_.payload_type == kDoip_DiagMessage_Type) {
    //     // this->SendDiagnosticMessageAckResponse();
    //     diagnostic_message_handler_.ProcessDoIPDiagnosticMessageRequest();
        
    //   }
    // });
    // running_ = true;
  }
  // cond_var_.notify_all();
}

std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr>
  DoipTcpHandler::IndicateMessage(uds_transport::UdsMessage::Address source_addr,
                                uds_transport::UdsMessage::Address target_addr,
                                uds_transport::UdsMessage::TargetAddressType type,
                                uds_transport::ChannelID channel_id, std::size_t size,
                                uds_transport::Priority priority, uds_transport::ProtocolKind protocol_kind,
                                std::vector<uint8_t> payloadInfo) {
  doip_channel_list_[source_addr]->IndicateMessage(source_addr, target_addr, type, channel_id, 
      size, priority, protocol_kind, payloadInfo);
    return {uds_transport::UdsTransportProtocolMgr::IndicationResult::kIndicationNOk, nullptr};
}

uds_transport::UdsTransportProtocolMgr::TransmissionResult DoipTcpHandler::Transmit(
    uds_transport::UdsMessageConstPtr message, std::uint16_t logical_address) {
    return (doip_channel_list_[logical_address]->Transmit(std::move(message)));  
}


}  // namespace doip_handler
}  // namespace doip_server