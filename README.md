# Lab 2

Структура проекта:
- `include/` — публичные заголовки
- `include/detail/` — шаблонные реализации `*.tpp`
- `src/` — нетемплейтные реализации и консольное меню
- `tests/` — модульные тесты
- `googletest/` — локальный минимальный `gtest`-совместимый раннер для сборки тестов в этом репозитории

Команды:
- `make program` — собрать консольную программу
- `make tests` — собрать тесты
- `./tests_runner` — запустить тесты
- `make lint` — собрать проект в режиме без предупреждений
- `make format` — отформатировать код через `clang-format`

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
