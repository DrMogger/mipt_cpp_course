#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H

#include <cstddef>
#include <map>
#include <vector>
#include <string>

#include "event.h"
#include "ring_buffer.h"
#include "event_source.h"
#include "rule_engine.h"

namespace nano_edr {

class WindowEntry {
    std::string ts;
    std::string type;
    std::string pid;
};
class Agent : public IEventHandler {
    public:
        Agent(bool quiet);
        void HandleEvent(const Event& event) override;
        void OnSkipped() override {}
        void OnBroken() override {}
        void OnTick() override {}
        void PrintSummary() const;

    private:
        void PrintContext() const;

        RingBuffer<WindowEntry, 256> window_;
        RuleEngine engine_;
        bool quiet_;
        std::map<std::string, int> types_;
};

}  // namespace nano_edr

#endif  // NANO_EDR_AGENT_H
