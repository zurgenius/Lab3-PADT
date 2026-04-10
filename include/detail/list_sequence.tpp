#include "list_sequence.h"

#include <stdexcept>

template <class T> ListSequence<T>::ListSequence() : list() {}

template <class T> ListSequence<T>::ListSequence(const T *items, int count) : list(items, count) {}

template <class T> ListSequence<T>::ListSequence(const LinkedList<T> &other) : list(other) {}

template <class T> ListSequence<T>::ListSequence(const ListSequence<T> &other) : list(other.list) {}

template <class T> const T &ListSequence<T>::get_first() const {
    if (list.get_length() == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    return list.get_first();
}

template <class T> const T &ListSequence<T>::get_last() const {
    if (list.get_length() == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    return list.get_last();
}

template <class T> const T &ListSequence<T>::get(int index) const {
    if (index < 0 || index >= list.get_length()) {
        throw std::out_of_range("Index out of range");
    }

    IEnumerator<T> *iterator = list.get_enumerator();
    for (int current = 0; iterator->move_next(); current++) {
        if (current == index) {
            const T *value = &iterator->get_current();
            delete iterator;
            return *value;
        }
    }

    delete iterator;
    throw std::out_of_range("Index out of range");
}

template <class T> Option<T> ListSequence<T>::try_get_first() const {
    return list.get_length() == 0 ? Option<T>::None() : Option<T>::Some(list.get_first());
}

template <class T> Option<T> ListSequence<T>::try_get_last() const {
    return list.get_length() == 0 ? Option<T>::None() : Option<T>::Some(list.get_last());
}

template <class T> Option<T> ListSequence<T>::try_get(int index) const {
    if (index < 0 || index >= list.get_length()) {
        return Option<T>::None();
    }

    IEnumerator<T> *iterator = list.get_enumerator();
    for (int current = 0; iterator->move_next(); current++) {
        if (current == index) {
            Option<T> result = Option<T>::Some(iterator->get_current());
            delete iterator;
            return result;
        }
    }

    delete iterator;
    return Option<T>::None();
}

template <class T> int ListSequence<T>::get_count() const { return list.get_length(); }

template <class T> Sequence<T> *ListSequence<T>::get_sub_sequence(int start, int end) {
    if (start < 0 || end < 0 || start >= get_count() || end >= get_count() || start > end) {
        throw std::out_of_range("Index out of range");
    }

    ListSequence<T> *sub_list = EmptyClone();
    LinkedList<T> *sub_list_inside = list.get_sub_list(start, end);

    sub_list->list = *sub_list_inside;
    delete sub_list_inside;

    return sub_list;
}

template <class T> Sequence<T> *ListSequence<T>::append(const T &item) {
    ListSequence<T> *target = Instance();
    target->list.append(item);
    return target;
}

template <class T> Sequence<T> *ListSequence<T>::prepend(const T &item) {
    ListSequence<T> *target = Instance();
    target->list.prepend(item);
    return target;
}

template <class T> Sequence<T> *ListSequence<T>::insert_at(const T &item, int index) {
    ListSequence<T> *target = Instance();
    target->list.insert_at(item, index);
    return target;
}

template <class T> Sequence<T> *ListSequence<T>::concat(const Sequence<T> *other) {
    if (other == nullptr) {
        throw std::invalid_argument("Cannot concat with nullptr");
    }

    ListSequence<T> *concat_list = EmptyClone();

    IEnumerator<T> *this_iter = this->get_enumerator();
    while (this_iter->move_next()) {
        concat_list->list.append(this_iter->get_current());
    }
    delete this_iter;

    IEnumerator<T> *other_iter = other->get_enumerator();
    while (other_iter->move_next()) {
        concat_list->list.append(other_iter->get_current());
    }
    delete other_iter;

    return concat_list;
}

template <class T> Sequence<T> *ListSequence<T>::map(T (*func)(const T &elem)) {
    ListSequence<T> *result = EmptyClone();
    IEnumerator<T> *iterator = get_enumerator();
    while (iterator->move_next()) {
        result->list.append(func(iterator->get_current()));
    }
    delete iterator;
    return result;
}

template <class T> Sequence<T> *ListSequence<T>::where(bool (*predicate)(const T &elem)) {
    ListSequence<T> *result = EmptyClone();
    IEnumerator<T> *iterator = get_enumerator();
    while (iterator->move_next()) {
        const T &value = iterator->get_current();
        if (predicate(value)) {
            result->list.append(value);
        }
    }
    delete iterator;
    return result;
}

template <class T>
T ListSequence<T>::reduce(T (*func)(const T &first_elem, const T &second_elem),
                          const T &initial_elem) {
    T accumulated = initial_elem;
    IEnumerator<T> *iterator = get_enumerator();
    while (iterator->move_next()) {
        accumulated = func(accumulated, iterator->get_current());
    }
    delete iterator;
    return accumulated;
}

template <class T>
Sequence<T> *ListSequence<T>::slice(int index, int count, const Sequence<T> *replace_seq) {
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
    ListSequence<T> *result = EmptyClone();

    if (index > 0) {
        LinkedList<T> *prefix = list.get_sub_list(0, index - 1);
        result->list = *prefix;
        delete prefix;
    }

    if (replace_seq != nullptr) {
        for (int current = 0; current < replace_seq->get_count(); current++) {
            result->list.append(replace_seq->get(current));
        }
    }

    if (index + removed < length) {
        LinkedList<T> *suffix = list.get_sub_list(index + removed, length - 1);
        IEnumerator<T> *suffix_iterator = suffix->get_enumerator();
        while (suffix_iterator->move_next()) {
            result->list.append(suffix_iterator->get_current());
        }
        delete suffix_iterator;
        delete suffix;
    }

    return result;
}
