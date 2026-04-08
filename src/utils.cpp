#include "utils.h"

#include <string>

namespace utils {

int square(const int &value) { return value * value; }

bool is_positive(const int &value) { return value > 0; }

bool is_even(const int &value) { return value % 2 == 0; }

int sum(const int &left, const int &right) { return left + right; }

bool is_zero(const int &value) { return value == 0; }

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

} // namespace utils
