//
// Created by Дмитрий Карпович on 04.11.2026.
//
#include "field_traits.h"
#include <charconv>
#include <cctype>

namespace nano_edr {
    bool FieldTraits<std::string>::Parse(const std::string& text, std::string* out) {
        *out = text;
        return true;
    };
    bool FieldTraits<uint64_t>::Parse(const std::string& text, uint64_t* out) {
        uint64_t value = 0;

        const char* begin = text.data();
        const char* end = begin + text.size();
        const auto [ptr, ec] = std::from_chars(begin, end, *out);
        if (ec != std::errc{} || ptr != end) {
            return false;
        }
        *out = value;
        return true;
    };
    bool FieldTraits<bool>::Parse(const std::string& text, bool* out) {
        std::string lower_text = text;
        std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (lower_text == "1" || lower_text == "true" || lower_text == "yes") {
            *out = true;
            return true;
        } else if (lower_text == "0" || lower_text == "false" || lower_text == "no") {
            *out = false;
            return true;
        }
        return false;
    };
};