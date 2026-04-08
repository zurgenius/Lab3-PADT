#pragma once

#include "ienumerator.h"

template <class T> class DynamicArray {
  private:
    T *data;
    int size;

  public:
    DynamicArray();
    explicit DynamicArray(int size);
    DynamicArray(const T *items, int count);
    DynamicArray(const DynamicArray<T> &other);

    const T &get(int index) const;
    const T &operator[](int index) const;

    int get_size() const;

    void set(int index, const T &value);
    void resize(int new_size);

    ~DynamicArray();

    class Enumerator : public IEnumerator<T> {
      private:
        const T *data;
        int size;
        int index;

      public:
        Enumerator(const T *data, int size) : data(data), size(size), index(-1) {}

        bool move_next() override {
            index++;
            return index < size;
        }

        const T &get_current() const override { return data[index]; }
    };

    IEnumerator<T> *get_enumerator() const { return new Enumerator(data, size); }
};

#include "detail/dynamic_array.tpp"
