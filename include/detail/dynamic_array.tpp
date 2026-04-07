#include "dynamic_array.h"

#include <stdexcept>

template <class T> DynamicArray<T>::DynamicArray() : data(nullptr), size(0) {}

template <class T> DynamicArray<T>::DynamicArray(int size) : data(nullptr), size(size) {
    if (size < 0) {
        throw std::invalid_argument("Size cannot be negative");
    }
    if (size > 0) {
        data = new T[size];
    }
}

template <class T>
DynamicArray<T>::DynamicArray(const T *items, int count) : data(nullptr), size(count) {
    if (count < 0) {
        throw std::invalid_argument("Count cannot be negative");
    }
    if (count > 0 && items == nullptr) {
        throw std::invalid_argument("Items cannot be nullptr");
    }

    if (size > 0) {
        data = new T[size];
        for (int index = 0; index < size; index++) {
            data[index] = items[index];
        }
    }
}

template <class T>
DynamicArray<T>::DynamicArray(const DynamicArray<T> &other) : data(nullptr), size(other.size) {
    if (size > 0) {
        data = new T[size];
        for (int index = 0; index < size; index++) {
            data[index] = other.data[index];
        }
    }
}

template <class T> const T &DynamicArray<T>::get(int index) const {
    if (index < 0 || index >= size) {
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}

template <class T> const T &DynamicArray<T>::operator[](int index) const { return get(index); }

template <class T> int DynamicArray<T>::get_size() const { return size; }

template <class T> void DynamicArray<T>::set(int index, const T &value) {
    if (index < 0 || index >= size) {
        throw std::out_of_range("Index out of range");
    }
    data[index] = value;
}

template <class T> void DynamicArray<T>::resize(int new_size) {
    if (new_size < 0) {
        throw std::invalid_argument("New size cannot be negative");
    }

    T *replacement = nullptr;
    if (new_size > 0) {
        replacement = new T[new_size];
        const int copied = (new_size < size) ? new_size : size;
        for (int index = 0; index < copied; index++) {
            replacement[index] = data[index];
        }
    }

    delete[] data;
    data = replacement;
    size = new_size;
}

template <class T> DynamicArray<T>::~DynamicArray() { delete[] data; }
