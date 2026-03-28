#pragma once

#include <stdexcept>

#include "sequence.h"

template <class T>
Sequence<Sequence<T> *> *split(const Sequence<T> *seq, bool (*predicate)(const T &)) {
    auto *sequences = new MutableArraySequence<Sequence<T> *>();
    auto *current_seq = new MutableArraySequence<T>();

    for (int i = 0; i < seq->get_count(); i++) {
        if (predicate(seq->get(i))) {
            sequences->append(current_seq);
            current_seq = new MutableArraySequence<T>();
        } else {
            current_seq->append(seq->get(i));
        }
    }

    sequences->append(current_seq);

    return sequences;
}

struct Stats {
    int min;
    int max;
    double avg;
};

inline Stats min_max_avg(const Sequence<int> *seq) {
    if (seq == nullptr) {
        throw std::invalid_argument("Sequence cannot be nullptr");
    }
    if (seq->get_count() == 0) {
        throw std::invalid_argument("Sequence cannot be empty");
    }

    int min_value = seq->get(0);
    int max_value = seq->get(0);
    long long sum = 0;

    for (int index = 0; index < seq->get_count(); index++) {
        const int value = seq->get(index);
        if (value < min_value) {
            min_value = value;
        }
        if (value > max_value) {
            max_value = value;
        }
        sum += value;
    }

    Stats stats = {min_value, max_value, static_cast<double>(sum) / seq->get_count()};
    return stats;
}
