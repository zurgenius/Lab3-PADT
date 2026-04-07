#include "menu.h"

#include "algorithms.h"
#include "array_sequence.h"
#include "bit_sequence.h"
#include "list_sequence.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

const int kMaxSequences = 10;

Sequence<int> *sequences[kMaxSequences];
int sequence_count = 0;

BitSequence *bit_sequences[kMaxSequences];
int bit_sequence_count = 0;

int square(const int &value) { return value * value; }

bool is_positive(const int &value) { return value > 0; }

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

void print_sequence(const Sequence<int> *sequence) {
    std::cout << "[";
    for (int index = 0; index < sequence->get_count(); index++) {
        if (index != 0) {
            std::cout << ", ";
        }
        std::cout << sequence->get(index);
    }
    std::cout << "]" << std::endl;
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

bool store_sequence(Sequence<int> *sequence) {
    if (sequence_count >= kMaxSequences) {
        return false;
    }
    sequences[sequence_count++] = sequence;
    return true;
}

bool store_bit_sequence(BitSequence *sequence) {
    if (bit_sequence_count >= kMaxSequences) {
        return false;
    }
    bit_sequences[bit_sequence_count++] = sequence;
    return true;
}

void replace_if_new(Sequence<int> *&slot, Sequence<int> *result) {
    if (result != slot) {
        delete slot;
        slot = result;
    }
}

int select_sequence(const char *prompt) {
    if (sequence_count == 0) {
        std::cout << "No sequences created" << std::endl;
        return -1;
    }

    std::cout << prompt << std::endl;
    for (int index = 0; index < sequence_count; index++) {
        std::cout << index << ": ";
        print_sequence(sequences[index]);
    }

    std::cout << "Index: ";
    int index = -1;
    read_int(index);
    if (index < 0 || index >= sequence_count) {
        std::cout << "Invalid index" << std::endl;
        return -1;
    }
    return index;
}

int select_bit_sequence(const char *prompt) {
    if (bit_sequence_count == 0) {
        std::cout << "No BitSequences created" << std::endl;
        return -1;
    }

    std::cout << prompt << std::endl;
    for (int index = 0; index < bit_sequence_count; index++) {
        std::cout << index << ": ";
        print_bit_sequence(bit_sequences[index]);
    }

    std::cout << "Index: ";
    int index = -1;
    read_int(index);
    if (index < 0 || index >= bit_sequence_count) {
        std::cout << "Invalid index" << std::endl;
        return -1;
    }
    return index;
}

Sequence<int> *create_number_sequence(int kind) {
    switch (kind) {
    case 1:
        return new MutableArraySequence<int>();
    case 2:
        return new MutableListSequence<int>();
    case 3:
        return new ImmutableArraySequence<int>();
    case 4:
        return new ImmutableListSequence<int>();
    default:
        return nullptr;
    }
}

void menu_create_sequence() {
    std::cout << "\n=== Create Sequence ===" << std::endl;
    std::cout << "1. MutableArraySequence<int>" << std::endl;
    std::cout << "2. MutableListSequence<int>" << std::endl;
    std::cout << "3. ImmutableArraySequence<int>" << std::endl;
    std::cout << "4. ImmutableListSequence<int>" << std::endl;
    std::cout << "5. BitSequence" << std::endl;
    std::cout << "Choice: ";

    int choice = 0;
    read_int(choice);

    if (choice == 5) {
        std::cout << "Enter number of bits: ";
        int count = 0;
        read_int(count);

        BitSequence *sequence = new BitSequence();
        for (int index = 0; index < count; index++) {
            std::cout << "Bit " << index << " (0/1): ";
            int value = 0;
            read_int(value);
            sequence->append(Bit(value));
        }

        if (!store_bit_sequence(sequence)) {
            delete sequence;
            std::cout << "No free slots to store BitSequence" << std::endl;
            return;
        }
        std::cout << "Stored as bit sequence " << (bit_sequence_count - 1) << std::endl;
        return;
    }

    Sequence<int> *sequence = create_number_sequence(choice);
    if (sequence == nullptr) {
        std::cout << "Invalid choice" << std::endl;
        return;
    }

    std::cout << "Enter number of elements: ";
    int count = 0;
    read_int(count);

    for (int index = 0; index < count; index++) {
        std::cout << "Element " << index << ": ";
        int value = 0;
        read_int(value);
        Sequence<int> *result = sequence->append(value);
        replace_if_new(sequence, result);
    }

    if (!store_sequence(sequence)) {
        delete sequence;
        std::cout << "No free slots to store sequence" << std::endl;
        return;
    }
    std::cout << "Stored as sequence " << (sequence_count - 1) << std::endl;
}

void menu_add_element() {
    const int index = select_sequence("Select sequence:");
    if (index == -1) {
        return;
    }

    std::cout << "Enter element: ";
    int value = 0;
    read_int(value);

    Sequence<int> *result = sequences[index]->append(value);
    replace_if_new(sequences[index], result);
    std::cout << "Element appended" << std::endl;
}

void menu_print_all() {
    if (sequence_count == 0 && bit_sequence_count == 0) {
        std::cout << "No sequences created" << std::endl;
        return;
    }

    std::cout << "\n--- Sequence<int> ---" << std::endl;
    for (int index = 0; index < sequence_count; index++) {
        std::cout << "[" << index << "] ";
        print_sequence(sequences[index]);
    }

    std::cout << "\n--- BitSequence ---" << std::endl;
    for (int index = 0; index < bit_sequence_count; index++) {
        std::cout << "[" << index << "] ";
        print_bit_sequence(bit_sequences[index]);
    }
}

void menu_get_element() {
    const int sequence_index = select_sequence("Select sequence:");
    if (sequence_index == -1) {
        return;
    }

    std::cout << "Enter element index: ";
    int item_index = 0;
    read_int(item_index);

    try {
        std::cout << "Element: " << (*sequences[sequence_index])[item_index] << std::endl;
    } catch (const std::out_of_range &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void menu_get_subsequence() {
    const int index = select_sequence("Select sequence:");
    if (index == -1) {
        return;
    }

    std::cout << "Start index: ";
    int start = 0;
    read_int(start);
    std::cout << "End index: ";
    int end = 0;
    read_int(end);

    try {
        Sequence<int> *result = sequences[index]->get_sub_sequence(start, end);
        std::cout << "Result: ";
        print_sequence(result);

        if (!store_sequence(result)) {
            delete result;
            std::cout << "No free slots to store result" << std::endl;
            return;
        }
        std::cout << "Stored as sequence " << (sequence_count - 1) << std::endl;
    } catch (const std::out_of_range &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void menu_concat() {
    const int left = select_sequence("Select first sequence:");
    if (left == -1) {
        return;
    }
    const int right = select_sequence("Select second sequence:");
    if (right == -1) {
        return;
    }

    Sequence<int> *result = sequences[left]->concat(sequences[right]);
    std::cout << "Result: ";
    print_sequence(result);

    if (!store_sequence(result)) {
        delete result;
        std::cout << "No free slots to store result" << std::endl;
        return;
    }
    std::cout << "Stored as sequence " << (sequence_count - 1) << std::endl;
}

void menu_map() {
    const int index = select_sequence("Select sequence:");
    if (index == -1) {
        return;
    }

    Sequence<int> *result = sequences[index]->map(square);
    std::cout << "Mapped sequence: ";
    print_sequence(result);

    if (result == sequences[index]) {
        std::cout << "Sequence updated in place" << std::endl;
        return;
    }
    if (!store_sequence(result)) {
        delete result;
        std::cout << "No free slots to store result" << std::endl;
        return;
    }
    std::cout << "Stored as sequence " << (sequence_count - 1) << std::endl;
}

void menu_where() {
    const int index = select_sequence("Select sequence:");
    if (index == -1) {
        return;
    }

    Sequence<int> *result = sequences[index]->where(is_positive);
    std::cout << "Filtered sequence: ";
    print_sequence(result);

    if (result == sequences[index]) {
        std::cout << "Sequence updated in place" << std::endl;
        return;
    }
    if (!store_sequence(result)) {
        delete result;
        std::cout << "No free slots to store result" << std::endl;
        return;
    }
    std::cout << "Stored as sequence " << (sequence_count - 1) << std::endl;
}

void menu_reduce() {
    const int index = select_sequence("Select sequence:");
    if (index == -1) {
        return;
    }
    std::cout << "Reduce(sum): " << sequences[index]->reduce(sum, 0) << std::endl;
}

void menu_stats() {
    const int index = select_sequence("Select sequence:");
    if (index == -1) {
        return;
    }

    try {
        const Stats stats = min_max_avg(sequences[index]);
        std::cout << "min=" << stats.min << ", max=" << stats.max << ", avg=" << stats.avg
                  << std::endl;
    } catch (const std::invalid_argument &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void menu_bit_operations() {
    std::cout << "\n=== BitSequence Operations ===" << std::endl;
    std::cout << "1. AND" << std::endl;
    std::cout << "2. OR" << std::endl;
    std::cout << "3. XOR" << std::endl;
    std::cout << "4. NOT" << std::endl;
    std::cout << "Choice: ";

    int choice = 0;
    read_int(choice);

    if (choice == 4) {
        const int index = select_bit_sequence("Select BitSequence:");
        if (index == -1) {
            return;
        }

        BitSequence *result = ~(*bit_sequences[index]);
        std::cout << "Result: ";
        print_bit_sequence(result);

        if (!store_bit_sequence(result)) {
            delete result;
            std::cout << "No free slots to store result" << std::endl;
            return;
        }
        std::cout << "Stored as bit sequence " << (bit_sequence_count - 1) << std::endl;
        return;
    }

    const int left = select_bit_sequence("Select first BitSequence:");
    if (left == -1) {
        return;
    }
    const int right = select_bit_sequence("Select second BitSequence:");
    if (right == -1) {
        return;
    }

    BitSequence *result = nullptr;
    if (choice == 1) {
        result = *bit_sequences[left] & *bit_sequences[right];
    } else if (choice == 2) {
        result = *bit_sequences[left] | *bit_sequences[right];
    } else if (choice == 3) {
        result = *bit_sequences[left] ^ *bit_sequences[right];
    } else {
        std::cout << "Invalid choice" << std::endl;
        return;
    }

    std::cout << "Result: ";
    print_bit_sequence(result);

    if (!store_bit_sequence(result)) {
        delete result;
        std::cout << "No free slots to store result" << std::endl;
        return;
    }
    std::cout << "Stored as bit sequence " << (bit_sequence_count - 1) << std::endl;
}

void menu_split() {
    const int index = select_sequence("Select sequence to split by zero:");
    if (index == -1) {
        return;
    }

    Sequence<Sequence<int> *> *groups = split(sequences[index], is_zero);
    for (int group = 0; group < groups->get_count(); group++) {
        std::cout << "Group " << group << ": ";
        print_sequence(groups->get(group));
        delete groups->get(group);
    }
    delete groups;
}

void menu_slice() {
    const int index = select_sequence("Select sequence:");
    if (index == -1) {
        return;
    }

    std::cout << "Start index: ";
    int start = 0;
    read_int(start);
    std::cout << "Delete count: ";
    int count = 0;
    read_int(count);
    std::cout << "Replacement size: ";
    int replacement_size = 0;
    read_int(replacement_size);

    Sequence<int> *replacement = nullptr;
    if (replacement_size > 0) {
        replacement = new MutableArraySequence<int>();
        for (int item = 0; item < replacement_size; item++) {
            std::cout << "Replacement element " << item << ": ";
            int value = 0;
            read_int(value);
            replacement->append(value);
        }
    }

    try {
        Sequence<int> *result = sequences[index]->slice(start, count, replacement);
        std::cout << "Result: ";
        print_sequence(result);

        if (!store_sequence(result)) {
            delete result;
            std::cout << "No free slots to store result" << std::endl;
        } else {
            std::cout << "Stored as sequence " << (sequence_count - 1) << std::endl;
        }
    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    delete replacement;
}

void menu_apply_mask() {
    const int mask_index = select_bit_sequence("Select mask BitSequence:");
    if (mask_index == -1) {
        return;
    }
    const int sequence_index = select_sequence("Select sequence to filter:");
    if (sequence_index == -1) {
        return;
    }

    try {
        Sequence<int> *result = bit_sequences[mask_index]->apply_mask(sequences[sequence_index]);
        std::cout << "Result: ";
        print_sequence(result);

        if (!store_sequence(result)) {
            delete result;
            std::cout << "No free slots to store result" << std::endl;
            return;
        }
        std::cout << "Stored as sequence " << (sequence_count - 1) << std::endl;
    } catch (const std::invalid_argument &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void menu_run_tests() {
    std::cout << "\nRunning tests..." << std::endl;
    const int status = std::system("make tests && ./tests_runner");
    if (status != 0) {
        std::cout << "Test run failed" << std::endl;
    }
}

void destroy_all() {
    for (int index = 0; index < sequence_count; index++) {
        delete sequences[index];
        sequences[index] = nullptr;
    }
    for (int index = 0; index < bit_sequence_count; index++) {
        delete bit_sequences[index];
        bit_sequences[index] = nullptr;
    }
}

} // namespace

void run_menu() {
    int choice = -1;
    while (choice != 0) {
        std::cout << "\n==== Main Menu ====" << std::endl;
        std::cout << "1. Create sequence" << std::endl;
        std::cout << "2. Append element" << std::endl;
        std::cout << "3. Print all sequences" << std::endl;
        std::cout << "4. Get element" << std::endl;
        std::cout << "5. Get subsequence" << std::endl;
        std::cout << "6. Concat sequences" << std::endl;
        std::cout << "7. Map (square)" << std::endl;
        std::cout << "8. Where (positive)" << std::endl;
        std::cout << "9. Reduce (sum)" << std::endl;
        std::cout << "10. Bit operations" << std::endl;
        std::cout << "11. Split by zero" << std::endl;
        std::cout << "12. Slice" << std::endl;
        std::cout << "13. Stats" << std::endl;
        std::cout << "14. Apply mask" << std::endl;
        std::cout << "15. Run tests" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Choice: ";
        read_int(choice);

        switch (choice) {
        case 1:
            menu_create_sequence();
            break;
        case 2:
            menu_add_element();
            break;
        case 3:
            menu_print_all();
            break;
        case 4:
            menu_get_element();
            break;
        case 5:
            menu_get_subsequence();
            break;
        case 6:
            menu_concat();
            break;
        case 7:
            menu_map();
            break;
        case 8:
            menu_where();
            break;
        case 9:
            menu_reduce();
            break;
        case 10:
            menu_bit_operations();
            break;
        case 11:
            menu_split();
            break;
        case 12:
            menu_slice();
            break;
        case 13:
            menu_stats();
            break;
        case 14:
            menu_apply_mask();
            break;
        case 15:
            menu_run_tests();
            break;
        case 0:
            break;
        default:
            std::cout << "Invalid choice" << std::endl;
            break;
        }
    }

    destroy_all();
}
