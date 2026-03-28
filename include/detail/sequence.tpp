#include "sequence.h"

#include <stdexcept>

template <class T>
Sequence<T> *Sequence<T>::slice(int index, int count,
                                const Sequence<T> *replace_seq) {
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
  auto *result = new MutableArraySequence<T>();

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
