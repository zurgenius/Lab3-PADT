#pragma once

#include <stdexcept>

#include "interpolator.h"

// Кусочно-линейная интерполяция: на каждом отрезке строится прямая между соседними узлами.
template <Field T> class LinearSplineInterpolator : public Interpolator<T> {
  public:
    Sequence<FunctionSegment<T>> *interpolate(const Sequence<Point<T>> &points) const override;
};

#include "detail/linear_spline.tpp"
