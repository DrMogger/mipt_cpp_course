#include "rule_engine.h"

#include <utility>

namespace nano_edr {

void RuleEngine::AddRule(std::unique_ptr<IRule> rule) {
    rules_.push_back(std::move(rule));
}

std::size_t RuleEngine::ProcessEvent(const Event& event,
                                   std::vector<const IRule*>* fired) {
    std::size_t matched = 0;
    for (const auto& rule : rules_) {
        if (rule->Check(event)) {
            ++matched;
            if (fired) {
                fired->push_back(rule.get());
            }
        }
    }
    return matched;
}

}  // namespace nano_edr
