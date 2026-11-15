#ifndef NANO_EDR_AGENT_RULES_H
#define NANO_EDR_AGENT_RULES_H

#include "rule_engine.h"

namespace nano_edr {

void RegisterAgentRules(RuleEngine* engine, const EntityModel* model);

}  // namespace nano_edr

#endif  // NANO_EDR_AGENT_RULES_H
