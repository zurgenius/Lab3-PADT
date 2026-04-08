#pragma once

#include "ienumerator.h"
#include "option.h"

template <class T> class Sequence {
  public:
    virtual const T &get_first() const = 0;
    virtual const T &get_last() const = 0;
    virtual const T &get(int index) const = 0;
    const T &operator[](int index) const { return get(index); }

    virtual Option<T> try_get_first() const = 0;
    virtual Option<T> try_get_last() const = 0;
    virtual Option<T> try_get(int index) const = 0;

    virtual int get_count() const = 0;

    virtual Sequence<T> *get_sub_sequence(int start, int end) = 0;

    virtual Sequence<T> *append(const T &item) = 0;
    virtual Sequence<T> *prepend(const T &item) = 0;
    virtual Sequence<T> *insert_at(const T &item, int index) = 0;

    virtual Sequence<T> *concat(const Sequence<T> *other) = 0;
    virtual Sequence<T> *map(T (*func)(const T &elem)) = 0;
    virtual Sequence<T> *where(bool (*predicate)(const T &elem)) = 0;
    virtual T reduce(T (*func)(const T &first_elem, const T &second_elem),
                     const T &initial_elem) = 0;

    virtual Sequence<T> *slice(int index, int count, const Sequence<T> *replace_seq = nullptr) = 0;

    virtual IEnumerator<T> *get_enumerator() const = 0;

    virtual ~Sequence() {}
};
