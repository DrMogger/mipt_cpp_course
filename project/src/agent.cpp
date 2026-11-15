#include "agent.h"

#include <print>
#include <utility>

#include "agent_rules.h"
#include "dedup_sink.h"

namespace nano_edr {

Agent::Agent(bool quiet, uint64_t dedup_period_ms)
    : quiet_(quiet), dedup_period_ms_(dedup_period_ms) {
    engine_.SetModel(&model_);
    RegisterAgentRules(&engine_, &model_);
}

void Agent::PrintContext() const {
    const size_t count = window_.size() < 2 ? window_.size() : 2;
    const size_t first = window_.size() - count;
    for (size_t i = first; i < window_.size(); ++i) {
        const WindowEntry& entry = window_[i];
        std::print("[CTX] -{}: {} {} pid={}\n", window_.size() - i, entry.ts, entry.type, entry.pid);
    }
}

void Agent::PrintContextOnce() {
    if (quiet_ || context_printed_) {
        return;
    }

    PrintContext();
    context_printed_ = true;
}

void Agent::Subscribe(AlertSink sink) {
    FanOutSink all;
    all.Add([this](const Detection&) { PrintContextOnce(); });
    all.Add(std::move(sink));
    engine_.Subscribe(DedupSink(std::move(all), dedup_period_ms_, &suppressed_));
}

void Agent::HandleEvent(const Event& event) {
    model_.Observe(event);
    context_printed_ = false;
    engine_.ProcessEvent(event, nullptr);

    window_.PushBack({event.raw_ts(), event.type(), event.pid()});
    ++types_[event.type()];
}

void Agent::PrintSummary() const {
    if (!quiet_) {
        std::print("[STAT] в окне {} из {} мест\n", window_.size(), window_.capacity());
        std::print("[STAT] в модели процессов {}, файлов {}\n",
                   model_.process_count(), model_.file_count());
        if (suppressed_ > 0) {
            std::print("[STAT] подавлено дубликатов {} (срок {} мс)\n",
                       suppressed_, dedup_period_ms_);
        }
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
