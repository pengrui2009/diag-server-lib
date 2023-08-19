/* Diagnostic Server library
 * Copyright (C) 2023  Rui Peng
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "src/dcm/conversation/vd_conversation.h"

#include <sstream>
#include <string>
#include <utility>

#include "src/common/logger.h"
#include "src/dcm/service/vd_message.h"
#include "common/doip_payload_type.h"
#include "common/common_doip_types.h"
#include "uds_transport/uds_message.h"

namespace diag {
namespace server {
namespace conversation {

std::string ConvertToHexString(std::uint8_t char_start, std::uint8_t char_count,
                               std::vector<std::uint8_t> &input_buffer) {
  std::string hex_string{};
  std::uint8_t total_char_count{static_cast<uint8_t>(char_start + char_count)};

  for (std::uint8_t char_start_count = char_start; char_start_count < total_char_count; char_start_count++) {
    std::stringstream vehicle_info_data_eid{};
    int payload_byte{input_buffer[char_start_count]};
    if ((payload_byte <= 15U)) {
      // "0" appended in case of value upto 15/0xF
      vehicle_info_data_eid << "0";
    }
    vehicle_info_data_eid << std::hex << payload_byte << ":";
    hex_string.append(vehicle_info_data_eid.str());
  }
  hex_string.pop_back();  // remove last ":" appended before
  return hex_string;
}

std::string ConvertToAsciiString(std::uint8_t char_start, std::uint8_t char_count,
                                 std::vector<std::uint8_t> &input_buffer) {
  std::string ascii_string{};
  std::uint8_t total_char_count{static_cast<uint8_t>(char_start + char_count)};

  for (std::uint8_t char_start_count = char_start; char_start_count < total_char_count; char_start_count++) {
    std::stringstream vehicle_info_data_vin{};
    vehicle_info_data_vin << input_buffer[char_start_count];
    ascii_string.append(vehicle_info_data_vin.str());
  }
  return ascii_string;
}

void SerializeEIDGIDFromString(std::string &input_string, std::vector<uint8_t> &output_buffer, std::uint8_t total_size,
                               std::uint8_t substring_range) {
                         
  for (auto char_count = 0U; char_count < total_size; char_count += substring_range+1) {
    std::string input_string_new{input_string.substr(char_count, static_cast<std::uint8_t>(substring_range))};
    std::stringstream input_string_stream{input_string_new};
    int get_byte;
    input_string_stream >> std::hex >> get_byte;
    output_buffer.emplace_back(static_cast<std::uint8_t>(get_byte));
  }
}

void SerializeVINFromString(std::string &input_string, std::vector<uint8_t> &output_buffer, std::uint8_t total_size,
                            std::uint8_t substring_range) {

  for (auto char_count = 0U; char_count < total_size; char_count += substring_range) {
    std::string input_string_new{input_string.substr(char_count, static_cast<std::uint8_t>(substring_range))};
    std::stringstream input_string_stream{input_string_new};
    int get_byte{input_string_stream.get()};
    output_buffer.emplace_back(static_cast<std::uint8_t>(get_byte));
  }
}

// Vehicle Info Message implementation class
class VehicleInfoMessageImpl final : public vehicle_info::VehicleInfoMessage {
public:
  explicit VehicleInfoMessageImpl(
      std::map<std::uint16_t, vehicle_info::VehicleAddrInfoResponse> &vehicle_info_collection)
      : vehicle_info_messages_{} {
    for (std::pair<std::uint16_t, vehicle_info::VehicleAddrInfoResponse> vehicle_info: vehicle_info_collection) {
      Push(vehicle_info.second);
    }
  }

  ~VehicleInfoMessageImpl() override = default;

  VehicleInfoListResponseType &GetVehicleList() override { return vehicle_info_messages_; }

private:
  // Function to push the vehicle address info received
  void Push(vehicle_info::VehicleAddrInfoResponse &vehicle_addr_info_response) {
    vehicle_info_messages_.emplace_back(vehicle_addr_info_response);
  }

  // store the vehicle info message list
  VehicleInfoListResponseType vehicle_info_messages_;
};

// Conversation class
VdConversation::VdConversation(std::string_view conversion_name,
                               uds_transport::conversion_manager::ConversionIdentifierType &conversion_identifier)
    : vin_name_{conversion_identifier.vin_name},
      eid_name_{conversion_identifier.eid_name},
      gid_name_{conversion_identifier.gid_name},
      logical_address_{conversion_identifier.logical_address},
      vd_conversion_handler_{std::make_shared<VdConversationHandler>(conversion_identifier.handler_id, *this)},
      conversation_name_{conversion_name},
      broadcast_address_{conversion_identifier.udp_broadcast_address},
      connection_ptr_{},
      vehicle_info_collection_{},
      exit_request_{false},
      running_{false},
      vehicle_info_container_mutex_{} {
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

void VdConversation::Startup() {
  // initialize the connection
  static_cast<void>(connection_ptr_->Initialize());
  // start the connection
  connection_ptr_->Start();
}

void VdConversation::Shutdown() {
  // shutdown connection
  connection_ptr_->Stop();
}

void VdConversation::RegisterConnection(std::shared_ptr<uds_transport::Connection> connection) {
  connection_ptr_ = std::move(connection);
}

void VdConversation::SendVehicleIdentificationResponse() {
  uds_transport::UdsMessagePtr vehicle_identification_response{
      std::make_unique<vd_message::VdMessage>()};
  // create header
  // vehicle_identification_response->tx_buffer_.reserve(kDoipheadrSize + kDoip_VehicleAnnouncement_ResMaxLen);
  // CreateDoipGenericHeader(vehicle_identification_response->tx_buffer_, kDoip_VehicleAnnouncement_ResType,
  //                         kDoip_VehicleAnnouncement_ResMaxLen);
  // vin
  std::vector<uint8_t> vehicle_info_payload;
  std::vector<uint8_t> vin_payload;
  SerializeVINFromString(vin_name_, vin_payload, vin_name_.length(), 1U);
  vehicle_info_payload.insert(vehicle_info_payload.begin(), vin_payload.begin(), vin_payload.end());
  // logical address
  vehicle_info_payload.emplace_back(logical_address_ >> 8U);
  vehicle_info_payload.emplace_back(logical_address_ & 0xFFU);
  // eid
  std::vector<uint8_t> eid_payload;
  SerializeEIDGIDFromString(eid_name_, eid_payload, eid_name_.length(), 2U);
  vehicle_info_payload.insert(vehicle_info_payload.begin()+vehicle_info_payload.size(), eid_payload.begin(), 
    eid_payload.end());
  // gid
  std::vector<uint8_t> gid_payload;
  SerializeEIDGIDFromString(gid_name_, gid_payload, gid_name_.length(), 2U);
  vehicle_info_payload.insert(vehicle_info_payload.begin()+vehicle_info_payload.size(), gid_payload.begin(), 
    gid_payload.end());

  vehicle_info_payload.push_back(0x00);
  vehicle_info_payload.push_back(0x00);

  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
        __FILE__, __LINE__, "", [eid_payload, gid_payload, &vehicle_info_payload](std::stringstream &msg) {
          msg << "VdConversation::SendVehicleIdentificationResponse eid_payload size:" << eid_payload.size()
              << "gid_payload size:" << gid_payload.size() 
              << "payloadInfo size:" << vehicle_info_payload.size() << " data:";
          for (int i=0; i<vehicle_info_payload.size(); i++) {
            msg<< static_cast<int>(vehicle_info_payload[i]) << " ";
          }
        });
  vehicle_identification_response->SetPayload(vehicle_info_payload);                            
  // set remote ip
  vehicle_identification_response->SetRemoteIpAddress("");//remote_ip_address_ = received_doip_message_.host_ip_address;
  // set remote port
  vehicle_identification_response->SetRemotePortNumber(0);// remote_port_num_ = received_doip_message_.port_num;

  //  set further action required
  // vehicle_identification_response->tx_buffer_.emplace_back(0U);

  uds_transport::UdsTransportProtocolMgr::TransmissionResult result = 
    connection_ptr_->Transmit(std::move(vehicle_identification_response));
  if (uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitOk == result) {
    running_ = false; 
  }

}

VdConversation::VehicleIdentificationResponseResult VdConversation::SendVehicleIdentificationRequest(
    vehicle_info::VehicleInfoListRequestType vehicle_info_request) {
  VehicleIdentificationResponseResult ret_val{VehicleResponseResult::kTransmitFailed, nullptr};
  // Deserialize first , Todo: Add optional when deserialize fails
  std::pair<PreselectionMode, PreselectionValue> vehicle_info_request_deserialized_value{
      DeserializeVehicleInfoRequest(vehicle_info_request)};

  if (VerifyVehicleInfoRequest(vehicle_info_request_deserialized_value.first,
                               vehicle_info_request_deserialized_value.second.size())) {
    if (connection_ptr_->Transmit(std::move(std::make_unique<diag::server::vd_message::VdMessage>(
            vehicle_info_request_deserialized_value.first, vehicle_info_request_deserialized_value.second,
            broadcast_address_))) != uds_transport::UdsTransportProtocolMgr::TransmissionResult::kTransmitFailed) {
      // Check if any response received
      if (vehicle_info_collection_.empty()) {
        // no response received
        ret_val.first = VehicleResponseResult::kNoResponseReceived;
      } else {
        ret_val.first = VehicleResponseResult::kStatusOk;
        ret_val.second = std::move(std::make_unique<VehicleInfoMessageImpl>(vehicle_info_collection_));
        // all the responses are copied, now clear the map
        vehicle_info_collection_.clear();
      }
    }
  } else {
    ret_val.first = VehicleResponseResult::kInvalidParameters;
  }
  return ret_val;
}

vehicle_info::VehicleInfoMessageResponseUniquePtr VdConversation::GetDiagnosticServerList() { return nullptr; }

auto VdConversation::GetDoIPPayloadType(std::vector<uint8_t> payload) noexcept -> uint16_t {
  return ((uint16_t) (((payload[BYTE_POS_TWO] & 0xFF) << 8) | (payload[BYTE_POS_THREE] & 0xFF)));
}

auto VdConversation::GetDoIPPayloadLength(std::vector<uint8_t> payload) noexcept -> uint32_t {
  return ((uint32_t) ((payload[BYTE_POS_FOUR] << 24U) & 0xFF000000) |
          (uint32_t) ((payload[BYTE_POS_FIVE] << 16U) & 0x00FF0000) |
          (uint32_t) ((payload[BYTE_POS_SIX] << 8U) & 0x0000FF00) |
          (uint32_t) ((payload[BYTE_POS_SEVEN] & 0x000000FF)));
}

auto VdConversation::VerifyVehicleIdentificationRequestWithExpectedVIN(uds_transport::ByteVector &doip_payload, 
  std::string vin) noexcept -> bool {
  constexpr std::uint8_t start_index_vin{0U};
  std::string vehicle_info_data_vin{
      ConvertToAsciiString(start_index_vin, doip_payload.size(), doip_payload)};
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
        __FILE__, __LINE__, "", [vehicle_info_data_vin, vin](std::stringstream &msg) {
          msg << "VdConversation::VerifyVehicleIdentificationRequestWithExpectedVIN vin_in:" << vehicle_info_data_vin.size() 
              << " vin:" << vin.size();
        });
  return (vehicle_info_data_vin == vin);
}

auto VdConversation::VerifyVehicleIdentificationRequestWithExpectedEID(uds_transport::ByteVector &doip_payload,
  std::string eid) noexcept -> bool {
  constexpr std::uint8_t start_index_eid{0U};
  std::string vehicle_info_data_eid{
      ConvertToHexString(start_index_eid, doip_payload.size(), doip_payload)};
  return (vehicle_info_data_eid == eid);
}


std::pair<VdConversation::IndicationResult, uds_transport::UdsMessagePtr> VdConversation::IndicateMessage(
    uds_transport::UdsMessage::Address /* source_addr */, uds_transport::UdsMessage::Address /* target_addr */,
    uds_transport::UdsMessage::TargetAddressType /* type */, uds_transport::ChannelID channel_id, std::size_t size,
    uds_transport::Priority priority, uds_transport::ProtocolKind protocol_kind, std::vector<uint8_t> payloadInfo) {
  std::pair<IndicationResult, uds_transport::UdsMessagePtr> ret_val{IndicationResult::kIndicationNOk, nullptr};

  DoipMessage receive_message;
  logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
        __FILE__, __LINE__, "", [&payloadInfo](std::stringstream &msg) {
          msg << "VdConversation::IndicateMessage payloadInfo size:" << payloadInfo.size() << " data:";
          for (int i=0; i<payloadInfo.size(); i++) {
            msg<< static_cast<int>(payloadInfo[i]) << " ";
          }
        });
  
  // receive_message.host_ip_address = udp_rx_message->host_ip_address_;
  // receive_message.port_num = udp_rx_message->host_port_num_;
  receive_message.protocol_version = payloadInfo[0];
  receive_message.protocol_version_inv = payloadInfo[1];
  receive_message.payload_type = GetDoIPPayloadType(payloadInfo);
  receive_message.payload_length = GetDoIPPayloadLength(payloadInfo);

  if (receive_message.payload_length > 0U) {    
    // receive_message.payload.resize(receive_message.payload_length);
    receive_message.payload.insert(receive_message.payload.begin(), 
      payloadInfo.begin() + kDoipheadrSize, payloadInfo.end());
    
    switch (receive_message.payload_type) {
    case 0U:
      break;
    case 1U:
      break;
    case 2U:
      break;
    case 3U:
      if (VerifyVehicleIdentificationRequestWithExpectedVIN(receive_message.payload, vin_name_)) {
        logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
          __FILE__, __LINE__, "", [&payloadInfo](std::stringstream &msg) {
            msg << "VdConversation::VerifyVehicleIdentificationRequestWithExpectedVIN success";
          });
          job_queue_.emplace([this]() { this->SendVehicleIdentificationResponse(); });
          running_ = true;
          cond_var_.notify_all();
      } else {
        logger::DiagServerLogger::GetDiagServerLogger().GetLogger().LogInfo(
          __FILE__, __LINE__, "", [&payloadInfo](std::stringstream &msg) {
            msg << "VdConversation::VerifyVehicleIdentificationRequestWithExpectedVIN failed";
          });
      }
      
      break;
    case 4U:
      break;
    default:
      break;
    }
  }
  
  // {
  //   UdpMessagePtr vehicle_identification_response{std::make_unique<UdpMessage>()};
  //   // create header
  //   vehicle_identification_response->tx_buffer_.reserve(kDoipheadrSize + kDoip_VehicleAnnouncement_ResMaxLen);
  //   CreateDoipGenericHeader(vehicle_identification_response->tx_buffer_, kDoip_VehicleAnnouncement_ResType,
  //                           kDoip_VehicleAnnouncement_ResMaxLen);
  //   // vin
  //   SerializeVINFromString(expected_vehicle_info_.vin, vehicle_identification_response->tx_buffer_,
  //                         expected_vehicle_info_.vin.length(), 1U);
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

  //   if (udp_socket_handler_unicast_.Transmit(std::move(vehicle_identification_response))) 
  //   { 
  //     running_ = false; 
  //   }
  // }

  return ret_val;
}

