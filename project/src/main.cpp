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
#include <cstdio>
#include <fstream>
#include <print>
#include <string>
#include <vector>
#include <map>

int main(int argc, char** argv) {
    // Аргументы разбираются грубо: путь к журналу и ничего больше. Остальное,
    // включая --quiet, добавляется по заданию.
    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return 2;
    }

    bool quiet = false;
    std::string path = "";
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--quiet")   quiet = true;
        else    path = argv[i];
    }

    std::ifstream log(path);
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", path);
        return 2;
    }

    long long lines = 0;
    long long comments = 0;
    std::string line;
    std::vector<std::string> patterns = {
        "wscript.exe",
        ".locked",
        "certutil.exe",
        "\\Startup\\"
    };

    std::map<std::string, int> types;
    const std::string key = "type=";

    while (std::getline(log, line)) {
        // Счётчик увеличивается до всех проверок: он считает строки файла,
        // а не события. Номер, посчитанный по событиям, бесполезен — по нему
        // нельзя открыть файл и посмотреть.
        ++lines;

        // Строки-комментарии в журнале начинаются с '#'. Они не события,
        // и детекта по ним быть не должно.
        if (!line.empty() && (line[0] == '#' || line[0] == ';')) {
            ++comments;
            continue;
        }

        // >>> Здесь начинается занятие 1.1.
        //
        // Проверка признаков и печать детекта. Номер строки, который нужен
        // в выводе, — это lines.

        // ts=1730000003600 type=process_end pid=1042 exit=0

        size_t pos = line.find(key);
        if (pos != std::string::npos) {
            pos += key.size();
            size_t end = pos;

            while (end < line.size() && line[end] != ' ')   ++end;
            types[line.substr(pos, end-pos)]++;
        }

        for (const auto& pattern : patterns) {
            if (line.find(pattern) != std::string::npos) {
                std::print("[DETECT] строка {}, признак {}: {}\n", lines, pattern, line);
            }
        }
    }

    if (!quiet){  
        std::print("строк {}, из них комментариев {}\n", lines, comments);
        for (auto& [type, value] : types) {
            std::print("{} раз встретился тип {}\n", value, type);
        }
    }
    return 0;
}
