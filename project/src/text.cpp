#include "text.h"

#include <cctype>

namespace nano_edr {

std::string ToLower(const std::string& text) {
    std::string result = text;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

bool EqualsIgnoreCase(const std::string& text, const std::string& value) {
    return ToLower(text) == ToLower(value);
}

bool ContainsIgnoreCase(const std::string& text, const std::string& needle) {
    return ToLower(text).find(ToLower(needle)) != std::string::npos;
}

bool EndsWithIgnoreCase(const std::string& text, const std::string& suffix) {
    return ToLower(text).ends_with(ToLower(suffix));
}

}  // namespace nano_edr
