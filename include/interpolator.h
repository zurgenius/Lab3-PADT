#pragma once

#include "field.h"
#include "function.h"
#include "option.h"
#include "sequence.h"

// в меню и гуи зависим от этого класса
// так что юзер может передлать любую реализацию интерполятора
template <Field T> class Interpolator {
  public:
    // алгоритм интерполяции должен реализовать наследник
    virtual Sequence<Function<T> *> *interpolate(const Sequence<Point<T>> &points) const = 0;

    // метод для вычисления значения интерполированной функции в точке x.
    // реализуем в базовом классе так как логика одинаковая для любого интерполятора
    Option<T> evaluate(const Sequence<Function<T> *> &segments, const T &x) const;

    virtual ~Interpolator() {}
};

#include "detail/interpolator.tpp"
