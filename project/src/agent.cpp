#include "agent.h"

#include <print>

#include "agent_rules.h"

namespace nano_edr {

Agent::Agent(bool quiet)
    : quiet_(quiet) {
    RegisterAgentRules(&engine_);
}

void Agent::PrintContext() const {
    const size_t count = window_.size() < 2 ? window_.size() : 2;
    const size_t first = window_.size() - count;
    for (size_t i = first; i < window_.size(); ++i) {
        const WindowEntry& entry = window_[i];
        std::print("[CTX] -{}: {} {} pid={}\n", window_.size() - i, entry.ts, entry.type, entry.pid);
    }
}

void Agent::HandleEvent(const Event& event) {
    std::vector<const IRule*> fired;
    const std::size_t matched = engine_.ProcessEvent(event, &fired);

    if (matched && !quiet_) PrintContext();

    for (const IRule* rule : fired) {
        std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(rule->severity()), rule->id(), event.raw_ts(), event.pid());
    }
    window_.PushBack({event.raw_ts(), event.type(), event.pid()});
    ++types_[event.type()];
}

void Agent::PrintSummary() const {
    if (!quiet_) {
        std::print("[STAT] в окне {} из {} мест\n", window_.size(), window_.capacity());
        for (const auto& [type, value] : types_) {
            std::print("{} раз встретился тип {}\n", value, type);
        }
        std::print("правила, детектов:\n");
        for (std::size_t i = 0; i < engine_.size(); ++i) {
            const IRule& rule = engine_.rule(i);
            std::print("  {}  {}\n", rule.id(), rule.hits());
        }
    }
}

}  // namespace nano_edr
