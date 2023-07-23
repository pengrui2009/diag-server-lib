/* Diagnostic Server library
* Copyright (C) 2023  Avijit Dey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef DIAG_SERVER_DOIP_TCP_HANDLER_H
#define DIAG_SERVER_DOIP_TCP_HANDLER_H

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string_view>

#include "common/doip_payload_type.h"
#include "sockets/tcp_socket_handler.h"
#include "uds_transport/protocol_mgr.h"

//forward declaration
namespace doip_server {
namespace connection {
class DoipTcpConnection;
}
}

namespace doip_handler {

using TcpMessage = tcpSocket::TcpMessage;
using TcpMessagePtr = tcpSocket::TcpMessagePtr;
using TcpMessageConstPtr = tcpSocket::TcpMessageConstPtr;

class DoipTcpHandler {
public:
  using TcpConnectionHandler = tcpSocket::DoipTcpSocketHandler::TcpConnectionHandler;
  using DoipChannelReadCallback = tcpSocket::DoipTcpSocketHandler::TcpHandlerRead;

  // Class maintaining the doip channel
  class DoipChannel {
  public:
    DoipChannel(std::uint16_t logical_address, tcpSocket::DoipTcpSocketHandler &tcp_socket_handler);

    ~DoipChannel();

    // Initialize
    void Initialize();

    // De-Initialize
    void DeInitialize();

    // Function to transmit the uds message
    uds_transport::UdsTransportProtocolMgr::TransmissionResult Transmit(
      uds_transport::UdsMessageConstPtr message);

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

  private:
    // Store the logical address
    std::uint16_t logical_address_;

    // store the tcp socket handler reference
    tcpSocket::DoipTcpSocketHandler &tcp_socket_handler_;

    // Tcp connection to handler tcp req and response
    std::unique_ptr<TcpConnectionHandler> tcp_connection_;

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
    void SendRoutingActivationResponse();

    // Function to trigger transmission diag ack response
    void SendDiagnosticMessageAckResponse();

    // Function to trigger transmission of diag uds message
    void SendDiagnosticMessageResponse();
    
    // Function to send diagnostic pending response
    void SendDiagnosticPendingMessageResponse();
  };

public:
  // ctor
  DoipTcpHandler(std::string_view local_tcp_address, std::uint16_t tcp_port_num, 
    doip_server::connection::DoipTcpConnection &doip_connection);

  // dtor
  ~DoipTcpHandler();

  // Function to transmit the uds message
  uds_transport::UdsTransportProtocolMgr::TransmissionResult Transmit(
    uds_transport::UdsMessageConstPtr message, std::uint16_t logical_address);

  // Function to create doip channel
  DoipChannel &CreateDoipChannel(std::uint16_t logical_address);

private:
  // reference to doip connection
  doip_server::connection::DoipTcpConnection &doip_connection_;

  // tcp socket handler
  std::unique_ptr<tcpSocket::DoipTcpSocketHandler> tcp_socket_handler_;

  // list of doip channel
  std::map<std::uint16_t, std::unique_ptr<DoipChannel>> doip_channel_list_;
};

}  // namespace doip_handler

#endif  //DIAG_SERVER_DOIP_TCP_HANDLER_H
