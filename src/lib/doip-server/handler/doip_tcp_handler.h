/* Diagnostic Server library
* Copyright (C) 2023  Rui Peng
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_HANDLER_DOIP_TCP_HANDLER_H_
#define DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_HANDLER_DOIP_TCP_HANDLER_H_

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string_view>

#include "common/doip_payload_type.h"
#include "sockets/tcp_socket_handler.h"
#include "uds_transport/protocol_mgr.h"
#include "uds_transport/conversion_handler.h"


//forward declaration
namespace doip_server {
namespace connection {
class DoipTcpConnection;
}

namespace doip_handler {

using TcpMessage = ::doip_handler::tcpSocket::TcpMessage;
using TcpMessagePtr = ::doip_handler::tcpSocket::TcpMessagePtr;
using TcpMessageConstPtr = ::doip_handler::tcpSocket::TcpMessageConstPtr;

class DoipChannel;
class DoipTcpHandler;
class DoipChannelHandlerImpl;
/*
 @ Class Name        : RoutingActivationHandler
 @ Class Description : Class used as a handler to process routing activation messages
 */
class RoutingActivationHandler {
public:
  // strong type holding activation type
  struct RoutingActivationAckType {
    std::uint8_t act_type_;
  };

public:
  // ctor
  RoutingActivationHandler(DoipChannel &tcp_channel, DoipChannelHandlerImpl &tcp_channel_handler_impl)
      : tcp_channel_{tcp_channel},
        tcp_channel_handler_impl_{tcp_channel_handler_impl} {}

  // dtor
  ~RoutingActivationHandler() = default;

  // Function to process Routing activation request
  auto ProcessDoIPRoutingActivationRequest(DoipMessage &doip_payload) noexcept -> void;

  // Function to send Routing activation response
  auto SendRoutingActivationResponse(uds_transport::UdsMessageConstPtr &message) noexcept
      -> uds_transport::UdsTransportProtocolMgr::TransmissionResult;

private:
  void CreateDoipGenericHeader(std::vector<uint8_t> &doipHeader, uint16_t payloadType, uint32_t payloadLen);

private:
  // socket reference
  // ::doip_handler::tcpSocket::DoipTcpSocketHandler &tcp_socket_handler_;
  // channel reference
  DoipChannelHandlerImpl &tcp_channel_handler_impl_;
  // 
  DoipChannel &tcp_channel_;
};

/*
 @ Class Name        : DiagnosticMessageHandler
 @ Class Description : Class used as a handler to process diagnostic messages
 */
class DiagnosticMessageHandler {
public:
  // strong type acknowledgement type
  struct DiagAckType {
    std::uint8_t ack_type_;
  };

public:
  // ctor
  DiagnosticMessageHandler(DoipChannel &tcp_channel, DoipChannelHandlerImpl &tcp_channel_handler_impl)
      : tcp_channel_{tcp_channel},
        tcp_channel_handler_impl_{tcp_channel_handler_impl} {}

  // dtor
  ~DiagnosticMessageHandler() = default;

  // Function to process Routing activation response
  // auto ProcessDoIPDiagnosticAckMessageResponse(DoipMessage &doip_payload) noexcept -> void;

  // Function to process Diagnostic message request
  auto ProcessDoIPDiagnosticMessageRequest(DoipMessage &doip_payload) noexcept -> void;

  // Function to send Diagnostic response
  auto SendDiagnosticResponse(uds_transport::UdsMessageConstPtr &message) noexcept
      -> uds_transport::UdsTransportProtocolMgr::TransmissionResult;

  auto SendDiagnosticAckResponse(uds_transport::UdsMessageConstPtr &message) noexcept
      -> uds_transport::UdsTransportProtocolMgr::TransmissionResult;
private:
  static auto CreateDoipGenericHeader(std::vector<uint8_t> &doipHeader, uint16_t payloadType,
                                      uint32_t payloadLen) noexcept -> void;

  // socket reference
  // ::doip_handler::tcpSocket::DoipTcpSocketHandler &tcp_socket_handler_;
  // transport handler reference
  DoipChannelHandlerImpl &tcp_channel_handler_impl_;
  // channel reference
  DoipChannel &tcp_channel_;
  // DoipTcpHandler &tcp_transport_handler_;
};

class DoipChannel;

class DoipChannelHandlerImpl {
public:
  using TcpConnectionHandler = ::doip_handler::tcpSocket::DoipTcpSocketHandler::TcpConnectionHandler;
  using DoipChannelReadCallback = ::doip_handler::tcpSocket::DoipTcpSocketHandler::TcpHandlerRead;  

public:
  // ctor
  DoipChannelHandlerImpl(DoipChannel &tcp_channle_, DoipTcpHandler &tcp_transport_handler);

  // dtor
  ~DoipChannelHandlerImpl();

