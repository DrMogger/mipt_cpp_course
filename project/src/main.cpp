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
#include <utility>

#include <agent.h>
#include <sources.h>
#include "alert_sink.h"
#include "edr_handle.h"
#include "field_traits.h"

int parse(int argc, char** argv, std::string &path, bool &quiet, bool &file_mode,
          std::string& out, bool& help) {
    for (int i = 1; i < argc; ++i) {
        const std::string ar = argv[i];
        if (ar == "--help" || ar == "-h") {
            help = true;
            return 0;
        } else if (ar == "--quiet") {
            quiet = true;
        } else if (ar == "--out") {
            if (i + 1 >= argc || std::string(argv[i + 1]).starts_with("-")) {
                std::print(stderr, "не хватает значения для --out\n");
                return 2;
            }
            out = argv[++i];
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
        std::string out;
        bool help = false;

        int return_code_parse = parse(argc, argv, path, quiet, file_mode, out, help);
        if (return_code_parse != 0) {
            return return_code_parse;
        }
        if (help) {
            std::print("nano-edr [--quiet] [--file] [--out файл] журнал\n"
                       "  --quiet      скрыть контекст и сводку\n"
                       "  --file       читать журнал напрямую\n"
                       "  --out файл   также записывать детекты в файл\n");
            return 0;
        }
        nano_edr::EdrHandle edr(path);
        const std::string period = edr.Config("dedup_period_ms", "10000");
        uint64_t dedup_period_ms = 0;
        if (!nano_edr::FieldTraits<uint64_t>::Parse(period, &dedup_period_ms)) {
            std::print(stderr, "[WARN] dedup_period_ms не разбирается: {}\n", period);
            dedup_period_ms = 10000;
        }
        nano_edr::Agent agent(quiet, dedup_period_ms);
        nano_edr::FanOutSink targets;
        targets.Add(nano_edr::ConsoleSink());
        if (!out.empty()) targets.Add(nano_edr::FileSink(out));
        targets.Add(nano_edr::EdrSink(&edr));
        agent.Subscribe(std::move(targets));
        std::unique_ptr<nano_edr::IEventSource> source;
        if (file_mode) {
            source = std::make_unique<nano_edr::FileSource>(path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(path);
        }
        source->Run(&agent);
        if (edr.Flush() != EDR_OK) {
            std::print(stderr, "[WARN] детекты не отправлены\n");
        }
        agent.PrintSummary();
        return 0;
    } catch (const std::exception& e) {
        std::print(stderr, "ошибка: {}\n", e.what());
        return 1;
    }
}
