#include <cstdint>
#include <string>
#include <stdexcept>

#include "event.h"
#include "fields.h"
#include "text.h"

namespace nano_edr {
const std::string* FindField(const Event& event, const std::string& key) {
    if (key == "ts") {
        return &event.raw_ts();
    }
    if (key == "type") {
        return &event.type();
    }
    if (key == "pid") {
        return &event.pid();
    }
    for(const auto& field : event.fields()) {
        if (field.key == key) {
            return &field.value;
        }
    }
    return nullptr;
}


const std::string& GetRequiredField(const Event& event, const std::string& key) {
    const auto* value = FindField(event, key);
    if (!value) {
        throw std::invalid_argument("Required field not found: " + key);
    }
    return *value;
}

bool GetIntField(const Event& event, const std::string& key, uint64_t* out) {
    if (!out) {
        return false;
    }
    const auto* value = FindField(event, key);
    if (!value) {
        return false;
    }
    try {
        size_t pos = 0;
        if (value->empty() || value->front() == '-') {
            return false;
        }
        uint64_t parsed = std::stoull(*value, &pos);
        if (pos != value->size()) {
            return false;
        }
        *out = parsed;
        return true;
    } catch (const std::invalid_argument& e) {
        return false;
    } catch (const std::out_of_range& e) {
        return false;
    }
}

uint64_t GetIntField(const Event& event, const std::string& key,
                     uint64_t fallback) {
    GetIntField(event, key, &fallback);
    return fallback;
}


bool IsProcessStart(const Event& event) {
    return event.type() == "process_start";
}
bool IsFileWrite(const Event& event) {
    return event.type() == "file_write";
}
bool IsNetConnect(const Event& event) {
    return event.type() == "net_connect";
}


bool PathEndsWith(const Event& event, const std::string& suffix) {
    const auto* path = FindField(event, "path");
    if (!path) {
        return false;
    }
    return EndsWithIgnoreCase(NormalizePath(*path), NormalizePath(suffix));
}

// Подстрока в командной строке. Тоже без учёта регистра и по той же причине.
bool CommandLineContains(const Event& event, const std::string& needle) {
    const auto* cmdline = FindField(event, "cmdline");
    if (!cmdline) {
        return false;
    }
    return ContainsIgnoreCase(NormalizePath(*cmdline), NormalizePath(needle));
}

std::string NormalizePath(const std::string& path){
    std::string result = "";

    for (char c : ToLower(path)) {
        char normalized = c == '/' ? '\\' : c;
        if (normalized == '\\' && !result.empty() && result.back() == '\\') {
            continue;
        }
        result += normalized;
    }
    for (const std::string& placeholder: {"%temp%", "%tmp%"}) {
        if (size_t pos = result.find(placeholder); pos != std::string::npos) {
            result.replace(pos, placeholder.size(), "\\appdata\\local\\temp");
        }
    }
    return result;
}
}
