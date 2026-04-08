#include "bit_sequence.h"

#include <stdexcept>

void zero_bytes(DynamicArray<unsigned char> &bytes) {
    for (int index = 0; index < bytes.get_size(); index++) {
        bytes.set(index, 0);
    }
}

void clear_unused_tail_bits(DynamicArray<unsigned char> &bytes, int bit_count) {
    if (bit_count == 0) {
        return;
    }

    const int used_bits = bit_count % 8;
    if (used_bits == 0) {
        return;
    }

    const int last_byte = bytes.get_size() - 1;
    const unsigned char mask = static_cast<unsigned char>((1u << used_bits) - 1u);
    bytes.set(last_byte, static_cast<unsigned char>(bytes.get(last_byte) & mask));
}

bool get_packed_bit(const DynamicArray<unsigned char> &bytes, int index) {
    const unsigned char byte = bytes.get(index / 8);
    return ((byte >> (index % 8)) & 1u) != 0;
}

int BitSequence::check_bytes_needed(int n) { return (n <= 0) ? 0 : (n + 7) / 8; }

bool BitSequence::get_bit(int index) const { return get_packed_bit(data, index); }

void BitSequence::set_bit(int index, bool value) {
    const int byte_index = index / 8;
    const int bit_index = index % 8;
    unsigned char byte = data.get(byte_index);
    const unsigned char mask = static_cast<unsigned char>(1u << bit_index);

    if (value) {
        byte = static_cast<unsigned char>(byte | mask);
    } else {
        byte = static_cast<unsigned char>(byte & static_cast<unsigned char>(~mask));
    }

    data.set(byte_index, byte);
}

BitSequence::BitSequence() : data(0), bit_count(0), cached_bit(false) {}

BitSequence::BitSequence(const Bit *items, int count)
    : data(check_bytes_needed(count)), bit_count(count), cached_bit(false) {
    if (count < 0) {
        throw std::invalid_argument("Count cannot be negative");
    }
    if (count > 0 && items == nullptr) {
        throw std::invalid_argument("Items cannot be nullptr");
    }

    zero_bytes(data);
    for (int index = 0; index < count; index++) {
        set_bit(index, items[index].get());
    }
}

BitSequence::BitSequence(const BitSequence &other)
    : data(other.data), bit_count(other.bit_count), cached_bit(false) {}

