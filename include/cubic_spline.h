#pragma once

#include <concepts>
#include <stdexcept>

#include "dynamic_array.h"
#include "sequence.h"

// Концепт алгебраической структуры упорядоченного поля.
// С++20 чтобы удобно сузить тип T в шаблоне CubicSpline.
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

template <Field T> struct SplineSegment {
    T left_x;
    T right_x;
    T a;
    T b;
    T c;
    T d;
};

// Кубический сплайн-интерполятор для упорядоченного поля T.
template <Field T> class CubicSpline : public Sequence<Point<T>> {
  protected:
    virtual CubicSpline<T> *Instance() = 0;
    virtual CubicSpline<T> *EmptyClone() = 0;

  public:
    CubicSpline();
    CubicSpline(const CubicSpline<T> &other);
    ~CubicSpline();

    // Построить сплайн по набору точек (x строго возрастают, n >= 3).
    void build(const Point<T> *points, int n);

    // Вычислить значение сплайна в точке x.
    T evaluate(const T &x) const;

    int get_segment_count() const;
    SplineSegment<T> get_segment(int index) const;
    Option<SplineSegment<T>> try_get_segment(int index) const;

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
    int nodes_count; // количество узлов
    DynamicArray<Point<T>> nodes;
    DynamicArray<T> coef_a;
    DynamicArray<T> coef_b;
    DynamicArray<T> coef_c;
    DynamicArray<T> coef_d;

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

    void take_raw(const Point<T> *points, int n);

    // Решение трёхдиагональной системы методом прогонки (Томаса).
    void solve_tridiag(T **TDM, T *F, T *b);
};

template <Field T> class MutableCubicSpline : public CubicSpline<T> {
  protected:
    CubicSpline<T> *Instance() override { return this; }

    CubicSpline<T> *EmptyClone() override { return new MutableCubicSpline<T>(); }

  public:
    MutableCubicSpline() : CubicSpline<T>() {}
    MutableCubicSpline(const CubicSpline<T> &other) : CubicSpline<T>(other) {}
};

template <Field T> class ImmutableCubicSpline : public CubicSpline<T> {
  protected:
    CubicSpline<T> *Instance() override { return new ImmutableCubicSpline<T>(*this); }

    CubicSpline<T> *EmptyClone() override { return new ImmutableCubicSpline<T>(); }

  public:
    ImmutableCubicSpline() : CubicSpline<T>() {}
    ImmutableCubicSpline(const CubicSpline<T> &other) : CubicSpline<T>(other) {}
};

#include "detail/cubic_spline.tpp"
