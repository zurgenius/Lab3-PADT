#pragma once

#include "interpolator.h"

template <Field T>
Option<T> Interpolator<T>::evaluate(const Sequence<Function<T> *> &segments, const T &x) const {
    // бин поиск сегмента которому принадлежит x
    const int count = segments.get_count();
    if (count == 0) {
        return Option<T>::None();
    }

    const Function<T> *first = segments.get_first();
    const Function<T> *last = segments.get_last();
    if (first == nullptr || last == nullptr) {
        return Option<T>::None();
    }
    if (x < first->get_left() || x > last->get_right()) {
        return Option<T>::None();
    }

    int segment_index = -1;
    if (x == first->get_left()) {
        segment_index = 0;
    } else {
        int left = 0;
        int right = count - 1;
        while (left <= right) {
            const int mid = left + (right - left) / 2;
            const Function<T> *segment = segments.get(mid);
            if (segment == nullptr) {
                return Option<T>::None();
            }

            if (x <= segment->get_left()) {
                right = mid - 1;
            } else if (x > segment->get_right()) {
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

    
    const Function<T> *segment = segments.get(segment_index);
    if (segment == nullptr) {
        return Option<T>::None();
    }
    
    return segment->try_evaluate(x);
}
