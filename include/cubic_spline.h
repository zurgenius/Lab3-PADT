#pragma once

#include <stdexcept>

#include "array_sequence.h"
#include "interpolator.h"

// пример конкретной реализации интерполятора - кубический сплайн
template <Field T> class CubicSplineInterpolator : public Interpolator<T> {
  private:
    // Решение трёхдиагональной системы методом прогонки (Томаса).
    void solve_tridiag(const DynamicArray<T> &lower_diag, const DynamicArray<T> &main_diag,
                       const DynamicArray<T> &upper_diag, const DynamicArray<T> &rhs,
                       DynamicArray<T> &solution, int size) const;

  public:
    Sequence<Function<T> *> *interpolate(const Sequence<Point<T>> &points) const override;
};

#include "detail/cubic_spline.tpp"
