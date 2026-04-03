#include "list_sequence.h"

#include <stdexcept>

template <class T> ListSequence<T>::ListSequence() : list() {}

template <class T>
ListSequence<T>::ListSequence(const T *items, int count) : list(items, count) {}

template <class T>
ListSequence<T>::ListSequence(const LinkedList<T> &other) : list(other) {}

template <class T>
ListSequence<T>::ListSequence(const ListSequence<T> &other)
    : list(other.list) {}

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
  return list.get(index);
}

template <class T> Option<T> ListSequence<T>::try_get_first() const {
  return list.get_length() == 0 ? Option<T>::None()
                                : Option<T>::Some(list.get_first());
}

template <class T> Option<T> ListSequence<T>::try_get_last() const {
  return list.get_length() == 0 ? Option<T>::None()
                                : Option<T>::Some(list.get_last());
}

template <class T> Option<T> ListSequence<T>::try_get(int index) const {
  return (index < 0 || index >= list.get_length())
             ? Option<T>::None()
             : Option<T>::Some(list.get(index));
}

template <class T> int ListSequence<T>::get_count() const {
  return list.get_length();
}

template <class T>
Sequence<T> *ListSequence<T>::get_sub_sequence(int start, int end) {
  if (start < 0 || end < 0 || start >= get_count() || end >= get_count() ||
      start > end) {
    throw std::out_of_range("Index out of range");
  }

  ListSequence<T> *result = EmptyClone();
  for (int index = start; index <= end; index++) {
    result->list.append(list.get(index));
  }
  return result;
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

template <class T>
Sequence<T> *ListSequence<T>::insert_at(const T &item, int index) {
  ListSequence<T> *target = Instance();
  target->list.insert_at(item, index);
  return target;
}

template <class T>
Sequence<T> *ListSequence<T>::concat(const Sequence<T> *other) {
  if (other == nullptr) {
    throw std::invalid_argument("Cannot concat with nullptr");
  }

  ListSequence<T> *result = EmptyClone();
  for (int index = 0; index < get_count(); index++) {
    result->list.append(list.get(index));
  }
  for (int index = 0; index < other->get_count(); index++) {
    result->list.append(other->get(index));
  }
  return result;
}

template <class T> Sequence<T> *ListSequence<T>::map(T (*func)(const T &elem)) {
  ListSequence<T> *result = EmptyClone();
  EnumeratorWrapper<T> it(get_enumerator());
  while (it.move_next()) {
    result->list.append(func(it.get_current()));
  }
  return result;
}

template <class T>
Sequence<T> *ListSequence<T>::where(bool (*predicate)(const T &elem)) {
  ListSequence<T> *result = EmptyClone();
  EnumeratorWrapper<T> it(get_enumerator());
  while (it.move_next()) {
    const T &value = it.get_current();
    if (predicate(value)) {
      result->list.append(value);
    }
  }
  return result;
}

template <class T>
T ListSequence<T>::reduce(T (*func)(const T &first_elem, const T &second_elem),
                          const T &initial_elem) {
  T accumulated = initial_elem;
  EnumeratorWrapper<T> it(get_enumerator());
  while (it.move_next()) {
    accumulated = func(accumulated, it.get_current());
  }
  return accumulated;
}
