#include <event.h>

#include <charconv>
#include <stdexcept>

namespace nano_edr {
namespace {

std::uint64_t ParseTs(const std::string& s) {
    std::uint64_t v = 0;
    const char* end = s.data() + s.size();
    auto [ptr, ec] = std::from_chars(s.data(), end, v);
    // пусто, не число ("12ab") или переполнение uint64_t
    if (s.empty() || ec != std::errc{} || ptr != end) {
        throw std::invalid_argument("неверное время события: " + s);
    }
    return v;
}

const std::string& CheckType(const std::string& type) {
    if (type.empty()) {
        throw std::invalid_argument("у события пустой тип");
    }
    return type;
}

}  // namespace

EventType EventTypeFromString(const std::string& type) {
    if (type == "process_start") return EventType::kProcessStart;
    if (type == "process_end") return EventType::kProcessEnd;
    if (type == "file_create") return EventType::kFileCreate;
    if (type == "file_write") return EventType::kFileWrite;
    if (type == "file_delete") return EventType::kFileDelete;
    if (type == "file_move") return EventType::kFileMove;
    if (type == "net_connect") return EventType::kNetConnect;
    return EventType::kOther;
}

const char* EventTypeName(EventType type) {
    switch (type) {
        case EventType::kProcessStart: return "process_start";
        case EventType::kProcessEnd:   return "process_end";
        case EventType::kFileCreate:   return "file_create";
        case EventType::kFileWrite:    return "file_write";
        case EventType::kFileDelete:   return "file_delete";
        case EventType::kFileMove:     return "file_move";
        case EventType::kNetConnect:   return "net_connect";
        case EventType::kOther:        return "other";
    }
    return "other";
}

Event::Event(const EventParts& parts)
    : ts_{ParseTs(parts.ts)},
      raw_ts_(parts.ts),
      type_(CheckType(parts.type)),
      event_type_(EventTypeFromString(type_)),
      pid_(parts.pid),            // pid не проверяется
      fields_(parts.fields) {}

std::string ToString(const Event& event) {
    std::string s = "ts=" + event.raw_ts() + " type=" + event.type() + " pid=" + event.pid();
    for (const Field& f : event.fields()) {
        s += " " + f.key + "=" + f.value;
    }
    return s;
}

std::ostream& operator<<(std::ostream& out, const Event& event) {
    return out << ToString(event);   // формат только в ToString
}

}  // namespace nano_edr
