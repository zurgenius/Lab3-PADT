#include "array_sequence.h"

#include <stdexcept>

template <class T> ArraySequence<T>::ArraySequence() : array(0), count(0) {}

template <class T>
ArraySequence<T>::ArraySequence(const T *items, int count)
    : array(items, count), count(count) {}

template <class T>
ArraySequence<T>::ArraySequence(const DynamicArray<T> &other)
    : array(other), count(other.get_size()) {}

template <class T>
ArraySequence<T>::ArraySequence(const ArraySequence<T> &other)
    : array(other.array), count(other.count) {}

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
  return (index < 0 || index >= count) ? Option<T>::None()
                                       : Option<T>::Some(array.get(index));
}

template <class T> int ArraySequence<T>::get_count() const { return count; }

template <class T>
Sequence<T> *ArraySequence<T>::get_sub_sequence(int start, int end) {
  if (start < 0 || end < 0 || start >= count || end >= count || start > end) {
    throw std::out_of_range("Index out of range");
  }

  const int length = end - start + 1;
  ArraySequence<T> *result = EmptyClone();
  result->array = DynamicArray<T>(length);
  result->count = length;

  for (int offset = 0; offset < length; offset++) {
    result->array.set(offset, array.get(start + offset));
  }

  return result;
}

template <class T> Sequence<T> *ArraySequence<T>::append(const T &item) {
  ArraySequence<T> *target = Instance();
  DynamicArray<T> rebuilt(target->count + 1);

  for (int index = 0; index < target->count; index++) {
    rebuilt.set(index, target->array.get(index));
  }
  rebuilt.set(target->count, item);

  target->array = rebuilt;
  target->count++;
  return target;
}

template <class T> Sequence<T> *ArraySequence<T>::prepend(const T &item) {
  ArraySequence<T> *target = Instance();
  DynamicArray<T> rebuilt(target->count + 1);

  rebuilt.set(0, item);
  for (int index = 0; index < target->count; index++) {
    rebuilt.set(index + 1, target->array.get(index));
  }

  target->array = rebuilt;
  target->count++;
  return target;
}

template <class T>
Sequence<T> *ArraySequence<T>::insert_at(const T &item, int index) {
  ArraySequence<T> *target = Instance();
  if (index < 0 || index > target->count) {
    throw std::out_of_range("Index out of range");
  }

  DynamicArray<T> rebuilt(target->count + 1);
  for (int current = 0; current < index; current++) {
    rebuilt.set(current, target->array.get(current));
  }
  rebuilt.set(index, item);
  for (int current = index; current < target->count; current++) {
    rebuilt.set(current + 1, target->array.get(current));
  }

  target->array = rebuilt;
  target->count++;
  return target;
}

template <class T>
Sequence<T> *ArraySequence<T>::concat(const Sequence<T> *other) {
  if (other == nullptr) {
    throw std::invalid_argument("Cannot concat with nullptr");
  }

  ArraySequence<T> *result = EmptyClone();
  result->array = DynamicArray<T>(count + other->get_count());
  result->count = count + other->get_count();

  int write_index = 0;
  for (int index = 0; index < count; index++) {
    result->array.set(write_index++, array.get(index));
  }
  for (int index = 0; index < other->get_count(); index++) {
    result->array.set(write_index++, other->get(index));
  }

  return result;
}

template <class T>
Sequence<T> *ArraySequence<T>::map(T (*func)(const T &elem)) {
  ArraySequence<T> *result = EmptyClone();
  result->array = DynamicArray<T>(count);
  result->count = count;

  for (int index = 0; index < count; index++) {
    result->array.set(index, func(array.get(index)));
  }

  return result;
}

template <class T>
Sequence<T> *ArraySequence<T>::where(bool (*predicate)(const T &elem)) {
  int matches = 0;
  for (int index = 0; index < count; index++) {
    if (predicate(array.get(index))) {
      matches++;
    }
  }

  ArraySequence<T> *result = EmptyClone();
  result->array = DynamicArray<T>(matches);
  result->count = matches;

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
