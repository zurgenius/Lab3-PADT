#pragma once

#include "interpolator.h"

template <Field T>
Option<T> Interpolator<T>::evaluate(const Sequence<FunctionSegment<T>> &segments,
                                    const T &x) const {
    // бин поиск сегмента которому принадлежит x
    const int count = segments.get_count();
    if (count == 0) {
        return Option<T>::None();
    }

    const FunctionSegment<T> &first = segments.get_first();
    const FunctionSegment<T> &last = segments.get_last();
    if (x < first.left || x > last.right) {
        return Option<T>::None();
    }

    int segment_index = -1;
    if (x == first.left) {
        segment_index = 0;
    } else {
        int left = 0;
        int right = count - 1;
        while (left <= right) {
            const int mid = left + (right - left) / 2;
            const FunctionSegment<T> &segment = segments.get(mid);

            if (x <= segment.left) {
                right = mid - 1;
            } else if (x > segment.right) {
                left = mid + 1;
            } else {
                segment_index = mid;
                break;
            }
        }
    }

    if (segment_index < 0) {
        return Option<T>::None();
    }

    // считаем значени полинома для этого x в выбранном сегменте
    // по схеме Горнера
    const FunctionSegment<T> &segment = segments.get(segment_index);
    const int coefficient_count = segment.coefficients.get_size();
    if (coefficient_count == 0) {
        return Option<T>::None();
    }

    const T dx = x - segment.left;
    T value = segment.coefficients.get(coefficient_count - 1);
    for (int index = coefficient_count - 2; index >= 0; --index) {
        value = value * dx + segment.coefficients.get(index);
    }

    return Option<T>::Some(value);
}