void VdConversation::HandleMessage(uds_transport::UdsMessagePtr message) {
  if (message != nullptr) {
    std::lock_guard<std::mutex> const lock{vehicle_info_container_mutex_};
    std::pair<std::uint16_t, VehicleAddrInfoResponseStruct> vehicle_info_request{
        DeserializeVehicleInfoResponse(std::move(message))};

    vehicle_info_collection_.emplace(vehicle_info_request.first, vehicle_info_request.second);
  }
}

bool VdConversation::VerifyVehicleInfoRequest(PreselectionMode preselection_mode,
                                              std::uint8_t preselection_value_length) {
  bool is_veh_info_valid{false};
  if ((preselection_mode != 0U) && (preselection_value_length != 0U)) {
    // 1U : DoIP Entities with given VIN
    if (preselection_mode == 1U && (preselection_value_length == 17U)) {
      is_veh_info_valid = true;
    }
    // 2U : DoIP Entities with given EID
    else if (preselection_mode == 2U && (preselection_value_length == 6U)) {
      is_veh_info_valid = true;
    } else {
    }
  }
  // 0U : No preselection
  else if (preselection_mode == 0U && (preselection_value_length == 0U)) {
    is_veh_info_valid = true;
  } else {
  }

  return is_veh_info_valid;
}

