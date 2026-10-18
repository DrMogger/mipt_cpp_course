#include "sources.h"

#include <stdexcept>

#include <event.h>
#include <parse.h>

namespace nano_edr {

FileSource::FileSource(const std::string& path) : path_(path), file_(path) {
    if (!file_) {
        throw std::runtime_error("не удалось открыть журнал: " + path);
    }
}

void FileSource::Run(Agent* agent) {
    std::string line;
    while (std::getline(file_, line)) {
        if (IsBlankOrComment(line)) {
            agent->CountComment();
            continue;
        }
        EventParts parts;
        if (!ParseEventParts(line, &parts)) {
            agent->CountSkipped();
            continue;
        }
        try {
            Event event(parts);
            agent->HandleEvent(event);
        } catch (const std::exception&) {
            agent->CountSkipped();
        }
    }
}
}