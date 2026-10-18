#include "agent.h"

#include <cstdio>
#include <exception>
#include <print>

#include "agent_rules.h"

namespace nano_edr {

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

    matched += CheckRules(event, AgentRules(), AgentRuleCount());
    if(matched && !quiet_)   PrintContext();
    prev_ = tail;

    window_.PushBack(event);

    if (window_.size() == 1)   prev_ = window_.back();
    ++lines_;
    ++types_[event.type()];
}

void Agent::PrintSummary() const {
    if (!quiet_) {
        std::print("строк {}, из них комментариев {}, пропущено событий {}\n",
                   lines_ + comments_ + skipped_, comments_, skipped_);
        for (const auto& [type, value] : types_) {
            std::print("{} раз встретился тип {}\n", value, type);
        }
    }
}

void Agent::Trampoline(const os_event* ev, void* ctx) noexcept {
    try {
        if (!ev || !ctx) {
            return;
        }
        EventParts parts;
        parts.ts = std::to_string(ev->ts);
        parts.type = ev->type;
        if (ev->pid != 0) {
            parts.pid = std::to_string(ev->pid);
        }
        parts.fields.reserve(ev->field_count);
        for (size_t i = 0; i < ev->field_count; ++i) {
            parts.fields.push_back(Field{ev->fields[i].key, ev->fields[i].value});
        }

        Event event(parts);
        static_cast<Agent*>(ctx)->HandleEvent(event);
    } catch (const std::exception& e) {
        if (ctx) {
            static_cast<Agent*>(ctx)->CountSkipped();
        }
        std::fprintf(stderr, "пропущено событие: %s\n", e.what());
    } catch (...) {
        if (ctx) {
            static_cast<Agent*>(ctx)->CountSkipped();
        }
        std::fprintf(stderr, "пропущено событие: неизвестная ошибка\n");
    }
}

}  // namespace nano_edr
