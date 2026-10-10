// Крутой агент: читает журнал событий построчно и считает строки.
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
#include <agent_rules.h>
#include <parse.h>

void print_resume(bool &quiet, long long &lines, long long &comments, std::map<std::string, int> &types) {
    if (!quiet) {
        std::print("строк {}, из них комментариев {}\n", lines, comments);
        for (auto& [type, value] : types) {
            std::print("{} раз встретился тип {}\n", value, type);
        }
    }
}

int parse(int &argc, char** &argv, std::string &path, bool &quiet, std::size_t &window_size, std::ifstream &log) {
    for (int i = 1; i < argc; ++i) {
        const std::string ar = argv[i];
        if (ar == "--quiet") {
            quiet = true;
        } else if (ar == "--window-size") {
            if (i + 1 >= argc) {
                std::print(stderr, "не хватает значения для --window-size\n");
                return 1;
            }

            const std::string value = argv[i + 1];
            std::size_t parsed = 0;
            const char* begin = value.data();
            const char* end = begin + value.size();
            const auto [ptr, ec] = std::from_chars(begin, end, parsed);
            if (ec != std::errc{} || ptr != end) {
                std::print(stderr, "неверное значение для --window-size: {}\n", value);
                return 1;
            }
            window_size = parsed;
            ++i;
        } else {
            path = ar;
        }
    }
    if (path.empty()) {
        std::print(stderr, "не указан путь к журналу\n");
        return 1;
    }
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", path);
        return 1;
    }
    
    return 0;
}

void print_ctx(const nano_edr::EventNode* prev) {
    if (prev){
        nano_edr::EventNode *tail = prev->next;
        int i = tail ? 2 : 1;
        while (prev) {
            std::print("[CTX] -{}: ts={} type={} pid={}\n", i, prev->event.ts, prev->event.type, prev->event.pid);
            prev = prev->next;
            i--;
        } 
    }
}

void processing_line(nano_edr::EventList &window, const std::string &line, long long lines, long long comments, std::map<std::string, int> &types, const nano_edr::EventNode* &prev, bool quiet) {
    nano_edr::Event event;

    if (nano_edr::IsBlankOrComment(&line)) {
        ++comments;
        return;
    }
    if (!nano_edr::ParseEventLine(&line, &event)) {
        return;
    }

    const nano_edr::EventNode* tail = window.tail;
    int matched = 0;
    ++lines;
    ++types[event.type];

    matched += nano_edr::CheckRules(event, nano_edr::AgentRules(), nano_edr::AgentRuleCount());
    if(matched && !quiet)   print_ctx(prev);
    prev = tail;

    nano_edr::ListPushBack(&window, &event);

    if (window.capacity == 1)   prev = nullptr;
}

int agent_work(std::ifstream &log, bool quiet, std::size_t window_size) {
    long long lines = 0;
    long long comments = 0;
    std::string line = "";
    std::map<std::string, int> types;

    const nano_edr::EventNode* prev{};
    nano_edr::EventList window;
    window.capacity = window_size;

    while (std::getline(log, line)) {
        processing_line(window, line, lines, comments, types, prev, quiet);
    }

    print_resume(quiet, lines, comments, types);
    return 0;
}

int main(int argc, char** argv) {

    try {
        bool quiet = false;
        std::string path;
        std::ifstream log(path);
        std::size_t window_size = 64;

        int return_code_parse = parse(argc, argv, path, quiet, window_size, log);
        if (return_code_parse != 0) {
            return return_code_parse;
        }


        agent_work(log, quiet, window_size);
        
    } catch (const std::exception& e) {
        std::print(stderr, "ошибка: {}\n", e.what());
        return 1;
    }
}