  // process message
  auto HandleMessage(TcpMessagePtr tcp_rx_message) noexcept -> void;
  
  // std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr>
  //   IndicateMessage(uds_transport::UdsMessage::Address source_addr,
  //                                 uds_transport::UdsMessage::Address target_addr,
  //                                 uds_transport::UdsMessage::TargetAddressType type,
  //                                 uds_transport::ChannelID channel_id, std::size_t size,
  //                                 uds_transport::Priority priority, uds_transport::ProtocolKind protocol_kind,
  //                                 std::vector<uint8_t> payloadInfo);

  // // Function to transmit the uds message
  // uds_transport::UdsTransportProtocolMgr::TransmissionResult Transmit(
  //   uds_transport::UdsMessageConstPtr message, std::uint16_t logical_address);

  // Function to create doip channel
  // DoipChannel &CreateDoipChannel(std::uint16_t logical_address, connection::DoipTcpConnection &tcp_connection);
  // Function to trigger Routing activation request
  auto SendRoutingActivationResponse(uds_transport::UdsMessageConstPtr &message) noexcept
      -> uds_transport::UdsTransportProtocolMgr::TransmissionResult;

  // Function to send Diagnostic request
  auto SendDiagnosticResponse(uds_transport::UdsMessageConstPtr &message) noexcept
      -> uds_transport::UdsTransportProtocolMgr::TransmissionResult;
private:
  // reference to doip connection
  // doip_server::connection::DoipTcpConnection &doip_connection_;
  // Store the conversion
  // const std::shared_ptr<ConversionHandler> &conversation_;
    // Function to process DoIP Header
  auto ProcessDoIPHeader(DoipMessage &doip_rx_message, uint8_t &nackCode) noexcept -> bool;

  // Function to verify payload length of various payload type
  auto ProcessDoIPPayloadLength(uint32_t payloadLen, uint16_t payloadType) noexcept -> bool;

  // Function to get payload type
  auto GetDoIPPayloadType(std::vector<uint8_t> payload) noexcept -> uint16_t;

  // Function to get payload length
  auto GetDoIPPayloadLength(std::vector<uint8_t> payload) noexcept -> uint32_t;

  // Function to process DoIP payload responses
  auto ProcessDoIPPayload(DoipMessage &doip_payload) noexcept -> void;


  // tcp socket handler
  // std::unique_ptr<::doip_handler::tcpSocket::DoipTcpSocketHandler> tcp_socket_handler_;

  // list of doip channel
  // std::map<std::uint16_t, std::unique_ptr<DoipChannel>> doip_channel_list_;
  DoipChannel& doip_channle_;

  // handler to process routing activation req/ resp
  RoutingActivationHandler routing_activation_handler_;
  
  // handler to process diagnostic message req/ resp
  DiagnosticMessageHandler diagnostic_message_handler_;
};

// Class maintaining the doip channel
class DoipChannel {
public:
  using TcpConnectionHandler = ::doip_handler::tcpSocket::DoipTcpSocketHandler::TcpConnectionHandler;
  using DoipChannelReadCallback = ::doip_handler::tcpSocket::DoipTcpSocketHandler::TcpHandlerRead;

public:
    DoipChannel(connection::DoipTcpConnection &tcp_connection, std::uint16_t logical_address, 
      ::doip_handler::tcpSocket::DoipTcpSocketHandler &tcp_socket_handler, 
      DoipTcpHandler &tcp_transport_handler);

    ~DoipChannel();

    // Initialize
    void Initialize();

    // De-Initialize
    void DeInitialize();

    // 
    bool IsAlive();
    
    // Function to transmit the uds message
    uds_transport::UdsTransportProtocolMgr::TransmissionResult Transmit(
      TcpMessagePtr message);

    uds_transport::UdsTransportProtocolMgr::TransmissionResult Transmit(
      uds_transport::UdsMessageConstPtr message);
    
    std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr>
      IndicateMessage(uds_transport::UdsMessage::Address source_addr,
                                   uds_transport::UdsMessage::Address target_addr,
                                   uds_transport::UdsMessage::TargetAddressType type,
                                   uds_transport::ChannelID channel_id, std::size_t size,
                                   uds_transport::Priority priority, uds_transport::ProtocolKind protocol_kind,
                                   std::vector<uint8_t> payloadInfo);

    // Function to Hand over all the message received
    void HandleMessage(TcpMessagePtr tcp_rx_message);

    // Set expected Routing Activation response
    void SetExpectedRoutingActivationResponseToBeSent(std::uint8_t routing_activation_res_code);

    // Set expected Diagnostic Message Acknowledgment response
    void SetExpectedDiagnosticMessageAckResponseToBeSend(std::uint8_t diag_msg_ack_code);

    // Set expected Diagnostic Uds Message
    void SetExpectedDiagnosticMessageUdsMessageToBeSend(std::vector<std::uint8_t> payload);

