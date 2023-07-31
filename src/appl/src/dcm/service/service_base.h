/* Diagnostic Server Base
 * Copyright (C) 2023  Rui Peng
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/** @file service_base.h
 *  @brief Entry function to service base
 *  @author Avijit Dey
 */
#ifndef DIAG_SERVER_LIB_APPL_SRC_DCM_SERVICE_SERVICE_BASE_H_
#define DIAG_SERVER_LIB_APPL_SRC_DCM_SERVICE_SERVICE_BASE_H_

#include <stdint.h>

enum DiagnosticSecurityLevel {
    SECURITY_LEVEL_0 = 0u, /* no need to unlocked */
    SECURITY_LEVEL_1 = 1u,
    SECURITY_LEVEL_2 = 2u,
    SECURITY_LEVEL_3 = 4u,
};

enum DiagnosticSessionType {
    defaultSession = 0x01u,
    programmingSession = 0x02u,
    extendedDiagnosticSession = 0x04u,
};

class ServiceBase {
public:
    ServiceBase() {}

    virtual ~ServiceBase() = default;

    virtual void Service() = 0;

    virtual uint8_t GetSessionType() const = 0;

    virtual uint16_t GetSecurityLevel() const = 0;

public:


};

#endif /* DIAG_SERVER_LIB_APPL_SRC_DCM_SERVICE_SERVICE_BASE_H_ */