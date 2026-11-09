#include "rule.h"

#include <utility>

namespace nano_edr {

RuleBase::RuleBase(const std::string& id, Severity severity)
    : id_(id), severity_(severity) {}

void RuleBase::AddAction(const std::string& action) {
    actions_.push_back(action);
}

MatchRule::MatchRule(const std::string& id, Severity severity)
    : RuleBase(id, severity) {}

void MatchRule::AddCondition(std::unique_ptr<ICondition> condition) {
    conditions_.push_back(std::move(condition));
}

bool MatchRule::Check(const Event& event) {
    for (const auto& condition : conditions_) {
        if (!condition->Matches(event)) {
            return false;
        }
    }
    CountHit();
    return true;
}

SequenceRule::SequenceRule(const std::string& id, Severity severity, uint64_t window_ms)
    : RuleBase(id, severity), window_ms_(window_ms) {}

void SequenceRule::SetFirst(std::unique_ptr<ICondition> condition) {
    first_ = std::move(condition);
}

void SequenceRule::SetSecond(std::unique_ptr<ICondition> condition) {
    second_ = std::move(condition);
}

bool SequenceRule::Check(const Event& event) {
    if (!first_ || !second_) {
        return false;
    }

    const uint64_t now = event.ts().ms;
    auto pending = pending_.begin();
    while (pending != pending_.end()) {
        if (now >= pending->ts_ms && now - pending->ts_ms > window_ms_) {
            pending = pending_.erase(pending);
        } else {
            ++pending;
        }
    }

    if (second_->Matches(event)) {
        for (auto match = pending_.begin(); match != pending_.end(); ++match) {
            if (now >= match->ts_ms && (!same_pid_ || match->pid == event.pid())) {
                pending_.erase(match);
                CountHit();
                return true;
            }
        }
    }

    if (first_->Matches(event)) {
        pending_.push_back(Pending{now, event.pid()});
    }
    return false;
}

}  // namespace nano_edr
