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

struct WindowEntry {
    std::string ts;
    std::string type;
    std::string pid;
};
class Agent : public IEventHandler {
    public:
        Agent(bool quiet, uint64_t dedup_period_ms = 10000);
        void HandleEvent(const Event& event) override;
        void OnSkipped() override {}
        void OnBroken() override {}
        void OnTick() override {}
        void PrintSummary() const;
        void Subscribe(AlertSink sink);

    private:
        void PrintContextOnce();
        bool context_printed_ = false;
        void PrintContext() const;

        RingBuffer<WindowEntry, 256> window_;
        EntityModel model_;
        RuleEngine engine_;
        bool quiet_;
        uint64_t dedup_period_ms_;
        std::size_t suppressed_ = 0;
        std::map<std::string, int> types_;
};

}  // namespace nano_edr

#endif  // NANO_EDR_AGENT_H
