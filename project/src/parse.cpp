#include <cctype>

#include "parse.h"

namespace nano_edr {
namespace {

bool IsSpace(char ch) {
    return ch == ' ' || ch == '\t';
}

}  // namespace

bool IsBlankOrComment(const std::string* line) {
    if (line == nullptr) {
        return true;
    }
    const char* ws = " \t";

    size_t pos = line->find_first_not_of(ws);
    if (pos == std::string::npos) {
        return true;
    }

    return (*line)[pos] == '#' || (*line)[pos] == ';';
}

// ts=1730000003600 type=process_end pid=1042 exit=0
// ts=1730000004200 type=net_connect pid=1101 raddr=185.12.3.4 rport=443 domain=evil.example
// ts=1730000006000 type=process_start pid=1156 ppid=1101 image="C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe"


bool ParseEventLine(const std::string* line, Event* out) {
    if(!line || !out || IsBlankOrComment(line)) {
        return false;
    }

    const std::string& s = *line;
    std::size_t i = 0;
    bool has_ts = false;
    bool has_type = false;
    bool has_pid = false;

    while (i < s.size()) {
        while (i < s.size() && IsSpace(s[i])) {
            ++i;
        }
        if (i >= s.size()) {
            break;
        }

        const std::size_t key_begin = i;
        while (i < s.size() && !IsSpace(s[i]) && s[i] != '=') {
            ++i;
        }

        if (i == key_begin || i >= s.size() || s[i] != '=') {
            return false;
        }

        const std::string key = s.substr(key_begin, i - key_begin);
        if (key.empty()) {
            return false;
        }
        ++i;

        std::string value;
        if (i < s.size() && s[i] == '"') {
            ++i;
            while (i < s.size() && s[i] != '"') {
                value.push_back(s[i]);
                ++i;
            }
            if (i >= s.size()) {
                return false;
            }
            ++i;
            if (i < s.size() && !IsSpace(s[i])) {
                return false;
            }
        } else {
            const std::size_t value_begin = i;
            while (i < s.size() && !IsSpace(s[i])) {
                ++i;
            }
            value = s.substr(value_begin, i - value_begin);
        }

        if (key == "ts") {
            if (!has_ts) {
                out->ts = value;
                has_ts = true;
            } else {
                out->fields.push_back(Field{key, value});
            }
        } else if (key == "type") {
            if (!has_type) {
                out->type = value;
                has_type = true;
            } else {
                out->fields.push_back(Field{key, value});
            }
        } else if (key == "pid") {
            if (!has_pid) {
                out->pid = value;
                has_pid = true;
            } else {
                out->fields.push_back(Field{key, value});
            }
        } else {
            out->fields.push_back(Field{key, value});
        }
    }

    return has_ts && has_type;
}

}  // namespace nano_edr