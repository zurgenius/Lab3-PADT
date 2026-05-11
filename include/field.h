#pragma once

#include <concepts>

// Концепт алгебраической структуры упорядоченного поля.
// С++20 чтобы удобно сузить тип T в шаблоне.
template <typename T>
concept Field = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
    { a - b } -> std::same_as<T>;
    { a * b } -> std::same_as<T>;
    { a / b } -> std::same_as<T>;
    { -a } -> std::same_as<T>;
    { a < b } -> std::convertible_to<bool>;
    T{0};
    T{1};
    T{2};
} && std::totally_ordered<T> && requires {
    requires(T{1} != T{0});
    requires(T{1} / T{2} != T{0});
};

// Точка (x, f(x)) для интерполяции.
template <Field T> struct Point {
    T x;
    T y;
};
