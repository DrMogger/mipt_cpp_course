#include "sources.h"

#include <cstdio>
#include <exception>
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
        try {
            if (IsBlankOrComment(line)) {
                continue;
            }
            EventParts parts;
            if (!ParseEventParts(line, &parts)) {
                std::fprintf(stderr, "пропущена строка журнала: %s\n", line.c_str());
                continue;
            }
            Event event(parts);
            agent->HandleEvent(event);
        } catch (const std::exception& e) {
            std::fprintf(stderr, "пропущено событие: %s\n", e.what());
        } catch (...) {
            std::fprintf(stderr, "пропущено событие: неизвестная ошибка\n");
        }
    }
    if (file_.bad() || !file_.eof()) {
        throw std::runtime_error("не удалось прочитать журнал: " + path_);
    }
}

OsSource::OsSource(const std::string& config_path) : handle_(config_path) {}

void OsSource::Trampoline(const os_event* ev, void* ctx) noexcept {
    try {
        if (!ev || !ctx) {
            return;
        }
        EventParts parts;
        parts.ts = std::to_string(ev->ts);
        parts.type = ev->type;
        if (ev->pid != 0) {
            parts.pid = std::to_string(ev->pid);
        }
        parts.fields.reserve(ev->field_count);
        for (size_t i = 0; i < ev->field_count; ++i) {
            parts.fields.push_back(Field{ev->fields[i].key, ev->fields[i].value});
        }

        Event event(parts);
        static_cast<Agent*>(ctx)->HandleEvent(event);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "пропущено событие: %s\n", e.what());
    } catch (...) {
        std::fprintf(stderr, "пропущено событие: неизвестная ошибка\n");
    }
}

void OsSource::Run(Agent* agent) {
    handle_.Subscribe(OsSource::Trampoline, agent);
    handle_.Start();

    os_status status = OS_TIMEOUT;
    while ((status = handle_.Wait(1000)) == OS_TIMEOUT) {
    }
    if (status != OS_OK) {
        throw std::runtime_error(std::string("os_wait: ") + os_status_str(status));
    }
}

}  // namespace nano_edr
