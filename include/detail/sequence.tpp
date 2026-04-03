#include "sequence_base.h"

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
