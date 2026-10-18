#include "sources.h"

#include <cstdio>
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
        try {
            EventParts parts;
            if (!ParseEventParts(line, &parts)) {
                agent->CountSkipped();
                std::fprintf(stderr, "пропущена строка журнала: %s\n", line.c_str());
                continue;
            }
            Event event(parts);
            agent->HandleEvent(event);
        } catch (const std::exception& e) {
            agent->CountSkipped();
            std::fprintf(stderr, "пропущено событие: %s\n", e.what());
        } catch (...) {
            agent->CountSkipped();
            std::fprintf(stderr, "пропущено событие: неизвестная ошибка\n");
        }
    }
    if (file_.bad() || !file_.eof()) {
        throw std::runtime_error("не удалось прочитать журнал: " + path_);
    }
}

OsSource::OsSource(const std::string& config_path) : handle_(config_path) {}

void OsSource::Run(Agent* agent) {
    handle_.Subscribe(Agent::Trampoline, agent);
    handle_.Start();

    os_status status = OS_TIMEOUT;
    while ((status = handle_.Wait(1000)) == OS_TIMEOUT) {
    }
    if (status != OS_OK) {
        throw std::runtime_error(std::string("os_wait: ") + os_status_str(status));
    }
}

}  // namespace nano_edr
