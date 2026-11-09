# Исходные файлы до занятия 2.2

Здесь сохранены полные копии 11 файлов в том виде, в котором они были
перед выполнением `practice/2.2.md`. Структура каталогов сохранена.
Готовое решение находится в рабочем каталоге `project/`.

| Старый файл | Что изменено в рабочем проекте |
| --- | --- |
| [agent.cpp](project/src/agent.cpp) | Агент хранит объекты `MatchRule`, проверяет их и печатает счётчики срабатываний. |
| [agent_rules.h](project/src/agent_rules.h) | Вместо доступа к таблице объявлена функция `BuildAgentRules()`. |
| [agent_rules.cpp](project/src/agent_rules.cpp) | Пять правил собраны из условий как семь объектов с действиями. |
| [rules.cpp](project/src/rules.cpp) | Осталась функция `SeverityName`; проверка правил перенесена в агента. |
| [fields.cpp](project/src/fields.cpp) | Сравнения без учёта регистра используют общие функции из `text.cpp`. |
| [sources.cpp](project/src/sources.cpp) | Источники не сообщают агенту о строках и пропусках; проверка комментариев включена в `try`. Трамплин перенесён из агента в `OsSource`. |
| [event_list.cpp](project/src/event_list.cpp) | Вместо цикла вытеснения используется одна проверка заполненности: инвариант списка гарантирует, что достаточно удалить один узел. |
| [agent.h](project/kit/include/l2.1/agent.h) | Новая версия находится в `project/src/agent.h` и содержит вектор правил. |
| [sources.h](project/kit/include/l2.1/sources.h) | Копия заголовка размещена в `project/src/sources.h`, чтобы он был доступен с комплектом 2.2. |
| [CMakeUserPresets.json](project/CMakeUserPresets.json) | Номер занятия в действующих пресетах CLion изменён с 2.1 на 2.2. |
| [CMakeUserPresets.json.example](clion/CMakeUserPresets.json.example) | Такое же изменение внесено в образец пресетов. |

В рабочий проект добавлены `text.h`, `text.cpp`, `conditions.cpp`, `rule.cpp`,
а также `src/agent.h` и `src/sources.h`. Ответы к упражнению находятся в
`project/tasks/2.2/exercise/answers.md`. Эти новые файлы в архив не входят.

Исходные заголовки в `project/kit/include/l2.1/` оставлены без изменений.
Сборка, запуск упражнения и тесты не выполнялись. Контейнер не менялся.
