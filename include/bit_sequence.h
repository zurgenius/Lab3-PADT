#pragma once

#include <stdexcept>

#include "array_sequence.h"
#include "bit.h"

class BitSequence : public Sequence<Bit> {
  private:
    DynamicArray<unsigned char> data;
    int bit_count;
    mutable Bit cached_bit;

    bool get_bit(int index) const;
    void set_bit(int index, bool value);

    static int check_bytes_needed(int n);

  public:
    BitSequence();
    BitSequence(const Bit *items, int count);
    BitSequence(const BitSequence &other);

    const Bit &get_first() const override;
    const Bit &get_last() const override;
    const Bit &get(int index) const override;
    const Bit &operator[](int index) const { return get(index); }

    Option<Bit> try_get_first() const override;
    Option<Bit> try_get_last() const override;
    Option<Bit> try_get(int index) const override;

    int get_count() const override;

    Sequence<Bit> *get_sub_sequence(int start, int end) override;

    Sequence<Bit> *append(const Bit &item) override;
    Sequence<Bit> *prepend(const Bit &item) override;
    Sequence<Bit> *insert_at(const Bit &item, int index) override;

    Sequence<Bit> *concat(const Sequence<Bit> *other) override;
    Sequence<Bit> *map(Bit (*func)(const Bit &elem)) override;
    Sequence<Bit> *where(bool (*predicate)(const Bit &elem)) override;
    Bit reduce(Bit (*func)(const Bit &first_elem, const Bit &second_elem),
               const Bit &initial_elem) override;
    Sequence<Bit> *slice(int index, int count, const Sequence<Bit> *replace_seq = nullptr) override;

    BitSequence *bit_and(const BitSequence &other) const;
    BitSequence *bit_or(const BitSequence &other) const;
    BitSequence *bit_xor(const BitSequence &other) const;
    BitSequence *bit_not() const;

    BitSequence *operator&(const BitSequence &other) const { return bit_and(other); }

    BitSequence *operator|(const BitSequence &other) const { return bit_or(other); }

    BitSequence *operator^(const BitSequence &other) const { return bit_xor(other); }

    BitSequence *operator~() const { return bit_not(); }

    template <class T> Sequence<T> *apply_mask(const Sequence<T> *source) const;

    class Enumerator : public IEnumerator<Bit> {
      private:
        const BitSequence *bit_seq;
        int index;
        mutable Bit current;

      public:
        explicit Enumerator(const BitSequence *bit_seq)
            : bit_seq(bit_seq), index(-1), current(false) {}

        bool move_next() override {
            index++;
            return index < bit_seq->bit_count;
        }

        const Bit &get_current() const override {
            current = Bit(bit_seq->get_bit(index));
            return current;
        }
    };

    IEnumerator<Bit> *get_enumerator() const override { return new Enumerator(this); }

    ~BitSequence() override {}
};

template <class T> Sequence<T> *BitSequence::apply_mask(const Sequence<T> *source) const {
    if (source == nullptr) {
        throw std::invalid_argument("Source sequence cannot be nullptr");
    }
    if (source->get_count() != bit_count) {
        throw std::invalid_argument("Mask length must match source length");
    }

    auto *filtered = new MutableArraySequence<T>();
    for (int index = 0; index < bit_count; index++) {
        if (get_bit(index)) {
            filtered->append(source->get(index));
        }
    }
    return filtered;
}