const Bit &BitSequence::get_first() const {
    if (bit_count == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    cached_bit = Bit(get_bit(0));
    return cached_bit;
}

const Bit &BitSequence::get_last() const {
    if (bit_count == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    cached_bit = Bit(get_bit(bit_count - 1));
    return cached_bit;
}

const Bit &BitSequence::get(int index) const {
    if (index < 0 || index >= bit_count) {
        throw std::out_of_range("Index out of range");
    }
    cached_bit = Bit(get_bit(index));
    return cached_bit;
}

Option<Bit> BitSequence::try_get_first() const {
    return (bit_count == 0) ? Option<Bit>::None() : Option<Bit>::Some(Bit(get_bit(0)));
}

Option<Bit> BitSequence::try_get_last() const {
    return (bit_count == 0) ? Option<Bit>::None() : Option<Bit>::Some(Bit(get_bit(bit_count - 1)));
}

Option<Bit> BitSequence::try_get(int index) const {
    return (index < 0 || index >= bit_count) ? Option<Bit>::None()
                                             : Option<Bit>::Some(Bit(get_bit(index)));
}

int BitSequence::get_count() const { return bit_count; }

Sequence<Bit> *BitSequence::get_sub_sequence(int start, int end) {
    if (start < 0 || end < 0 || start >= bit_count || end >= bit_count || start > end) {
        throw std::out_of_range("Index out of range");
    }

    const int new_count = end - start + 1;
    BitSequence *result = new BitSequence();
    result->data.resize(check_bytes_needed(new_count));
    result->bit_count = new_count;
    zero_bytes(result->data);

    for (int index = 0; index < new_count; index++) {
        result->set_bit(index, get_bit(start + index));
    }

    return result;
}

Sequence<Bit> *BitSequence::append(const Bit &item) {
    const int old_count = bit_count;
    const int new_count = old_count + 1;
    const int new_bytes = check_bytes_needed(new_count);

    if (new_bytes != data.get_size()) {
        DynamicArray<unsigned char> previous(data);
        data.resize(new_bytes);
        zero_bytes(data);
        for (int index = 0; index < old_count; index++) {
            set_bit(index, get_packed_bit(previous, index));
        }
    }

    bit_count = new_count;
    set_bit(new_count - 1, item.get());
    return this;
}

Sequence<Bit> *BitSequence::prepend(const Bit &item) {
    const int old_count = bit_count;
    const DynamicArray<unsigned char> previous(data);

    bit_count = old_count + 1;
    data.resize(check_bytes_needed(bit_count));
    zero_bytes(data);

    set_bit(0, item.get());
    for (int index = 0; index < old_count; index++) {
        set_bit(index + 1, get_packed_bit(previous, index));
    }

    return this;
}

Sequence<Bit> *BitSequence::insert_at(const Bit &item, int index) {
    if (index < 0 || index > bit_count) {
        throw std::out_of_range("Index out of range");
    }

    const int old_count = bit_count;
    const DynamicArray<unsigned char> previous(data);

    bit_count = old_count + 1;
    data.resize(check_bytes_needed(bit_count));
    zero_bytes(data);

    for (int current = 0; current < index; current++) {
        set_bit(current, get_packed_bit(previous, current));
    }
    set_bit(index, item.get());
    for (int current = index; current < old_count; current++) {
        set_bit(current + 1, get_packed_bit(previous, current));
    }

    return this;
}

Sequence<Bit> *BitSequence::concat(const Sequence<Bit> *other) {
    if (other == nullptr) {
        throw std::invalid_argument("Cannot concat with nullptr");
    }

    const int original_count = bit_count;
    const DynamicArray<unsigned char> previous(data);

    bit_count = original_count + other->get_count();
    data.resize(check_bytes_needed(bit_count));
    zero_bytes(data);

    for (int index = 0; index < original_count; index++) {
        set_bit(index, get_packed_bit(previous, index));
    }
    for (int index = 0; index < other->get_count(); index++) {
        set_bit(original_count + index, other->get(index).get());
    }

    return this;
}

Sequence<Bit> *BitSequence::map(Bit (*func)(const Bit &elem)) {
    BitSequence *result = new BitSequence();
    result->data.resize(data.get_size());
    result->bit_count = bit_count;
    zero_bytes(result->data);

    for (int index = 0; index < bit_count; index++) {
        result->set_bit(index, func(Bit(get_bit(index))).get());
    }

    return result;
}

Sequence<Bit> *BitSequence::where(bool (*predicate)(const Bit &elem)) {
    BitSequence *result = new BitSequence();
    for (int index = 0; index < bit_count; index++) {
        Bit value(get_bit(index));
        if (predicate(value)) {
            result->append(value);
        }
    }
    return result;
}

Bit BitSequence::reduce(Bit (*func)(const Bit &first_elem, const Bit &second_elem),
                        const Bit &initial_elem) {
    Bit accumulated = initial_elem;
    for (int index = 0; index < bit_count; index++) {
        accumulated = func(accumulated, Bit(get_bit(index)));
    }
    return accumulated;
}

Sequence<Bit> *BitSequence::slice(int index, int count, const Sequence<Bit> *replace_seq) {
    const int length = get_count();
    if (count < 0) {
        throw std::invalid_argument("Slice count cannot be negative");
    }
    if (length == 0) {
        throw std::out_of_range("Slice index out of range");
    }

    if (index < 0) {
        index += length;
    }
    if (index < 0 || index >= length) {
        throw std::out_of_range("Slice index out of range");
    }

    const int removed = (count > length - index) ? (length - index) : count;
    BitSequence *result = new BitSequence();

    for (int current = 0; current < index; current++) {
        result->append(get(current));
    }

    if (replace_seq != nullptr) {
        for (int current = 0; current < replace_seq->get_count(); current++) {
            result->append(replace_seq->get(current));
        }
    }

    for (int current = index + removed; current < length; current++) {
        result->append(get(current));
    }

    return result;
}

BitSequence *BitSequence::bit_and(const BitSequence &other) const {
    const int result_count = (bit_count < other.bit_count) ? bit_count : other.bit_count;
    BitSequence *result = new BitSequence();
    result->data.resize(check_bytes_needed(result_count));
    result->bit_count = result_count;
    zero_bytes(result->data);

    for (int index = 0; index < result_count; index++) {
        result->set_bit(index, get_bit(index) && other.get_bit(index));
    }

    clear_unused_tail_bits(result->data, result->bit_count);
    return result;
}

BitSequence *BitSequence::bit_or(const BitSequence &other) const {
    const int result_count = (bit_count > other.bit_count) ? bit_count : other.bit_count;
    BitSequence *result = new BitSequence();
    result->data.resize(check_bytes_needed(result_count));
    result->bit_count = result_count;
    zero_bytes(result->data);

    for (int index = 0; index < result_count; index++) {
        const bool left = index < bit_count ? get_bit(index) : false;
        const bool right = index < other.bit_count ? other.get_bit(index) : false;
        result->set_bit(index, left || right);
    }

    clear_unused_tail_bits(result->data, result->bit_count);
    return result;
}

BitSequence *BitSequence::bit_xor(const BitSequence &other) const {
    const int result_count = (bit_count > other.bit_count) ? bit_count : other.bit_count;
    BitSequence *result = new BitSequence();
    result->data.resize(check_bytes_needed(result_count));
    result->bit_count = result_count;
    zero_bytes(result->data);

    for (int index = 0; index < result_count; index++) {
        const bool left = index < bit_count ? get_bit(index) : false;
        const bool right = index < other.bit_count ? other.get_bit(index) : false;
        result->set_bit(index, left != right);
    }

    clear_unused_tail_bits(result->data, result->bit_count);
    return result;
}

BitSequence *BitSequence::bit_not() const {
    BitSequence *result = new BitSequence();
    result->data.resize(data.get_size());
    result->bit_count = bit_count;
    zero_bytes(result->data);

    for (int index = 0; index < bit_count; index++) {
        result->set_bit(index, !get_bit(index));
    }

    clear_unused_tail_bits(result->data, result->bit_count);
    return result;
}
