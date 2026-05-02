#pragma once

#include <concepts>
#include <stdexcept>

#include "sequence.h"

// Концепт алгебраической структуры упорядоченного поля.
//С++20 чтобы удобно сузить тип T в шаблоне cubic_spline.
template <typename T>
concept Field = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
    { a - b } -> std::same_as<T>;
    { a * b } -> std::same_as<T>;
    { a / b } -> std::same_as<T>;
    { -a } -> std::same_as<T>;
    { a < b } -> std::convertible_to<bool>;
    T{0};
    T{1};
} && std::totally_ordered<T>;

// Кубический сплайн-интерполятор для упорядоченного поля T.
template <Field T> class cubic_spline : public Sequence<T> {
  public:
    cubic_spline();
    ~cubic_spline();

    // Построить сплайн по узлам (x строго возрастают, n >= 3).
    void build(const T *x, const T *f, int n);

    // Вычислить значение сплайна в точке x.
    T evaluate(const T &x) const;

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

    IEnumerator<T> *get_enumerator() const override;

  private:
    int nodes_count;    // количество узлов
    T *x_coordinates;     // копии x-координат
    T *function_values;     // копии значений функции
    T **piecewise_given_func_coefs; // матрица коэффициентов: 4 строки (a, b, c, d)

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

    void take_raw(T *x, T *f, int n);

    // Решение трёхдиагональной системы методом прогонки (Томаса).
    void solve_tridiag(T **TDM, T *F, T *b);
};

#include "detail/cubic_spline.tpp"