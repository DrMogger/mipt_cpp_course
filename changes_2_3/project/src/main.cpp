// Крутой агент: получает события от os.h или читает журнал сам.
//
// Запуск:
//   nano-edr <журнал.log>
//   nano-edr --file <журнал.log>
#include <charconv>
#include <cstdio>
#include <exception>
#include <print>
#include <string>
#include <system_error>

#include <agent.h>
#include <sources.h>

int parse(int argc, char** argv, std::string &path, bool &quiet, std::size_t &window_size, bool &file_mode) {
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
        } else if (ar == "--file") {
            if (i + 1 >= argc || std::string(argv[i + 1]).starts_with("-")) {
                std::print(stderr, "не хватает значения для --file\n");
                return 2;
            }
            if (!path.empty()) {
                std::print(stderr, "путь к журналу указан дважды\n");
                return 2;
            }
            file_mode = true;
            path = argv[++i];
        } else if (ar.starts_with("-")) {
            std::print(stderr, "неизвестная опция: {}\n", ar);
            return 2;
        } else {
            if (!path.empty()) {
                std::print(stderr, "путь к журналу указан дважды\n");
                return 2;
            }
            path = ar;
        }
    }
    if (path.empty()) {
        std::print(stderr, "не указан путь к журналу\n");
        return 2;
    }
    
    return 0;
}

int main(int argc, char** argv) {

    try {
        bool quiet = false;
        bool file_mode = false;
        std::string path;
        std::size_t window_size = 64;

        int return_code_parse = parse(argc, argv, path, quiet, window_size, file_mode);
        if (return_code_parse != 0) {
            return return_code_parse;
        }
        nano_edr::Agent agent(window_size, quiet);
        if (file_mode) {
            nano_edr::FileSource source(path);
            source.Run(&agent);
        } else {
            nano_edr::OsSource source(path);
            source.Run(&agent);
        }
        agent.PrintSummary();
        return 0;
    } catch (const std::exception& e) {
        std::print(stderr, "ошибка: {}\n", e.what());
        return 1;
    }
}
