#pragma once

#include <concepts>

#include "array_sequence.h"

// Концепт алгебраической структуры упорядоченного поля.
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

// Один кусок кусочно-заданной функции.
// coefficients задаются относительно left:
// a0 + a1 * (x - left) + a2 * (x - left)^2 + ...
template <Field T> struct FunctionSegment {
    T left;
    T right;
    DynamicArray<T> coefficients;
};

// в меню и гуи зависим от этого класса
// так что юзер может передлать любую реализацию интерполятора
template <Field T> class Interpolator {
  public:
    // алгоритм интерполяции должен реализовать наследник
    virtual Sequence<FunctionSegment<T>> *interpolate(const Sequence<Point<T>> &points) const = 0;

    // метод для вычисления значения интерполированной функции в точке x.
    // реализуем в базовом классе так как логика одинаковая для любого интерполятора
    Option<T> evaluate(const Sequence<FunctionSegment<T>> &segments, const T &x) const;

    virtual ~Interpolator() {}
};

#include "detail/interpolator.tpp"
