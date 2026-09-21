// Каркас агента: читает журнал событий построчно и считает строки.
//
// Это заготовка занятия 1.1, а не решение. Детектов она не ищет — их вы
// добавите здесь же, в отмеченном месте ниже. Формат строки детекта, список
// признаков и правило про их порядок заданы в постановке занятия: по ним
// сравниваются эталоны.
//
// Весь код лежит в main, и на этом занятии так и надо: функции появятся
// на занятии 1.2, ссылки — на 1.3. Разбор аргументов, коды возврата и флаг
// --quiet — часть задания.
//
// Запуск:
//   nano-edr <журнал.log>
#include <charconv>
#include <cstdio>
#include <fstream>
#include <map>
#include <print>
#include <string>
#include <system_error>
#include <vector>

#include <event.h>
#include <event_list.h>
#include <parse.h>

void PrintContext(const nano_edr::EventList* window) {
    const nano_edr::EventNode* last = nullptr;
    const nano_edr::EventNode* prev = nullptr;

    for (const nano_edr::EventNode* node = window->head; node != nullptr; node = node->next) {
        prev = last;
        last = node;
    }
    if (prev != nullptr) {
        std::print("[CTX] -2: ts={} type={} pid={}\n", prev->event.ts, prev->event.type, prev->event.pid);
    }
    if (last != nullptr) {
        std::print("[CTX] -1: ts={} type={} pid={}\n", last->event.ts, last->event.type, last->event.pid);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return 2;
    }
    bool quiet = false;
    std::string path;
    std::size_t window_size = 64;
    for (int i = 1; i < argc; ++i) {
        const std::string ar = argv[i];
        if (ar == "--quiet") {
            quiet = true;
        } else if (ar == "--window-size") {
            if (i + 1 >= argc) {
                std::print(stderr, "не хватает значения для --window-size\n");
                return 2;
            }

            const std::string value = argv[i + 1];
            std::size_t parsed = 0;
            const char* begin = value.data();
            const char* end = begin + value.size();
            const auto [ptr, ec] = std::from_chars(begin, end, parsed);
            if (ec != std::errc{} || ptr != end) {
                std::print(stderr, "неверное значение для --window-size: {}\n", value);
                return 2;
            }
            window_size = parsed;
            ++i;
        } else {
            path = ar;
        }
    }
    std::ifstream log(path);
    if (path.empty()) {
        std::print(stderr, "не указан путь к журналу\n");
        return 2;
    }

    // bool quiet = false;
    // std::string path;
    // std::size_t window_size = 64;

    // for (int i = 1; i < argc; ++i) {
    //     std::string ar = argv[i];
    //     if (ar == "--quiet")   quiet = true;
    //     else if (ar == "--window-size")    window_size = std::stoi(argv[i+1]);
    //     else    path = ar;
    // }

    // std::ifstream log(path);
    // if (!log) {
    //     std::print(stderr, "не удалось открыть журнал: {}\n", path);
    //     return 2;
    // }

    long long lines = 0;
    long long comments = 0;
    std::string line = "";
    std::vector<std::string> patterns = {
        "wscript.exe",
        ".locked",
        "certutil.exe",
        "\\Startup\\"
    };

    std::map<std::string, int> types;

    nano_edr::EventList window;
    window.capacity = window_size;

    while (std::getline(log, line)) {
        ++lines;

        if (nano_edr::IsBlankOrComment(&line)) {
            ++comments;
            continue;
        }

        nano_edr::Event event;
        if (!nano_edr::ParseEventLine(&line, &event)) {
            continue;
        }

        bool matched = false;
        for (const auto& pattern : patterns) {
            if (line.find(pattern) != std::string::npos) {
                std::print("[DETECT] строка {}, признак {}: {}\n", lines, pattern, line);
                matched = true;
            }
        }
        
        nano_edr::EventNode* prev = window.capacity == 1 ? nullptr : window.tail;
        nano_edr::ListPushBack(&window, &event);
        ++types[event.type];
        
        if (matched && !quiet) {
            nano_edr::EventNode* last = window.tail;

            if (prev != nullptr) {
            std::print("[CTX] -2: ts={} type={} pid={}\n", prev->event.ts, prev->event.type, prev->event.pid);
            }
            if (last != nullptr) {
                std::print("[CTX] -1: ts={} type={} pid={}\n", last->event.ts, last->event.type, last->event.pid);
            }
        }
    }

    if (!quiet) {
        std::print("строк {}, из них комментариев {}\n", lines, comments);
        for (auto& [type, value] : types) {
            std::print("{} раз встретился тип {}\n", value, type);
        }
    }
    return 0;
}
