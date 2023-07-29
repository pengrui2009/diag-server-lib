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
