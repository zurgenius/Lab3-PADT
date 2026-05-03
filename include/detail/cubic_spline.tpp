#pragma once

#include "cubic_spline.h"
#include <stdexcept>

// ===== Конструкторы и деструктор =====
template <Field T>
CubicSpline<T>::CubicSpline() : ArraySequence<Point<T>>(), coef_a(), coef_b(), coef_c(), coef_d() {}

template <Field T>
CubicSpline<T>::CubicSpline(const CubicSpline<T> &other)
    : ArraySequence<Point<T>>(other), coef_a(other.coef_a), coef_b(other.coef_b),
      coef_c(other.coef_c), coef_d(other.coef_d) {}

template <Field T> CubicSpline<T>::~CubicSpline() {}

// === Ключевые методы ===
template <Field T> void CubicSpline<T>::assign_nodes(const DynamicArray<Point<T>> &points) {
    this->count = points.get_size();
    this->array.resize(this->count);
    this->capacity = this->count;
    for (int i = 0; i < this->count; ++i) {
        this->array.set(i, points.get(i));
    }
}

template <Field T> void CubicSpline<T>::clear_coefficients() {
    coef_a.resize(0);
    coef_b.resize(0);
    coef_c.resize(0);
    coef_d.resize(0);
}

// обертка, как работаем с последовательностью
// при кол-ве узлов >= 3 - строим сплайн, чтобы не терять состояние собранности при переделке
// последовательности иначе просто храним узлы без построения
template <Field T> void CubicSpline<T>::rebuild_or_store_raw() {
    if (this->count >= 3) {
        build(&this->array.get(0), this->count);
    } else {
        store_raw_points();
    }
}

template <Field T> void CubicSpline<T>::build(const Point<T> *points, int n) {
    if (n < 3) {
        throw std::invalid_argument("CubicSpline::build: at least 3 points required");
    }

    DynamicArray<Point<T>> input_points(points, n);
    assign_nodes(input_points);
    clear_coefficients();

    // вычисление h_i и delta_i
    DynamicArray<T> delta(n - 1);
    DynamicArray<T> h(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        const T segment_width = this->array.get(i + 1).x - this->array.get(i).x;
        h.set(i, segment_width);
        delta.set(i, (this->array.get(i + 1).y - this->array.get(i).y) / segment_width);
    }

    // построение трёхдиагональной системы Am = F
    //  здесь неизвестные - вторые производные в узлах
    DynamicArray<T> lower_diag(n);
    DynamicArray<T> main_diag(n);
    DynamicArray<T> upper_diag(n);
    DynamicArray<T> f_vec(n);
    DynamicArray<T> m(n);

    for (int i = 0; i < n; ++i) {
        lower_diag.set(i, T{0});
        main_diag.set(i, T{0});
        upper_diag.set(i, T{0});
        f_vec.set(i, T{0});
        m.set(i, T{0});
    }

    main_diag.set(0, T{1});
    main_diag.set(n - 1, T{1});

    // в этом цикле строим уравнение для внутренних узлов; для крайних задаём m[0] = m[n-1] = 0
    // (естественный сплайн)
    // такое уравнение гарантирует непрерывность первой производной в узлах
    for (int i = 1; i < n - 1; ++i) {
        lower_diag.set(i, h.get(i - 1));
        main_diag.set(i, T{2} * (h.get(i - 1) + h.get(i)));
        upper_diag.set(i, h.get(i));
        f_vec.set(i, T{6} * (delta.get(i) - delta.get(i - 1)));
    }

    // решение системы методом прогонки (Томаса)
    solve_tridiag(lower_diag, main_diag, upper_diag, f_vec, m);

    // вычисление коэффициентов сплайна
    coef_a.resize(n - 1);
    coef_b.resize(n - 1);
    coef_c.resize(n - 1);
    coef_d.resize(n - 1);

    // храним матрицу коээфициентов 4*n-1, где n-1 - кол-во кусков в сплайне
    //  по этой матрице можем однозначно задать кусочно-заданную функцию из кубических полиномов
    for (int j = 0; j < n - 1; ++j) {
        coef_a.set(j, this->array.get(j).y);                                              // a_j
        coef_b.set(j, delta.get(j) - h.get(j) * (T{2} * m.get(j) + m.get(j + 1)) / T{6}); // b_j
        coef_c.set(j, m.get(j) / T{2});                                                   // c_j
        coef_d.set(j, (m.get(j + 1) - m.get(j)) / (T{6} * h.get(j)));                     // d_j
    }
}

template <Field T> void CubicSpline<T>::store_raw_points() { clear_coefficients(); }

template <Field T> T CubicSpline<T>::evaluate(const T &x) const {
    // Бинарный поиск интервала
    int left = 0;
    int right = this->count - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (x < this->array.get(mid).x) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    int i = right;
    if (i < 0) {
        i = 0;
    } else if (i >= this->count - 1) {
        i = this->count - 2;
    }

    T dx = x - this->array.get(i).x;

    // S_i(x) = a_i + b_i*dx + c_i*dx^2 + d_i*dx^3
    return coef_a.get(i) + coef_b.get(i) * dx + coef_c.get(i) * dx * dx +
           coef_d.get(i) * dx * dx * dx;
}

