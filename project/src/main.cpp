// Крутой агент: получает события от os.h или читает журнал сам.
//
// Запуск:
//   nano-edr <журнал.log>
//   nano-edr --file <журнал.log>
#include <charconv>
#include <cstdio>
#include <exception>
#include <memory>
#include <print>
#include <string>
#include <system_error>

#include <agent.h>
#include <sources.h>

int parse(int argc, char** argv, std::string &path, bool &quiet, bool &file_mode) {
    for (int i = 1; i < argc; ++i) {
        const std::string ar = argv[i];
        if (ar == "--quiet") {
            quiet = true;
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

        int return_code_parse = parse(argc, argv, path, quiet, file_mode);
        if (return_code_parse != 0) {
            return return_code_parse;
        }
        nano_edr::Agent agent(quiet);
        std::unique_ptr<nano_edr::IEventSource> source;
        if (file_mode) {
            source = std::make_unique<nano_edr::FileSource>(path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(path);
        }
        source->Run(&agent);
        agent.PrintSummary();
        return 0;
    } catch (const std::exception& e) {
        std::print(stderr, "ошибка: {}\n", e.what());
        return 1;
    }
}
