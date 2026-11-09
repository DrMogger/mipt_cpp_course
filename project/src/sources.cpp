#include "sources.h"

#include <cstdio>
#include <exception>
#include <stdexcept>

#include <event.h>
#include <parse.h>

namespace nano_edr {
namespace {

void NotifyBroken(IEventHandler* handler) noexcept {
    try {
        if (handler) {
            handler->OnBroken();
        }
    } catch (...) {
        // Ошибка уведомления не должна выйти за границу C-колбэка.
    }
}

}  // namespace

FileSource::FileSource(const std::string& path) : path_(path), file_(path) {
    if (!file_) {
        throw std::runtime_error("не удалось открыть журнал: " + path);
    }
}

void FileSource::Run(IEventHandler* handler) {
    std::string line;
    while (std::getline(file_, line)) {
        try {
            if (IsBlankOrComment(line)) {
                handler->OnSkipped();
                continue;
            }
            EventParts parts;
            if (!ParseEventParts(line, &parts)) {
                NotifyBroken(handler);
                std::fprintf(stderr, "пропущена строка журнала: %s\n", line.c_str());
                continue;
            }
            Event event(parts);
            handler->HandleEvent(event);
        } catch (const std::exception& e) {
            NotifyBroken(handler);
            std::fprintf(stderr, "пропущено событие: %s\n", e.what());
        } catch (...) {
            NotifyBroken(handler);
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
        static_cast<IEventHandler*>(ctx)->HandleEvent(event);
    } catch (const std::exception& e) {
        NotifyBroken(static_cast<IEventHandler*>(ctx));
        std::fprintf(stderr, "пропущено событие: %s\n", e.what());
    } catch (...) {
        NotifyBroken(static_cast<IEventHandler*>(ctx));
        std::fprintf(stderr, "пропущено событие: неизвестная ошибка\n");
    }
}

void OsSource::Run(IEventHandler* handler) {
    handle_.Subscribe(OsSource::Trampoline, handler);
    handle_.Start();

    os_status status = OS_TIMEOUT;
    while ((status = handle_.Wait(1000)) == OS_TIMEOUT) {
        handler->OnTick();
    }
    if (status != OS_OK) {
        throw std::runtime_error(std::string("os_wait: ") + os_status_str(status));
    }
}

}  // namespace nano_edr
