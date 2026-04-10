#pragma once

#include "linked_list.h"
#include "sequence.h"

template <class T> class ListSequence : public Sequence<T> {
  protected:
    LinkedList<T> list;
    virtual ListSequence<T> *Instance() = 0;
    virtual ListSequence<T> *EmptyClone() = 0;

  public:
    ListSequence();
    ListSequence(const T *items, int count);
    ListSequence(const LinkedList<T> &other);
    ListSequence(const ListSequence<T> &other);

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

    IEnumerator<T> *get_enumerator() const override { return list.get_enumerator(); }

    ~ListSequence() override {}
};

template <class T> class MutableListSequence : public ListSequence<T> {
  protected:
    ListSequence<T> *Instance() override { return this; }

    ListSequence<T> *EmptyClone() override { return new MutableListSequence<T>(); }

  public:
    MutableListSequence() : ListSequence<T>() {}
    MutableListSequence(const T *items, int count) : ListSequence<T>(items, count) {}
    MutableListSequence(const LinkedList<T> &other) : ListSequence<T>(other) {}
    MutableListSequence(const ListSequence<T> &other) : ListSequence<T>(other) {}
};

template <class T> class ImmutableListSequence : public ListSequence<T> {
  protected:
    ListSequence<T> *Instance() override { return new ImmutableListSequence<T>(*this); }

    ListSequence<T> *EmptyClone() override { return new ImmutableListSequence<T>(); }

  public:
    ImmutableListSequence() : ListSequence<T>() {}
    ImmutableListSequence(const T *items, int count) : ListSequence<T>(items, count) {}
    ImmutableListSequence(const LinkedList<T> &other) : ListSequence<T>(other) {}
    ImmutableListSequence(const ListSequence<T> &other) : ListSequence<T>(other) {}
};

#include "detail/list_sequence.tpp"
