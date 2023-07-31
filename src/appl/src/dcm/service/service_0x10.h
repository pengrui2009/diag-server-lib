/* Diagnostic Server 0x10
 * Copyright (C) 2023  Rui Peng
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/** @file service_0x10.h
 *  @brief Entry function to service 0x10
 *  @author Avijit Dey
 */
#ifndef DIAG_SERVER_LIB_APPL_SRC_DCM_SERVICE_SERVICE_0X10_H_
#define DIAG_SERVER_LIB_APPL_SRC_DCM_SERVICE_SERVICE_0X10_H_

#include "service_base.h"

namespace diag {
namespace server {

namespace conversation {
class DmConversation;
}

namespace service {

class Service0x10 : public ServiceBase {
public:
    Service0x10(conversation::DmConversation &);

    ~Service0x10();

    void Service() override;

    uint8_t GetSessionType() const override;

    uint16_t GetSecurityLevel() const override;
private:
    uint8_t sub_sid_;
    uint8_t session_type_;
    uint16_t security_level_;

    conversation::DmConversation &conversation_;
};

} // namespace service
} // namespace server
} // namespace diag


#endif /* DIAG_SERVER_LIB_APPL_SRC_DCM_SERVICE_SERVICE_0X10_H_ */