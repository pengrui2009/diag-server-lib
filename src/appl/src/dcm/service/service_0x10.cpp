#include "service_0x10.h"
#include "src/dcm/conversation/dm_conversation.h"

namespace diag {
namespace server {
namespace service {

Service0x10::Service0x10(conversation::DmConversation &conversation) :
    session_type_{defaultSession|programmingSession|extendedDiagnosticSession}, 
    security_level_{SECURITY_LEVEL_0},
    conversation_(conversation) {

}

Service0x10::~Service0x10() {

}

uint8_t Service0x10::GetSessionType() const {
    return session_type_;
}

uint16_t Service0x10::GetSecurityLevel() const {
    return security_level_;
}

void Service0x10::Service() {

}

} // namespace service
} // namespace server
} // namespace diag