std::pair<std::uint16_t, VdConversation::VehicleAddrInfoResponseStruct> VdConversation::DeserializeVehicleInfoResponse(
    uds_transport::UdsMessagePtr message) {
  constexpr std::uint8_t start_index_vin{0U};
  constexpr std::uint8_t total_vin_length{17U};
  constexpr std::uint8_t start_index_eid{19U};
  constexpr std::uint8_t start_index_gid{25U};
  constexpr std::uint8_t total_eid_gid_length{6U};

  std::string vehicle_info_data_vin{ConvertToAsciiString(start_index_vin, total_vin_length, message->GetPayload())};
  std::string vehicle_info_data_eid{ConvertToHexString(start_index_eid, total_eid_gid_length, message->GetPayload())};
  std::string vehicle_info_data_gid{ConvertToHexString(start_index_gid, total_eid_gid_length, message->GetPayload())};

  std::uint16_t logical_address{
      (static_cast<std::uint16_t>(((message->GetPayload()[17U] & 0xFF) << 8) | (message->GetPayload()[18U] & 0xFF)))};

  // Create the structure out of the extracted string
  VehicleAddrInfoResponseStruct vehicle_addr_info{std::string{message->GetRemoteIpAddress()},  // remote ip address
                                                  logical_address,                           // logical address
                                                  vehicle_info_data_vin,                     // vin
                                                  vehicle_info_data_eid,                     // eid
                                                  vehicle_info_data_gid};                    // gid

  return {logical_address, vehicle_addr_info};
}

