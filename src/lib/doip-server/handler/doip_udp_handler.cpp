/* Diagnostic Server library
* Copyright (C) 2023  Avijit Dey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include <algorithm>
#include <utility>

#include "handler/doip_udp_handler.h"

#include "common/common_doip_types.h"
#include "uds_transport/uds_message.h"
#include "connection/connection_manager.h"

#include "common/logger.h"

namespace doip_server {
namespace doip_handler {

// std::string ConvertToHexString(std::uint8_t char_start, std::uint8_t char_count,
//                                std::vector<std::uint8_t>& input_buffer) {
//   std::string hex_string{};
//   std::uint8_t total_char_count{static_cast<uint8_t>(char_start + char_count)};

//   for (std::uint8_t char_start_count = char_start; char_start_count < total_char_count; char_start_count++) {
//     std::stringstream vehicle_info_data_eid{};
//     int payload_byte{input_buffer[char_start_count]};
//     if ((payload_byte <= 15U)) {
//       // "0" appended in case of value upto 15/0xF
//       vehicle_info_data_eid << "0";
//     }
//     vehicle_info_data_eid << std::hex << payload_byte << ":";
//     hex_string.append(vehicle_info_data_eid.str());
//   }
//   hex_string.pop_back();  // remove last ":" appended before
//   return hex_string;
// }

// std::string ConvertToAsciiString(std::uint8_t char_start, std::uint8_t char_count,
//                                  std::vector<std::uint8_t>& input_buffer) {
//   std::string ascii_string{};
//   std::uint8_t total_char_count{static_cast<uint8_t>(char_start + char_count)};

//   for (std::uint8_t char_start_count = char_start; char_start_count < total_char_count; char_start_count++) {
//     std::stringstream vehicle_info_data_vin{};
//     vehicle_info_data_vin << input_buffer[char_start_count];
//     ascii_string.append(vehicle_info_data_vin.str());
//   }
//   return ascii_string;
// }

// void SerializeEIDGIDFromString(std::string& input_string, std::vector<uint8_t>& output_buffer, std::uint8_t total_size,
//                                std::uint8_t substring_range) {

//   for (auto char_count = 0U; char_count < total_size; char_count += substring_range) {
//     std::string input_string_new{input_string.substr(char_count, static_cast<std::uint8_t>(substring_range))};
//     std::stringstream input_string_stream{input_string_new};
//     int get_byte;
//     input_string_stream >> std::hex >> get_byte;
//     output_buffer.emplace_back(static_cast<std::uint8_t>(get_byte));
//   }
// }

// void SerializeVINFromString(std::string& input_string, std::vector<uint8_t>& output_buffer, std::uint8_t total_size,
//                             std::uint8_t substring_range) {

//   for (auto char_count = 0U; char_count < total_size; char_count += substring_range) {
//     std::string input_string_new{input_string.substr(char_count, static_cast<std::uint8_t>(substring_range))};
//     std::stringstream input_string_stream{input_string_new};
//     int get_byte{input_string_stream.get()};
//     output_buffer.emplace_back(static_cast<std::uint8_t>(get_byte));
//   }
// }

DoipUdpHandler::DoipUdpHandler(std::string_view broadcast_udp_address, uint16_t broadcast_port_num,
    std::string_view unicast_udp_address, uint16_t unicast_port_num,
    doip_server::connection::DoipUdpConnection &doip_connection)
    : doip_connection_(doip_connection), 
      udp_socket_handler_unicast_{unicast_udp_address, unicast_port_num,
                                  ::doip_handler::udpSocket::DoipUdpSocketHandler::PortType::kUdp_Unicast,
                                  [this](UdpMessagePtr udp_rx_message) {
                                    HandleMessage(std::move(udp_rx_message));
                                  }},
      udp_socket_handler_broadcast_{broadcast_udp_address, broadcast_port_num,
                                    ::doip_handler::udpSocket::DoipUdpSocketHandler::PortType::kUdp_Broadcast,
                                    [this](UdpMessagePtr udp_rx_message) { 
                                      HandleMessage(std::move(udp_rx_message)); 
                                    }} {
  // Start thread to receive messages
  // thread_ = std::thread([&]() {
  //   std::unique_lock<std::mutex> lck(mutex_);
  //   while (!exit_request_.load()) {
  //     if (!running_.load()) {
  //       cond_var_.wait(lck, [this]() { return exit_request_ || running_; });
  //     }
  //     if (!exit_request_.load()) {
  //       if (running_) { Transmit(); }
  //     }
  //   }
  // });
}

DoipUdpHandler::~DoipUdpHandler() {
  exit_request_ = true;
  running_ = false;
  cond_var_.notify_all();
  thread_.join();
};

void DoipUdpHandler::Initialize() {
  udp_socket_handler_unicast_.Start();
  udp_socket_handler_broadcast_.Start();
}

void DoipUdpHandler::DeInitialize() {
  udp_socket_handler_unicast_.Stop();
  udp_socket_handler_broadcast_.Stop();
}

void DoipUdpHandler::HandleMessage(UdpMessagePtr udp_rx_message) {
  
  remote_ip_address_ = udp_rx_message->host_ip_address_;
  // remote_port_num_ = udp_rx_message->host_port_num_;

  // received_doip_message_.protocol_version = udp_rx_message->rx_buffer_[0];
  // received_doip_message_.protocol_version_inv = udp_rx_message->rx_buffer_[1];
  // received_doip_message_.payload_type = GetDoIPPayloadType(udp_rx_message->rx_buffer_);
  // received_doip_message_.payload_length = GetDoIPPayloadLength(udp_rx_message->rx_buffer_);

  // if (received_doip_message_.payload_length > 0U) {
  //   received_doip_message_.payload.insert(received_doip_message_.payload.begin(),
  //                                         udp_rx_message->rx_buffer_.begin() + kDoipheadrSize,
  //                                         udp_rx_message->rx_buffer_.end());
  // }
  
  // TODO
  // uds_transport::UdsMessage::Address source_addr, uds_transport::UdsMessage::Address target_addr,
  //       uds_transport::UdsMessage::TargetAddressType type, uds_transport::ChannelID channel_id, std::size_t size,
  //       uds_transport::Priority priority, uds_transport::ProtocolKind protocol_kind,
  //       std::vector<uint8_t> payloadInfo
  uds_transport::UdsMessage::Address source_addr = 0x00;
  uds_transport::UdsMessage::Address target_addr = 0x00;
  uds_transport::UdsMessage::TargetAddressType type = uds_transport::UdsMessage::TargetAddressType::kPhysical;
  uds_transport::ChannelID channel_id = 0;
  std::size_t size = udp_rx_message->rx_buffer_.size();
  uds_transport::Priority priority;
  uds_transport::ProtocolKind protocol_kind = "doip";
  ::doip_handler::logger::DoipServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
          __FILE__, __LINE__, "",
          [this](std::stringstream &msg) { 
            msg << "DoipUdpHandler HandleMessage and IndicateMessage next type:"
                << "payload_type:" << remote_ip_address_
                << "payload_length:" << remote_port_num_;
          });

  doip_connection_.IndicateMessage(source_addr, target_addr, type, channel_id, size, priority, 
                                   protocol_kind, udp_rx_message->rx_buffer_);
  // Trigger async transmission
  running_ = true;
  cond_var_.notify_all();
}

// void DoipUdpHandler::ProcessUdpUnicastMessage(UdpMessagePtr udp_rx_message) {
//   received_doip_message_.host_ip_address = udp_rx_message->host_ip_address_;
//   received_doip_message_.port_num = udp_rx_message->host_port_num_;
//   received_doip_message_.protocol_version = udp_rx_message->rx_buffer_[0];
//   received_doip_message_.protocol_version_inv = udp_rx_message->rx_buffer_[1];
//   received_doip_message_.payload_type = GetDoIPPayloadType(udp_rx_message->rx_buffer_);
//   received_doip_message_.payload_length = GetDoIPPayloadLength(udp_rx_message->rx_buffer_);

//   if (received_doip_message_.payload_length > 0U) {
//     received_doip_message_.payload.insert(received_doip_message_.payload.begin(),
//                                           udp_rx_message->rx_buffer_.begin() + kDoipheadrSize,
//                                           udp_rx_message->rx_buffer_.end());
//   }
//   // Trigger async transmission
//   running_ = true;
//   cond_var_.notify_all();
// }

auto DoipUdpHandler::GetDoIPPayloadType(std::vector<uint8_t> payload) noexcept -> uint16_t {
  return ((uint16_t) (((payload[BYTE_POS_TWO] & 0xFF) << 8) | (payload[BYTE_POS_THREE] & 0xFF)));
}

auto DoipUdpHandler::GetDoIPPayloadLength(std::vector<uint8_t> payload) noexcept -> uint32_t {
  return ((uint32_t) ((payload[BYTE_POS_FOUR] << 24U) & 0xFF000000) |
          (uint32_t) ((payload[BYTE_POS_FIVE] << 16U) & 0x00FF0000) |
          (uint32_t) ((payload[BYTE_POS_SIX] << 8U) & 0x0000FF00) |
          (uint32_t) ((payload[BYTE_POS_SEVEN] & 0x000000FF)));
}

void DoipUdpHandler::SetExpectedVehicleIdentificationResponseToBeSent(DoipUdpHandler::VehicleAddrInfo& vehicle_info) {
  DoipUdpHandler::VehicleAddrInfo create_info{vehicle_info};
  create_info.eid.erase(remove(create_info.eid.begin(), create_info.eid.end(), ':'), create_info.eid.end());
  create_info.gid.erase(remove(create_info.gid.begin(), create_info.gid.end(), ':'), create_info.gid.end());
  expected_vehicle_info_ = create_info;
}

// auto DoipUdpHandler::VerifyVehicleIdentificationRequestWithExpectedVIN(std::string_view vin) noexcept -> bool {
//   constexpr std::uint8_t start_index_vin{0U};
//   std::string vehicle_info_data_vin{
//       ConvertToAsciiString(start_index_vin, received_doip_message_.payload.size(), received_doip_message_.payload)};
//   return (vehicle_info_data_vin == vin);
// }

// auto DoipUdpHandler::VerifyVehicleIdentificationRequestWithExpectedEID(std::string_view eid) noexcept -> bool {
//   constexpr std::uint8_t start_index_eid{0U};
//   std::string vehicle_info_data_vin{
//       ConvertToHexString(start_index_eid, received_doip_message_.payload.size(), received_doip_message_.payload)};
//   return (vehicle_info_data_vin == eid);
// }

// void DoipUdpHandler::Transmit() {
//   UdpMessagePtr vehicle_identification_response{std::make_unique<UdpMessage>()};
//   // create header
//   vehicle_identification_response->tx_buffer_.reserve(kDoipheadrSize + kDoip_VehicleAnnouncement_ResMaxLen);
//   CreateDoipGenericHeader(vehicle_identification_response->tx_buffer_, kDoip_VehicleAnnouncement_ResType,
//                           kDoip_VehicleAnnouncement_ResMaxLen);
//   // vin
//   SerializeVINFromString(expected_vehicle_info_.vin, vehicle_identification_response->tx_buffer_,
//                          expected_vehicle_info_.vin.length(), 1U);
//   // logical address
//   vehicle_identification_response->tx_buffer_.emplace_back(expected_vehicle_info_.logical_address >> 8U);
//   vehicle_identification_response->tx_buffer_.emplace_back(expected_vehicle_info_.logical_address & 0xFFU);
//   // eid
//   SerializeEIDGIDFromString(expected_vehicle_info_.eid, vehicle_identification_response->tx_buffer_,
//                             expected_vehicle_info_.eid.length(), 2U);
//   // gid
//   SerializeEIDGIDFromString(expected_vehicle_info_.gid, vehicle_identification_response->tx_buffer_,
//                             expected_vehicle_info_.eid.length(), 2U);
//   // set remote ip
//   vehicle_identification_response->host_ip_address_ = received_doip_message_.host_ip_address;
//   // set remote port
//   vehicle_identification_response->host_port_num_ = received_doip_message_.port_num;

//   //  set further action required
//   vehicle_identification_response->tx_buffer_.emplace_back(0U);

//   if (udp_socket_handler_unicast_.Transmit(std::move(vehicle_identification_response))) { running_ = false; }
// }

uds_transport::UdsTransportProtocolMgr::TransmissionResult DoipUdpHandler::Transmit(uds_transport::UdsMessageConstPtr message) {
  // 
  ::doip_handler::udpSocket::UdpMessagePtr udp_tx_message = std::make_unique<::doip_handler::udpSocket::UdpMessage>();
  const uds_transport::ByteVector &payload = message->GetPayload();
  uint16_t payload_type = 0x0004;
  udp_tx_message->host_ip_address_ = remote_ip_address_;
  udp_tx_message->host_port_num_ = remote_port_num_;
  
  CreateDoipGenericHeader(udp_tx_message->tx_buffer_, payload_type, payload.size());
  
  udp_tx_message->tx_buffer_.insert(udp_tx_message->tx_buffer_.begin() + udp_tx_message->tx_buffer_.size(), 
    payload.begin(), payload.end());
  
  if (udp_socket_handler_unicast_.Transmit(std::move(udp_tx_message))) {
    return uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk;
  } else {
    return uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitFailed;
  }
  // udp_socket_handler_broadcast_.Transmit(std::move(udp_tx_message));
}

void DoipUdpHandler::CreateDoipGenericHeader(std::vector<uint8_t>& doipHeader, std::uint16_t payload_type,
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

}  // namespace doip_handler
}  // namespace doip_server