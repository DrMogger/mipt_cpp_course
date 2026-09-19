#include "kit/include/parse.h"

namespace nano_edr

const char* ws = " \t";
bool IsBlankOrComment(const std::string* line) {
    if (line->empty()) {
        return true;
    }

    size_t pos = line->find_first_not_of(ws);
    if (pos == std::string::npos) {
        return true;
    }

    return (*line)[pos] == '#' || (*line[pos]) == ';';
}