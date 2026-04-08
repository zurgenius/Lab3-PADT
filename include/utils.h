#pragma once

#include "bit.h"
#include "bit_sequence.h"
#include "sequence.h"

#include <iostream>

namespace utils {

int square(const int &value);
bool is_positive(const int &value);
bool is_even(const int &value);
int sum(const int &left, const int &right);
bool is_zero(const int &value);
bool is_zero_bit(const Bit &bit);
void read_int(int &value);

template <class T> void print_sequence(const Sequence<T> *sequence) {
    std::cout << "[";
    for (int index = 0; index < sequence->get_count(); index++) {
        if (index != 0) {
            std::cout << ", ";
        }
        std::cout << sequence->get(index);
    }
    std::cout << "]" << std::endl;
}

void print_bit_sequence(const BitSequence *sequence);

} // namespace utils