template <Field T>
void CubicSpline<T>::solve_tridiag(const DynamicArray<T> &lower_diag,
                                   const DynamicArray<T> &main_diag,
                                   const DynamicArray<T> &upper_diag, const DynamicArray<T> &rhs,
                                   DynamicArray<T> &solution) {
    // решение через метод Томаса, оптимально для трёхдиагональных систем
    // дает O(n) по времени
    DynamicArray<T> alph(this->count - 1);
    DynamicArray<T> beta(this->count - 1);

    // Прямая прогонка
    alph.set(0, -upper_diag.get(0) / main_diag.get(0));
    beta.set(0, rhs.get(0) / main_diag.get(0));

    for (int i = 1; i < this->count - 1; ++i) {
        const T denom = main_diag.get(i) + lower_diag.get(i) * alph.get(i - 1);
        alph.set(i, -upper_diag.get(i) / denom);
        beta.set(i, (rhs.get(i) - lower_diag.get(i) * beta.get(i - 1)) / denom);
    }

    solution.set(
        this->count - 1,
        (rhs.get(this->count - 1) - lower_diag.get(this->count - 1) * beta.get(this->count - 2)) /
            (main_diag.get(this->count - 1) +
             lower_diag.get(this->count - 1) * alph.get(this->count - 2)));

    // Обратная прогонка
    for (int i = this->count - 2; i >= 0; --i) {
        solution.set(i, solution.get(i + 1) * alph.get(i) + beta.get(i));
    }
}

// === Sequence методы ===

// === Операции над сегментами ===
template <Field T> int CubicSpline<T>::get_segment_count() const { return coef_a.get_size(); }

template <Field T> SplineSegment<T> CubicSpline<T>::get_segment(int index) const {
    if (index < 0 || index >= coef_a.get_size()) {
        throw std::out_of_range("Spline segment index out of range");
    }

    return SplineSegment<T>{this->array.get(index).x, this->array.get(index + 1).x,
                            coef_a.get(index),        coef_b.get(index),
                            coef_c.get(index),        coef_d.get(index)};
}

template <Field T> Option<SplineSegment<T>> CubicSpline<T>::try_get_segment(int index) const {
    if (index < 0 || index >= coef_a.get_size()) {
        return Option<SplineSegment<T>>::None();
    }

    return Option<SplineSegment<T>>::Some(get_segment(index));
}

// === Операции над нодами ===
template <Field T> Sequence<Point<T>> *CubicSpline<T>::get_sub_sequence(int start, int end) {
    Sequence<Point<T>> *result_seq = ArraySequence<Point<T>>::get_sub_sequence(start, end);
    CubicSpline<T> *result = static_cast<CubicSpline<T> *>(result_seq);
    result->rebuild_or_store_raw();
    return result;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::append(const Point<T> &item) {
    Sequence<Point<T>> *target_seq = ArraySequence<Point<T>>::append(item);
    CubicSpline<T> *target = static_cast<CubicSpline<T> *>(target_seq);
    target->rebuild_or_store_raw();
    return target;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::prepend(const Point<T> &item) {
    Sequence<Point<T>> *target_seq = ArraySequence<Point<T>>::prepend(item);
    CubicSpline<T> *target = static_cast<CubicSpline<T> *>(target_seq);
    target->rebuild_or_store_raw();
    return target;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::insert_at(const Point<T> &item, int index) {
    Sequence<Point<T>> *target_seq = ArraySequence<Point<T>>::insert_at(item, index);
    CubicSpline<T> *target = static_cast<CubicSpline<T> *>(target_seq);
    target->rebuild_or_store_raw();
    return target;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::concat(const Sequence<Point<T>> *other) {
    Sequence<Point<T>> *result_seq = ArraySequence<Point<T>>::concat(other);
    CubicSpline<T> *result = static_cast<CubicSpline<T> *>(result_seq);
    result->rebuild_or_store_raw();
    return result;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::map(Point<T> (*func)(const Point<T> &elem)) {
    Sequence<Point<T>> *result_seq = ArraySequence<Point<T>>::map(func);
    CubicSpline<T> *result = static_cast<CubicSpline<T> *>(result_seq);
    result->rebuild_or_store_raw();
    return result;
}

template <Field T>
Sequence<Point<T>> *CubicSpline<T>::where(bool (*predicate)(const Point<T> &elem)) {
    Sequence<Point<T>> *result_seq = ArraySequence<Point<T>>::where(predicate);
    CubicSpline<T> *result = static_cast<CubicSpline<T> *>(result_seq);
    result->rebuild_or_store_raw();
    return result;
}

template <Field T>
Point<T> CubicSpline<T>::reduce(Point<T> (*func)(const Point<T> &first_elem,
                                                 const Point<T> &second_elem),
                                const Point<T> &initial_elem) {
    return ArraySequence<Point<T>>::reduce(func, initial_elem);
}

template <Field T>
Sequence<Point<T>> *CubicSpline<T>::slice(int index, int count,
                                          const Sequence<Point<T>> *replace_seq) {
    Sequence<Point<T>> *result_seq = ArraySequence<Point<T>>::slice(index, count, replace_seq);
    CubicSpline<T> *result = static_cast<CubicSpline<T> *>(result_seq);
    result->rebuild_or_store_raw();
    return result;
}
