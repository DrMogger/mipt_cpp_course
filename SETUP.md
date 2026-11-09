# CLion на macOS: существующее окружение курса

Проект использует существующий образ Linux x86-64 с clang 21, CMake,
Ninja и GDB. На этом Mac ему дан дополнительный тег `nano-edr-clion:local`.
Образ не пересобирался. Исходники остаются в этом репозитории и монтируются
в контейнер.

## Открытие и ежедневная работа

1. Запустить Docker Desktop.
2. Открыть в CLion корень `mipt_cpp_course`.
3. Дождаться загрузки CMake и выбрать профиль **CLion Debug**.
4. Нажать молоток **Build**: он собирает `project/build/clion-debug/nano-edr`.
5. Открыть **Terminal** в CLion. Новая вкладка сразу подключается к прежнему
   контейнеру курса и открывается в `/workspaces/mipt_cpp_course/project`.
6. Запустить программу со своими аргументами:

   ```bash
   ./build/clion-debug/nano-edr scenarios/phishing_macro.log
   ./build/clion-debug/nano-edr --quiet --file scenarios/ransomware.log
   ```

Контейнер автоматически запускается, если был остановлен. Команды `docker run`
или `docker exec` вводить вручную не требуется. Выход — обычная команда `exit`.

CLion видит весь репозиторий, включая лекции и постановки занятий.
Файл `.idea/misc.xml` указывает, что CMake-проект находится в `project/`.
Рабочий каталог программы — `project/`, поэтому пути `scenarios/...`
остаются такими же, как в курсе.

Для запуска кнопкой **Run** можно выбрать **nano-edr — phishing_macro**.
Для другого сценария открыть **Run → Edit Configurations**, выбрать
**nano-edr — phishing_macro** и изменить **Program arguments**, например
на `scenarios/ransomware.log` или `scenarios/phishing_macro.cfg`.

### Восстановление терминала

Автоподключение настроено только для этого проекта через
**Settings → Tools → Terminal → Project Settings**:

- Start directory: `$PROJECT_DIR$/project`.
- Environment variables: `ZDOTDIR=<абсолютный путь к репозиторию>/clion/terminal-zsh`.

Shell path остаётся `/bin/zsh`. Файл `clion/terminal-zsh/.zshrc` передаёт
управление `clion/container-terminal.sh`: скрипт находит существующий контейнер
по метке каталога репозитория и подключается к нему. Если подходят несколько
контейнеров, в этих же переменных можно указать `MIPT_COURSE_CONTAINER=<имя>`.
После изменения настроек открыть новую вкладку Terminal.

## Расширения

В CLion 2025.2 уже есть C++, CMake, Docker, CTest, GDB,
Shell Script и Markdown. Ставить расширения из инструкции VS Code не нужно.
Подсказки, переходы по коду и ошибки берутся из CMake-модели CLion.
Форматирование использует существующий `project/.clang-format`.

Для Docker 29 нужен **CLion 2025.2.5 или новее**. В более ранних выпусках
встроенный Docker-плагин обращается к уже неподдерживаемому API 1.24;
это [известная ошибка JetBrains](https://youtrack.jetbrains.com/issue/IJPL-217878).
На этом Mac установлен CLion **2025.2.6.2**.

## Занятие и профили

Номер занятия хранится **в одном месте**:
`project/CMakeUserPresets.json`, поле `NANO_EDR_LESSON` в `clion-common`.
При переносе сохранено текущее значение **2.1**.

После смены номера выполнить **Tools → CMake → Reload CMake Project**.
Выданные `CMakeLists.txt` и `CMakePresets.json` менять не требуется.
Личные пресеты игнорируются Git; образец для восстановления лежит
в `clion/CMakeUserPresets.json.example`.

| Профиль CLion | Назначение | Каталог сборки |
|---|---|---|
| CLion Debug | обычная работа, символы отладки | `project/build/clion-debug` |
| CLion Release | оптимизированная сборка | `project/build/clion-release` |
| CLion ASan | AddressSanitizer и UBSan | `project/build/clion-asan` |
| CLion Strict | предупреждения считаются ошибками | `project/build/clion-strict` |

Изначально включён только **CLion Debug**. Остальные можно включить
в **Settings → Build, Execution, Deployment → CMake**.
Пресеты, которые CLion дополнительно импортировал автоматически,
включать не требуется.

Санитайзеры сохранены из курса; перенос не меняет ограничения их работы
под эмуляцией x86-64. Они не нужны для обычной сборки и запуска.

## Тесты

Выбрать **CTest — все тесты занятия** и нажать **Run**.
Конфигурация сначала собирает все цели, затем запускает CTest активного
профиля и показывает результаты в CLion.

Из терминала те же тесты запускаются так, из корня репозитория:

```bash
docker run --rm --platform linux/amd64 \
  --mount "type=bind,src=$PWD,dst=/workspaces/mipt_cpp_course" \
  --workdir /workspaces/mipt_cpp_course/project nano-edr-clion:local \
  bash -lc 'cmake --preset clion-debug && cmake --build --preset clion-debug --parallel 4 && ctest --preset clion-debug'
```

При переносе проверены сборка, обычный Run и CTest непосредственно из CLion:
**17 из 17 тестов занятия 2.1** прошли успешно.

## Отладка на Apple Silicon

В существующем контейнере Linux x86-64 обычный запуск GDB с `ptrace`
возвращает `Function not implemented`. Поэтому для пошаговой отладки
приготовлена отдельная конфигурация **nano-edr — QEMU Debug**.

1. Поставить breakpoint в `project/src/main.cpp`.
2. Выбрать **nano-edr — QEMU Debug**.
3. Нажать **Debug**.

Перед подключением автоматически запускается конфигурация **QEMU server**:
она собирает агент и запускает его со встроенным GDB-сервером эмулятора.
Встроенный GDB CLion подключается к `127.0.0.1:12345`; сопоставление
исходников из контейнера с локальными файлами задано в конфигурации.
Сервер доступен только на этом Mac.

Сценарий для отладки задаётся в **QEMU server → Script options**.
Изменение аргументов обычного запуска не меняет сценарий QEMU Debug.
Вывод агента можно посмотреть во время отладки командой:

```bash
docker logs nano-edr-clion-qemu
```

После завершения программы временный контейнер удаляется автоматически.
Если отладку прервали до завершения, остановить его можно так:

```bash
bash clion/qemu-debug.sh --stop
```

## Восстановление настроек CLion

Toolchain создаётся один раз в
**Settings → Build, Execution, Deployment → Toolchains → + → Docker**:

| Поле | Значение |
|---|---|
| Name | `Course Docker` |
| Docker connection | Docker Desktop, локальный Unix socket |
| Image | `nano-edr-clion:local` |
| CMake | `/usr/bin/cmake` |
| Build Tool | `/usr/bin/ninja` |
| C Compiler | `/usr/bin/clang` |
| C++ Compiler | `/usr/bin/clang++` |
| Debugger | `/usr/bin/gdb` |

В Container Settings задаётся `--platform=linux/amd64` и монтирование
каталога репозитория в `/workspaces/mipt_cpp_course`.
Имя toolchain должно совпадать с `Course Docker` в профилях.

Если личный файл пресетов отсутствует, восстановить его перед открытием IDE:

```bash
cp clion/CMakeUserPresets.json.example project/CMakeUserPresets.json
```

Справка JetBrains: [Docker toolchain](https://www.jetbrains.com/help/clion/clion-toolchains-in-docker.html),
[CMake presets](https://www.jetbrains.com/help/clion/cmake-presets.html).
