/* Diagnostic Server library
* Copyright (C) 2023  Avijit Dey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef DIAGNOSTIC_SERVER_LIB_APPL_SRC_DCM_SERVICE_VD_MESSAGE_H
#define DIAGNOSTIC_SERVER_LIB_APPL_SRC_DCM_SERVICE_VD_MESSAGE_H
/* includes */
#include "include/diagnostic_server_uds_message_type.h"
#include "include/diagnostic_server_vehicle_info_message_type.h"
#include "uds_transport/uds_message.h"

namespace diag {
namespace server {
namespace vd_message {

class VdMessage final : public uds_transport::UdsMessage {
public:
  // ctor
  VdMessage(std::uint8_t preselection_mode, uds_transport::ByteVector& preselection_value,
            std::string_view remote_ip_address);

  // default ctor
  VdMessage() noexcept;

  // dtor
  ~VdMessage() noexcept override = default;

protected:
  // SA
  Address source_address_;

  // TA
  Address target_address_;

  // TA type
  TargetAddressType target_address_type;

  // Remote Ip Address
  IpAddress remote_ip_address_;

  // Remote Port Number
  PortNumber remote_port_number_;

  // store the vehicle info payload
  uds_transport::ByteVector vehicle_info_payload_;

  // store the
  std::shared_ptr<const MetaInfoMap> meta_info_{};

  // add new metaInfo to this message.
  void AddMetaInfo(std::shared_ptr<const MetaInfoMap> meta_info) override {
    // update meta info data
    if (meta_info != nullptr) {
      meta_info_ = meta_info;
      remote_ip_address_ = meta_info_->at("kRemoteIpAddress");
    }
  }

  // Get the UDS message data starting with the SID (A_Data as per ISO)
  const uds_transport::ByteVector& GetPayload() const override { return vehicle_info_payload_; }

  // return the underlying buffer for write access
  uds_transport::ByteVector& GetPayload() override { return vehicle_info_payload_; }

  void SetPayload(const std::vector<uint8_t> &payload) noexcept override {
    vehicle_info_payload_ = payload;
  }
  // Get the source address of the uds message.
  Address GetSa() const noexcept override { return source_address_; }

  // Get the target address of the uds message.
  Address GetTa() const noexcept override { return target_address_; }

  // Get the target address type (phys/func) of the uds message.
  TargetAddressType GetTaType() const noexcept override { return target_address_type; }

  // Get Remote Ip address
  IpAddress GetRemoteIpAddress() const noexcept override { return remote_ip_address_; }

  void SetRemoteIpAddress(const IpAddress &ip_address) noexcept override { remote_ip_address_ = ip_address; }

  // Get Remote port number
  PortNumber GetRemotePortNumber() const noexcept override { return remote_port_number_; }

  void SetRemotePortNumber(const PortNumber &port_number) noexcept {remote_port_number_ = port_number;}
};

}  // namespace vd_message
}  // namespace server
}  // namespace diag

#endif  // DIAGNOSTIC_SERVER_LIB_APPL_SRC_DCM_SERVICE_VD_MESSAGE_H