std::shared_ptr<uds_transport::ConversionHandler> &VdConversation::GetConversationHandler() {
  return vd_conversion_handler_;
}

std::pair<VdConversation::PreselectionMode, VdConversation::PreselectionValue>
VdConversation::DeserializeVehicleInfoRequest(vehicle_info::VehicleInfoListRequestType &vehicle_info_request) {

  std::pair<VdConversation::PreselectionMode, VdConversation::PreselectionValue> ret_val{};
  ret_val.first = vehicle_info_request.preselection_mode;

  if (ret_val.first == 1U) {
    // 1U : DoIP Entities with given VIN
    SerializeVINFromString(vehicle_info_request.preselection_value, ret_val.second,
                           vehicle_info_request.preselection_value.length(), 1U);
  } else if (ret_val.first == 2U) {
    // 2U : DoIP Entities with given EID
    vehicle_info_request.preselection_value.erase(
        remove(vehicle_info_request.preselection_value.begin(), vehicle_info_request.preselection_value.end(), ':'),
        vehicle_info_request.preselection_value.end());
    SerializeEIDGIDFromString(vehicle_info_request.preselection_value, ret_val.second,
                              vehicle_info_request.preselection_value.length(), 2U);
  }
  return ret_val;
}

VdConversationHandler::VdConversationHandler(uds_transport::conversion_manager::ConversionHandlerID handler_id,
                                             VdConversation &vd_conversion)
    : uds_transport::ConversionHandler{handler_id},
      vd_conversation_{vd_conversion} {}

std::pair<uds_transport::UdsTransportProtocolMgr::IndicationResult, uds_transport::UdsMessagePtr>
VdConversationHandler::IndicateMessage(uds_transport::UdsMessage::Address source_addr,
                                       uds_transport::UdsMessage::Address target_addr,
                                       uds_transport::UdsMessage::TargetAddressType type,
                                       uds_transport::ChannelID channel_id, std::size_t size,
                                       uds_transport::Priority priority, uds_transport::ProtocolKind protocol_kind,
                                       std::vector<uint8_t> payloadInfo) {
  return (vd_conversation_.IndicateMessage(source_addr, target_addr, type, channel_id, size, priority, protocol_kind,
                                           payloadInfo));
}

void VdConversationHandler::HandleMessage(uds_transport::UdsMessagePtr message) {
  vd_conversation_.HandleMessage(std::move(message));
}

}  // namespace conversation
}  // namespace server
}  // namespace diag
