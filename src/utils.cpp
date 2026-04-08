#include "utils.h"

#include <string>

namespace utils {

int square(const int &value) { return value * value; }

bool is_positive(const int &value) { return value > 0; }

bool is_even(const int &value) { return value % 2 == 0; }

int sum(const int &left, const int &right) { return left + right; }

bool is_zero(const int &value) { return value == 0; }

bool is_zero_bit(const Bit &bit) { return !bit.get(); }

void read_int(int &value) {
    std::string line;
    while (true) {
        std::getline(std::cin, line);
        try {
            std::size_t parsed = 0;
            value = std::stoi(line, &parsed);
            if (parsed == line.size()) {
                return;
            }
        } catch (...) {
        }
        std::cout << "Incorrect input. Try again: ";
    }
}

void print_bit_sequence(const BitSequence *sequence) {
    std::cout << "[";
    for (int index = 0; index < sequence->get_count(); index++) {
        if (index != 0) {
            std::cout << ", ";
        }
        std::cout << sequence->get(index).get();
    }
    std::cout << "]" << std::endl;
}

} // namespace utils
