#pragma once

#include "linear_spline.h"

#include <stdexcept>

template <Field T>
Sequence<Function<T> *> *
LinearSplineInterpolator<T>::interpolate(const Sequence<Point<T>> &points) const {
    const int count = points.get_count();
    if (count < 2) {
        throw std::invalid_argument(
            "LinearSplineInterpolator::interpolate: at least 2 points required");
    }

    MutableArraySequence<Function<T> *> *segments = new MutableArraySequence<Function<T> *>();

    for (int index = 0; index < count - 1; ++index) {
        const T segment_width = points.get(index + 1).x - points.get(index).x;
        if (segment_width <= T{0}) {
            delete segments;
            throw std::invalid_argument("LinearSplineInterpolator::interpolate: point X values "
                                        "must be strictly increasing");
        }

        DynamicArray<T> coefficients(2);
        coefficients.set(0, points.get(index).y);
        coefficients.set(1, (points.get(index + 1).y - points.get(index).y) / segment_width);

        Function<T> *segment =
            new PolynomialFunction<T>(points.get(index).x, points.get(index + 1).x, coefficients);
        segments->append(segment);
    }

    return segments;
}
