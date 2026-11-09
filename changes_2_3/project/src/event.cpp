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

Event::Event(const EventParts& parts)
    : ts_{ParseTs(parts.ts)},
      raw_ts_(parts.ts),
      type_(CheckType(parts.type)),
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