    // Set expected Diagnostic Pending Response Uds message
    void SetExpectedDiagnosticMessageWithPendingUdsMessageToBeSend(std::vector<std::uint8_t> payload, std::uint8_t num_of_pending_response);

    bool GetConnectState();
  private:
    // Store the logical address
    std::uint16_t logical_address_;

    connection::DoipTcpConnection &tcp_connection_;

    // 
    // const std::shared_ptr<uds_transport::ConversionHandler> &conversation_;
    
    // store the tcp socket handler reference
    ::doip_handler::tcpSocket::DoipTcpSocketHandler &tcp_socket_handler_;

    // Tcp connection to handler tcp req and response
    std::unique_ptr<TcpConnectionHandler> tcp_connection_handler_;

    // flag to terminate the thread
    std::atomic_bool exit_request_;

    // flag th start the thread
    std::atomic_bool running_;

    // conditional variable to block the thread
    std::condition_variable cond_var_;

    // threading var
    std::thread thread_;

    // locking critical section
    std::mutex mutex_;

    // queue to hold task
    std::queue<std::function<void(void)>> job_queue_;

    // Received doip message
    DoipMessage received_doip_message_{};

    // Routing activation response code
    std::uint8_t routing_activation_res_code_;

    // Diag message ack response code
    std::uint8_t diag_msg_ack_code_;

    // Total number of pending response
    std::uint8_t num_of_pending_response_;

    // Diag message uds payload
    std::vector<std::uint8_t> uds_response_payload_;

    // Diag message uds pending payload
    std::vector<std::uint8_t> uds_pending_response_payload_;
  private:
    // Function invoked during reception
    // void HandleMessage(TcpMessagePtr tcp_rx_message);

    // Start accepting connection from client
    void StartAcceptingConnection();

    // Function to get payload type
    static auto GetDoIPPayloadType(std::vector<uint8_t> payload) noexcept -> uint16_t;

    // Function to get payload length
    static auto GetDoIPPayloadLength(std::vector<uint8_t> payload) noexcept -> uint32_t;

    // Function to create the generic header
    static void CreateDoipGenericHeader(std::vector<uint8_t> &doipHeader, std::uint16_t payload_type,
                                        std::uint32_t payload_len);

    // Function to trigger transmission routing activation response
    void SendRoutingActivationResponse(const DoipMessage &msg);

    // Function to trigger transmission diag ack response
    void SendDiagnosticMessageAckResponse();

    // Function to trigger transmission of diag uds message
    void SendDiagnosticMessageResponse();
    
    // Function to send diagnostic pending response
    void SendDiagnosticPendingMessageResponse();    

    std::unique_ptr<DoipChannelHandlerImpl> doip_channel_handle_impl_;

};


class DoipTcpHandler {
public:
  using TcpConnectionHandler = ::doip_handler::tcpSocket::DoipTcpSocketHandler::TcpConnectionHandler;
  using DoipChannelReadCallback = ::doip_handler::tcpSocket::DoipTcpSocketHandler::TcpHandlerRead;  

public:
  // ctor
  DoipTcpHandler(std::string_view local_tcp_address, uint16_t tcp_port_num);

  // dtor
  ~DoipTcpHandler();

  // process message
  auto HandleMessage(TcpMessagePtr tcp_rx_message) noexcept -> void;
  
  // std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr>
  //   IndicateMessage(uds_transport::UdsMessage::Address source_addr,
  //                                 uds_transport::UdsMessage::Address target_addr,
  //                                 uds_transport::UdsMessage::TargetAddressType type,
  //                                 uds_transport::ChannelID channel_id, std::size_t size,
  //                                 uds_transport::Priority priority, uds_transport::ProtocolKind protocol_kind,
  //                                 std::vector<uint8_t> payloadInfo);

  // Function to transmit the uds message
  // uds_transport::UdsTransportProtocolMgr::TransmissionResult Transmit(
  //   uds_transport::UdsMessageConstPtr message, std::uint16_t logical_address);

  // Function to create doip channel
  DoipChannel &CreateDoipChannel(std::uint16_t logical_address, connection::DoipTcpConnection &tcp_connection);

private:
  // reference to doip connection
  // doip_server::connection::DoipTcpConnection &doip_connection_;
  // Store the conversion
  // const std::shared_ptr<ConversionHandler> &conversation_;
  // 
  // tcp socket handler
  std::unique_ptr<::doip_handler::tcpSocket::DoipTcpSocketHandler> tcp_socket_handler_;

  // list of doip channel
  std::map<std::uint16_t, std::unique_ptr<DoipChannel>> doip_channel_list_;
};

}  // namespace doip_handler
}  // namespace doip_server

#endif  //DIAGNOSTIC_SERVER_LIB_LIB_DOIP_SERVER_HANDLER_DOIP_TCP_HANDLER_H_
