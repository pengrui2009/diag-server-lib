#ifndef DIAG_SERVER_LIB_SERVICE_BASE_H
#define DIAG_SERVER_LIB_SERVICE_BASE_H

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

#endif /* DIAG_SERVER_LIB_SERVICE_BASE_H */