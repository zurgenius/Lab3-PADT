# Lab 2

Структура проекта:
- `include/` — публичные заголовки
- `include/detail/` — шаблонные реализации `*.tpp`
- `src/` — нетемплейтные реализации и консольное меню
- `tests/` — модульные тесты

Команды:
- `make program` — сконфигурировать CMake, собрать программу и обновить локальный `./program`
- `make tests` — сконфигурировать CMake, собрать тесты и запустить `ctest`
- `make lint` — собрать проект через отдельный CMake build с `-Werror`
- `make format` — отформатировать код через `clang-format`
- `cmake -S . -B build` — ручная конфигурация проекта
- `cmake --build build` — ручная сборка целей
- `ctest --test-dir build --output-on-failure` — ручной запуск тестов

Примечание:
- при первой CMake-сборке GoogleTest подтягивается автоматически через `FetchContent`

Реализовано:
- `DynamicArray`
- `LinkedList`
- `Sequence`, `ArraySequence`, `ListSequence`
- mutable / immutable варианты последовательностей
- `Bit` и `BitSequence`
- `Option<T>`
- итераторы
- `split`, `slice`, `min_max_avg`
- консольный UI
