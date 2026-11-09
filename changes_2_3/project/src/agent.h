#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H

#include <cstddef>
#include <map>
#include <vector>

#include "event.h"
#include "event_list.h"
#include "rule.h"

namespace nano_edr {

class Agent {
    public:
        Agent(std::size_t window_size, bool quiet);
        void HandleEvent(const Event& event);
        void PrintSummary() const;

    private:
        void PrintContext() const;

        EventList window_;
        std::vector<MatchRule> rules_;
        const EventNode* prev_ = nullptr;
        bool quiet_;
        std::map<std::string, int> types_;
};

}  // namespace nano_edr

#endif  // NANO_EDR_AGENT_H
