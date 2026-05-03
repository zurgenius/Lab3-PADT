#pragma once

#include <concepts>
#include <stdexcept>

#include "sequence.h"

// Концепт алгебраической структуры упорядоченного поля.
//С++20 чтобы удобно сузить тип T в шаблоне CubicSpline.
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

// Точка (x, f(x)) для сплайн-узлов.
template <Field T> struct Point {
    T x;
    T y;
};

// Кубический сплайн-интерполятор для упорядоченного поля T.
template <Field T> class CubicSpline : public Sequence<Point<T>> {
  public:
    CubicSpline();
    ~CubicSpline();

  
    // Построить сплайн по набору точек (x строго возрастают, n >= 3).
    void build(const Point<T> *points, int n);

    // Вычислить значение сплайна в точке x.
    T evaluate(const T &x) const;

    const Point<T> &get_first() const override;
    const Point<T> &get_last() const override;
    const Point<T> &get(int index) const override;

    Option<Point<T>> try_get_first() const override;
    Option<Point<T>> try_get_last() const override;
    Option<Point<T>> try_get(int index) const override;

    int get_count() const override;

    Sequence<Point<T>> *get_sub_sequence(int start, int end) override;

    Sequence<Point<T>> *append(const Point<T> &item) override;
    Sequence<Point<T>> *prepend(const Point<T> &item) override;
    Sequence<Point<T>> *insert_at(const Point<T> &item, int index) override;

    Sequence<Point<T>> *concat(const Sequence<Point<T>> *other) override;
    Sequence<Point<T>> *map(Point<T> (*func)(const Point<T> &elem)) override;
    Sequence<Point<T>> *where(bool (*predicate)(const Point<T> &elem)) override;
    Point<T> reduce(Point<T> (*func)(const Point<T> &first_elem, const Point<T> &second_elem),
            const Point<T> &initial_elem) override;

    Sequence<Point<T>> *slice(int index, int count,
                  const Sequence<Point<T>> *replace_seq = nullptr) override;

    IEnumerator<Point<T>> *get_enumerator() const override;

  private:
    int nodes_count;    // количество узлов
    Point<T> *nodes;    // копии узлов (x, f(x))
    T **piecewise_given_func_coefs; // матрица коэффициентов: 4 строки (a, b, c, d)

    class Enumerator : public IEnumerator<Point<T>> {
      private:
        const Point<T> *data;
        int size;
        int index;

      public:
        Enumerator(const Point<T> *data, int size) : data(data), size(size), index(-1) {}

        bool move_next() override {
            index++;
            return index < size;
        }

        const Point<T> &get_current() const override { return data[index]; }
    };

      void take_raw(Point<T> *points, int n);

    // Решение трёхдиагональной системы методом прогонки (Томаса).
    void solve_tridiag(T **TDM, T *F, T *b);
};

#include "detail/cubic_spline.tpp"