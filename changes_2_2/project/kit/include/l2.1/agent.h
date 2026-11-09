#ifndef NANO_EDR_KIT_AGENT_H
#define NANO_EDR_KIT_AGENT_H

#include <cstddef>
#include <map>

#include "event.h"
#include "event_list.h"
#include "os.h"
#include "rules.h"

namespace nano_edr {

class Agent {
    public:
        Agent(std::size_t window_size, bool quiet) : window_(window_size), quiet_(quiet) {}
        void HandleEvent(const Event& event);
        void PrintSummary() const;
        void PrintContext() const;
        void CountComment() { ++comments_; }
        void CountSkipped() noexcept { ++skipped_; }
        static void Trampoline(const os_event* ev, void* ctx) noexcept;

    private:
        EventList window_;
        const EventNode* prev_ = nullptr;
        bool quiet_;
        long long lines_ = 0;
        long long comments_ = 0;
        long long skipped_ = 0;
        std::map<std::string, int> types_;
};

}  // namespace nano_edr

#endif  // NANO_EDR_KIT_AGENT_H
