#include "array_sequence.h"

#include <stdexcept>

template <class T> void ArraySequence<T>::reallocate_if_needed(int required) {
    if (required <= capacity) {
        return;
    }

    int new_capacity = (capacity == 0) ? 1 : capacity;
    while (new_capacity < required) {
        new_capacity *= 2;
    }

    array.resize(new_capacity);
    capacity = new_capacity;
}

template <class T> ArraySequence<T>::ArraySequence() : array(0), count(0), capacity(0) {}

template <class T>
ArraySequence<T>::ArraySequence(const T *items, int count)
    : array(items, count), count(count), capacity(count) {}

template <class T>
ArraySequence<T>::ArraySequence(const DynamicArray<T> &other)
    : array(other), count(other.get_size()), capacity(other.get_size()) {}

template <class T>
ArraySequence<T>::ArraySequence(const ArraySequence<T> &other)
    : array(other.array), count(other.count), capacity(other.capacity) {}

template <class T> const T &ArraySequence<T>::get_first() const {
    if (count == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    return array.get(0);
}

template <class T> const T &ArraySequence<T>::get_last() const {
    if (count == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    return array.get(count - 1);
}

template <class T> const T &ArraySequence<T>::get(int index) const {
    if (index < 0 || index >= count) {
        throw std::out_of_range("Index out of range");
    }
    return array.get(index);
}

template <class T> Option<T> ArraySequence<T>::try_get_first() const {
    return count == 0 ? Option<T>::None() : Option<T>::Some(array.get(0));
}

template <class T> Option<T> ArraySequence<T>::try_get_last() const {
    return count == 0 ? Option<T>::None() : Option<T>::Some(array.get(count - 1));
}

template <class T> Option<T> ArraySequence<T>::try_get(int index) const {
    return (index < 0 || index >= count) ? Option<T>::None() : Option<T>::Some(array.get(index));
}

template <class T> int ArraySequence<T>::get_count() const { return count; }

template <class T> Sequence<T> *ArraySequence<T>::get_sub_sequence(int start, int end) {
    if (start < 0 || end < 0 || start >= count || end >= count || start > end) {
        throw std::out_of_range("Index out of range");
    }

    const int length = end - start + 1;
    ArraySequence<T> *result = EmptyClone();
    result->array.resize(length);
    result->count = length;
    result->capacity = length;

    for (int offset = 0; offset < length; offset++) {
        result->array.set(offset, array.get(start + offset));
    }

    return result;
}

template <class T> Sequence<T> *ArraySequence<T>::append(const T &item) {
    ArraySequence<T> *target = Instance();
    const int old_count = target->count;

    target->reallocate_if_needed(old_count + 1);
    target->array.set(old_count, item);
    target->count = old_count + 1;
    return target;
}

template <class T> Sequence<T> *ArraySequence<T>::prepend(const T &item) {
    ArraySequence<T> *target = Instance();
    const int old_count = target->count;

    target->reallocate_if_needed(old_count + 1);
    for (int index = old_count; index > 0; index--) {
        target->array.set(index, target->array.get(index - 1));
    }
    target->array.set(0, item);
    target->count = old_count + 1;
    return target;
}

template <class T> Sequence<T> *ArraySequence<T>::insert_at(const T &item, int index) {
    ArraySequence<T> *target = Instance();
    if (index < 0 || index > target->count) {
        throw std::out_of_range("Index out of range");
    }

    const int old_count = target->count;
    target->reallocate_if_needed(old_count + 1);
    for (int current = old_count; current > index; current--) {
        target->array.set(current, target->array.get(current - 1));
    }
    target->array.set(index, item);
    target->count = old_count + 1;
    return target;
}

template <class T> Sequence<T> *ArraySequence<T>::concat(const Sequence<T> *other) {
    if (other == nullptr) {
        throw std::invalid_argument("Cannot concat with nullptr");
    }

    ArraySequence<T> *result = EmptyClone();
    result->count = count + other->get_count();
    result->array.resize(result->count);
    result->capacity = result->count;

    int write_index = 0;
    for (int index = 0; index < count; index++) {
        result->array.set(write_index++, array.get(index));
    }
    for (int index = 0; index < other->get_count(); index++) {
        result->array.set(write_index++, other->get(index));
    }

    return result;
}

template <class T> Sequence<T> *ArraySequence<T>::map(T (*func)(const T &elem)) {
    ArraySequence<T> *result = EmptyClone();
    result->count = count;
    result->array.resize(count);
    result->capacity = count;

    for (int index = 0; index < count; index++) {
        result->array.set(index, func(array.get(index)));
    }

    return result;
}

template <class T> Sequence<T> *ArraySequence<T>::where(bool (*predicate)(const T &elem)) {
    int matches = 0;
    for (int index = 0; index < count; index++) {
        if (predicate(array.get(index))) {
            matches++;
        }
    }

    ArraySequence<T> *result = EmptyClone();
    result->count = matches;
    result->array.resize(matches);
    result->capacity = matches;

    int write_index = 0;
    for (int index = 0; index < count; index++) {
        const T &value = array.get(index);
        if (predicate(value)) {
            result->array.set(write_index++, value);
        }
    }

    return result;
}

template <class T>
T ArraySequence<T>::reduce(T (*func)(const T &first_elem, const T &second_elem),
                           const T &initial_elem) {
    T accumulated = initial_elem;
    for (int index = 0; index < count; index++) {
        accumulated = func(accumulated, array.get(index));
    }
    return accumulated;
}

template <class T>
Sequence<T> *ArraySequence<T>::slice(int index, int count, const Sequence<T> *replace_seq) {
    const int length = this->count;
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
    const int replacement_count = (replace_seq == nullptr) ? 0 : replace_seq->get_count();
    const int result_count = index + replacement_count + (length - index - removed);

    ArraySequence<T> *result = EmptyClone();
    result->array.resize(result_count);
    result->count = result_count;
    result->capacity = result_count;

    int write_index = 0;
    for (int current = 0; current < index; current++) {
        result->array.set(write_index++, array.get(current));
    }

    if (replace_seq != nullptr) {
        for (int current = 0; current < replacement_count; current++) {
            result->array.set(write_index++, replace_seq->get(current));
        }
    }

    for (int current = index + removed; current < length; current++) {
        result->array.set(write_index++, array.get(current));
    }

    return result;
}
