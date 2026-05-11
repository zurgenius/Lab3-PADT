#pragma once

#include "dynamic_array.h"
#include "field.h"
#include "option.h"

// Абстрактная функция на отрезке [left, right].
template <Field T> class Function {
  public:
    virtual T get_left() const = 0;
    virtual T get_right() const = 0;
    virtual Option<T> try_evaluate(const T &x) const = 0;
    virtual ~Function() {}
};

// Полином, заданный относительно левой границы: a0 + a1*(x-left) + a2*(x-left)^2 + ...
template <Field T> class PolynomialFunction : public Function<T> {
  private:
    T left;
    T right;
    DynamicArray<T> coefficients;

  public:
    PolynomialFunction(const T &left, const T &right, const DynamicArray<T> &coefficients)
        : left(left), right(right), coefficients(coefficients) {}

    T get_left() const override { return left; }

    T get_right() const override { return right; }

    // считаем значени полинома для этого x в выбранном сегменте
    // по схеме Горнера
    Option<T> try_evaluate(const T &x) const override {
        const int count = coefficients.get_size();
        if (count == 0) {
            return Option<T>::None();
        }

        const T dx = x - left;
        T value = coefficients.get(count - 1);
        for (int index = count - 2; index >= 0; --index) {
            value = value * dx + coefficients.get(index);
        }

        return Option<T>::Some(value);
    }

    const DynamicArray<T> &get_coefficients() const { return coefficients; }
};
