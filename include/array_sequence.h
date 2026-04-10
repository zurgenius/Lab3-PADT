#pragma once

#include "dynamic_array.h"
#include "sequence.h"

template <class T> class ArraySequence : public Sequence<T> {
  protected:
    DynamicArray<T> array;
    int count;
    int capacity;
    virtual ArraySequence<T> *Instance() = 0;
    virtual ArraySequence<T> *EmptyClone() = 0;
    void reallocate_if_needed(int required);

  public:
    ArraySequence();
    ArraySequence(const T *items, int count);
    ArraySequence(const DynamicArray<T> &other);
    ArraySequence(const ArraySequence<T> &other);

    const T &get_first() const override;
    const T &get_last() const override;
    const T &get(int index) const override;

    Option<T> try_get_first() const override;
    Option<T> try_get_last() const override;
    Option<T> try_get(int index) const override;

    int get_count() const override;

    Sequence<T> *get_sub_sequence(int start, int end) override;

    Sequence<T> *append(const T &item) override;
    Sequence<T> *prepend(const T &item) override;
    Sequence<T> *insert_at(const T &item, int index) override;

    Sequence<T> *concat(const Sequence<T> *other) override;
    Sequence<T> *map(T (*func)(const T &elem)) override;
    Sequence<T> *where(bool (*predicate)(const T &elem)) override;
    T reduce(T (*func)(const T &first_elem, const T &second_elem), const T &initial_elem) override;
    Sequence<T> *slice(int index, int count, const Sequence<T> *replace_seq = nullptr) override;

    class Enumerator : public IEnumerator<T> {
      private:
        const DynamicArray<T> &array;
        int count;
        int index;

      public:
        Enumerator(const DynamicArray<T> &array, int count)
            : array(array), count(count), index(-1) {}

        bool move_next() override {
            index++;
            return index < count;
        }

        const T &get_current() const override { return array.get(index); }
    };

    IEnumerator<T> *get_enumerator() const override { return new Enumerator(array, count); }

    ~ArraySequence() override {}
};

template <class T> class MutableArraySequence : public ArraySequence<T> {
  protected:
    ArraySequence<T> *Instance() override { return this; }

    ArraySequence<T> *EmptyClone() override { return new MutableArraySequence<T>(); }

  public:
    MutableArraySequence() : ArraySequence<T>() {}
    MutableArraySequence(const T *items, int count) : ArraySequence<T>(items, count) {}
    MutableArraySequence(const DynamicArray<T> &other) : ArraySequence<T>(other) {}
    MutableArraySequence(const ArraySequence<T> &other) : ArraySequence<T>(other) {}
};

template <class T> class ImmutableArraySequence : public ArraySequence<T> {
  protected:
    ArraySequence<T> *Instance() override { return new ImmutableArraySequence<T>(*this); }

    ArraySequence<T> *EmptyClone() override { return new ImmutableArraySequence<T>(); }

  public:
    ImmutableArraySequence() : ArraySequence<T>() {}
    ImmutableArraySequence(const T *items, int count) : ArraySequence<T>(items, count) {}
    ImmutableArraySequence(const DynamicArray<T> &other) : ArraySequence<T>(other) {}
    ImmutableArraySequence(const ArraySequence<T> &other) : ArraySequence<T>(other) {}
};

#include "detail/array_sequence.tpp"
