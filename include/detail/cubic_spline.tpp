#pragma once

#include "cubic_spline.h"

#include <stdexcept>

template <Field T>
void CubicSplineInterpolator<T>::solve_tridiag(const DynamicArray<T> &lower_diag,
                                               const DynamicArray<T> &main_diag,
                                               const DynamicArray<T> &upper_diag,
                                               const DynamicArray<T> &rhs,
                                               DynamicArray<T> &solution, int size) const {
    DynamicArray<T> alpha(size - 1);
    DynamicArray<T> beta(size - 1);

    alpha.set(0, -upper_diag.get(0) / main_diag.get(0));
    beta.set(0, rhs.get(0) / main_diag.get(0));

    for (int index = 1; index < size - 1; ++index) {
        const T denominator = main_diag.get(index) + lower_diag.get(index) * alpha.get(index - 1);
        alpha.set(index, -upper_diag.get(index) / denominator);
        beta.set(index,
                 (rhs.get(index) - lower_diag.get(index) * beta.get(index - 1)) / denominator);
    }

    solution.set(size - 1,
                 (rhs.get(size - 1) - lower_diag.get(size - 1) * beta.get(size - 2)) /
                     (main_diag.get(size - 1) + lower_diag.get(size - 1) * alpha.get(size - 2)));

    for (int index = size - 2; index >= 0; --index) {
        solution.set(index, solution.get(index + 1) * alpha.get(index) + beta.get(index));
    }
}

template <Field T>
Sequence<FunctionSegment<T>> *
CubicSplineInterpolator<T>::interpolate(const Sequence<Point<T>> &points) const {
    const int count = points.get_count();
    if (count < 3) {
        throw std::invalid_argument(
            "CubicSplineInterpolator::interpolate: at least 3 points required");
    }

    DynamicArray<T> delta(count - 1);
    DynamicArray<T> h(count - 1);
    for (int index = 0; index < count - 1; ++index) {
        const T segment_width = points.get(index + 1).x - points.get(index).x;
        if (segment_width <= T{0}) {
            throw std::invalid_argument(
                "CubicSplineInterpolator::interpolate: point X values must be strictly increasing");
        }
        h.set(index, segment_width);
        delta.set(index, (points.get(index + 1).y - points.get(index).y) / segment_width);
    }

    DynamicArray<T> lower_diag(count);
    DynamicArray<T> main_diag(count);
    DynamicArray<T> upper_diag(count);
    DynamicArray<T> f_vec(count);
    DynamicArray<T> m(count);

    for (int index = 0; index < count; ++index) {
        lower_diag.set(index, T{0});
        main_diag.set(index, T{0});
        upper_diag.set(index, T{0});
        f_vec.set(index, T{0});
        m.set(index, T{0});
    }

    main_diag.set(0, T{1});
    main_diag.set(count - 1, T{1});

    for (int index = 1; index < count - 1; ++index) {
        lower_diag.set(index, h.get(index - 1));
        main_diag.set(index, T{2} * (h.get(index - 1) + h.get(index)));
        upper_diag.set(index, h.get(index));
        f_vec.set(index, T{6} * (delta.get(index) - delta.get(index - 1)));
    }

    solve_tridiag(lower_diag, main_diag, upper_diag, f_vec, m, count);

    MutableArraySequence<FunctionSegment<T>> *segments =
        new MutableArraySequence<FunctionSegment<T>>();

    for (int index = 0; index < count - 1; ++index) {
        DynamicArray<T> coefficients(4);
        coefficients.set(0, points.get(index).y);
        coefficients.set(1, delta.get(index) -
                                h.get(index) * (T{2} * m.get(index) + m.get(index + 1)) / T{6});
        coefficients.set(2, m.get(index) / T{2});
        coefficients.set(3, (m.get(index + 1) - m.get(index)) / (T{6} * h.get(index)));

        FunctionSegment<T> segment{points.get(index).x, points.get(index + 1).x, coefficients};
        segments->append(segment);
    }

    return segments;
}
