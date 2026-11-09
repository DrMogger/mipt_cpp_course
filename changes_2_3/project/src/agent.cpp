#include "agent.h"

#include <print>

#include "agent_rules.h"

namespace nano_edr {

Agent::Agent(std::size_t window_size, bool quiet)
    : window_(window_size), rules_(BuildAgentRules()), quiet_(quiet) {}

void Agent::PrintContext() const {
    if (prev_){
        nano_edr::EventNode *tail = prev_->next;
        int i = tail ? 2 : 1;
        auto* current = prev_;
        while (current) {
            std::print("[CTX] -{}: ts={} type={} pid={}\n", i, current->event.raw_ts(), current->event.type(), current->event.pid());
            current = current->next;
            i--;
        }
    }
}

void Agent::HandleEvent(const Event& event) {
    const EventNode* tail = window_.back();
    int matched = 0;

    for (MatchRule& rule : rules_) {
        if (rule.Check(event)) {
            ++matched;
            std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(rule.severity()), rule.id(), event.raw_ts(), event.pid());
        }
    }
    if(matched && !quiet_)   PrintContext();
    prev_ = tail;

    window_.PushBack(event);

    if (window_.size() == 1)   prev_ = window_.back();
    ++types_[event.type()];
}

void Agent::PrintSummary() const {
    if (!quiet_) {
        for (const auto& [type, value] : types_) {
            std::print("{} раз встретился тип {}\n", value, type);
        }
        std::print("правила, детектов:\n");
        for (const MatchRule& rule : rules_) {
            std::print("  {}  {}\n", rule.id(), rule.hits());
        }
    }
}

}  // namespace nano_edr
