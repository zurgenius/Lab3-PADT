#pragma once

#include "cubic_spline.h"
#include <stdexcept>

template <Field T>
CubicSpline<T>::CubicSpline() : nodes_count(0), nodes(), coef_a(), coef_b(), coef_c(), coef_d() {}

template <Field T>
CubicSpline<T>::CubicSpline(const CubicSpline<T> &other)
    : nodes_count(other.nodes_count), nodes(other.nodes), coef_a(other.coef_a),
      coef_b(other.coef_b), coef_c(other.coef_c), coef_d(other.coef_d) {}

template <Field T> CubicSpline<T>::~CubicSpline() {}

template <Field T> void CubicSpline<T>::build(const Point<T> *points, int n) {
    if (n < 3) {
        throw std::invalid_argument("CubicSpline::build: at least 3 points required");
    }

    nodes_count = n;
    nodes.resize(n);
    for (int i = 0; i < n; ++i) {
        nodes.set(i, points[i]);
    }

    // вычисление h_i и delta_i
    T *delta = new T[n - 1];
    T *h = new T[n - 1];
    for (int i = 0; i < n - 1; ++i) {
        h[i] = nodes.get(i + 1).x - nodes.get(i).x;
        delta[i] = (nodes.get(i + 1).y - nodes.get(i).y) / h[i];
    }

    // построение трёхдиагональной системы Am = F
    //  здесь неизвестные - вторые производные в узлах
    T **TriDiagMatrix = new T *[3];
    TriDiagMatrix[0] = new T[n];
    TriDiagMatrix[1] = new T[n];
    TriDiagMatrix[2] = new T[n];

    T *f_vec = new T[n];
    T *m = new T[n];

    for (int i = 0; i < n; ++i) {
        TriDiagMatrix[0][i] = T{0};
        TriDiagMatrix[1][i] = T{0};
        TriDiagMatrix[2][i] = T{0};
        f_vec[i] = T{0};
    }

    TriDiagMatrix[1][0] = T{1};
    TriDiagMatrix[1][n - 1] = T{1};

    // в этом цикле строим уравнение для внутренних узлов; для крайних задаём m[0] = m[n-1] = 0
    // (естественный сплайн)
    // такое уравнение гарантирует непрерывность первой производной в узлах
    for (int i = 1; i < n - 1; ++i) {
        TriDiagMatrix[0][i] = h[i - 1];
        TriDiagMatrix[1][i] = T{2} * (h[i - 1] + h[i]);
        TriDiagMatrix[2][i] = h[i];
        f_vec[i] = T{6} * (delta[i] - delta[i - 1]);
    }

    // решение системы методом прогонки (Томаса)
    solve_tridiag(TriDiagMatrix, f_vec, m);

    // вычисление коэффициентов сплайна
    coef_a.resize(n - 1);
    coef_b.resize(n - 1);
    coef_c.resize(n - 1);
    coef_d.resize(n - 1);

    // храним матрицу коээфициентов 4*n-1, где n-1 - кол-во кусков в сплайне
    //  по этой матрице можем однозначно задать кусочно-заданную функцию из кубических полиномов
    for (int j = 0; j < n - 1; ++j) {
        coef_a.set(j, nodes.get(j).y);                                    // a_j
        coef_b.set(j, delta[j] - h[j] * (T{2} * m[j] + m[j + 1]) / T{6}); // b_j
        coef_c.set(j, m[j] / T{2});                                       // c_j
        coef_d.set(j, (m[j + 1] - m[j]) / (T{6} * h[j]));                 // d_j
    }

    delete[] delta;
    delete[] h;
    delete[] TriDiagMatrix[0];
    delete[] TriDiagMatrix[1];
    delete[] TriDiagMatrix[2];
    delete[] TriDiagMatrix;
    delete[] f_vec;
    delete[] m;
}

template <Field T> void CubicSpline<T>::take_raw(const Point<T> *points, int n) {
    nodes_count = n;
    nodes.resize(n);
    for (int index = 0; index < n; ++index) {
        nodes.set(index, points[index]);
    }
    coef_a.resize(0);
    coef_b.resize(0);
    coef_c.resize(0);
    coef_d.resize(0);
}

template <Field T> T CubicSpline<T>::evaluate(const T &x) const {
    // Бинарный поиск интервала
    int left = 0;
    int right = nodes_count - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (x < nodes.get(mid).x) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    int i = right;
    if (i < 0) {
        i = 0;
    } else if (i >= nodes_count - 1) {
        i = nodes_count - 2;
    }

    T dx = x - nodes.get(i).x;

    // S_i(x) = a_i + b_i*dx + c_i*dx^2 + d_i*dx^3
    return coef_a.get(i) + coef_b.get(i) * dx + coef_c.get(i) * dx * dx +
           coef_d.get(i) * dx * dx * dx;
}

template <Field T> int CubicSpline<T>::get_segment_count() const { return coef_a.get_size(); }

template <Field T> SplineSegment<T> CubicSpline<T>::get_segment(int index) const {
    if (index < 0 || index >= coef_a.get_size()) {
        throw std::out_of_range("Spline segment index out of range");
    }

    return SplineSegment<T>{nodes.get(index).x, nodes.get(index + 1).x, coef_a.get(index),
                            coef_b.get(index),  coef_c.get(index),      coef_d.get(index)};
}

template <Field T> Option<SplineSegment<T>> CubicSpline<T>::try_get_segment(int index) const {
    if (index < 0 || index >= coef_a.get_size()) {
        return Option<SplineSegment<T>>::None();
    }

    return Option<SplineSegment<T>>::Some(get_segment(index));
}

template <Field T> const Point<T> &CubicSpline<T>::get_first() const {
    if (nodes_count == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    return nodes.get(0);
}

template <Field T> const Point<T> &CubicSpline<T>::get_last() const {
    if (nodes_count == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    return nodes.get(nodes_count - 1);
}

template <Field T> const Point<T> &CubicSpline<T>::get(int index) const {
    if (index < 0 || index >= nodes_count) {
        throw std::out_of_range("Index out of range");
    }
    return nodes.get(index);
}

template <Field T> Option<Point<T>> CubicSpline<T>::try_get_first() const {
    return nodes_count == 0 ? Option<Point<T>>::None() : Option<Point<T>>::Some(nodes.get(0));
}

template <Field T> Option<Point<T>> CubicSpline<T>::try_get_last() const {
    return nodes_count == 0 ? Option<Point<T>>::None()
                            : Option<Point<T>>::Some(nodes.get(nodes_count - 1));
}

template <Field T> Option<Point<T>> CubicSpline<T>::try_get(int index) const {
    return (index < 0 || index >= nodes_count) ? Option<Point<T>>::None()
                                               : Option<Point<T>>::Some(nodes.get(index));
}

template <Field T> int CubicSpline<T>::get_count() const { return nodes_count; }

template <Field T> Sequence<Point<T>> *CubicSpline<T>::get_sub_sequence(int start, int end) {
    if (start < 0 || end < 0 || start >= nodes_count || end >= nodes_count || start > end) {
        throw std::out_of_range("Index out of range");
    }

    const int length = end - start + 1;
    Point<T> *new_points = new Point<T>[length];
    for (int offset = 0; offset < length; offset++) {
        new_points[offset] = nodes.get(start + offset);
    }

    CubicSpline<T> *result = EmptyClone();
    if (length >= 3) {
        result->build(new_points, length);
        delete[] new_points;
    } else {
        result->take_raw(new_points, length);
        delete[] new_points;
    }

    return result;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::append(const Point<T> &item) {
    CubicSpline<T> *target = Instance();
    const int new_count = nodes_count + 1;
    Point<T> *new_points = new Point<T>[new_count];
    for (int index = 0; index < nodes_count; index++) {
        new_points[index] = nodes.get(index);
    }

    new_points[nodes_count] = item;

    if (new_count >= 3) {
        target->build(new_points, new_count);
        delete[] new_points;
    } else {
        target->take_raw(new_points, new_count);
        delete[] new_points;
    }

    return target;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::prepend(const Point<T> &item) {
    CubicSpline<T> *target = Instance();
    const int new_count = nodes_count + 1;
    Point<T> *new_points = new Point<T>[new_count];
    new_points[0] = item;

    for (int index = 0; index < nodes_count; index++) {
        new_points[index + 1] = nodes.get(index);
    }

    if (new_count >= 3) {
        target->build(new_points, new_count);
        delete[] new_points;
    } else {
        target->take_raw(new_points, new_count);
        delete[] new_points;
    }

    return target;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::insert_at(const Point<T> &item, int index) {
    if (index < 0 || index > nodes_count) {
        throw std::out_of_range("Index out of range");
    }
    if (index == 0) {
        return prepend(item);
    }
    if (index == nodes_count) {
        return append(item);
    }

    CubicSpline<T> *target = Instance();
    const int new_count = nodes_count + 1;
    Point<T> *new_points = new Point<T>[new_count];

    for (int current = 0; current < index; current++) {
        new_points[current] = nodes.get(current);
    }

    new_points[index] = item;

    for (int current = index; current < nodes_count; current++) {
        new_points[current + 1] = nodes.get(current);
    }

    if (new_count >= 3) {
        target->build(new_points, new_count);
        delete[] new_points;
    } else {
        target->take_raw(new_points, new_count);
        delete[] new_points;
    }

    return target;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::concat(const Sequence<Point<T>> *other) {
    if (other == nullptr) {
        throw std::invalid_argument("Cannot concat with nullptr");
    }

    const int other_count = other->get_count();
    const int new_count = nodes_count + other_count;
    CubicSpline<T> *result = EmptyClone();
    if (new_count == 0) {
        return result;
    }

    Point<T> *new_points = new Point<T>[new_count];
    for (int index = 0; index < nodes_count; index++) {
        new_points[index] = nodes.get(index);
    }

    for (int index = 0; index < other_count; index++) {
        new_points[nodes_count + index] = other->get(index);
    }

    if (new_count >= 3) {
        result->build(new_points, new_count);
        delete[] new_points;
    } else {
        result->take_raw(new_points, new_count);
        delete[] new_points;
    }

    return result;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::map(Point<T> (*func)(const Point<T> &elem)) {
    if (nodes_count == 0) {
        return EmptyClone();
    }

    Point<T> *new_points = new Point<T>[nodes_count];
    for (int index = 0; index < nodes_count; index++) {
        new_points[index] = func(nodes.get(index));
    }

    CubicSpline<T> *result = EmptyClone();
    if (nodes_count >= 3) {
        result->build(new_points, nodes_count);
        delete[] new_points;
    } else {
        result->take_raw(new_points, nodes_count);
        delete[] new_points;
    }

    return result;
}

template <Field T>
Sequence<Point<T>> *CubicSpline<T>::where(bool (*predicate)(const Point<T> &elem)) {
    int matches = 0;
    for (int index = 0; index < nodes_count; index++) {
        if (predicate(nodes.get(index))) {
            matches++;
        }
    }

    CubicSpline<T> *result = EmptyClone();
    if (matches == 0) {
        return result;
    }

    Point<T> *new_points = new Point<T>[matches];
    int write_index = 0;
    for (int index = 0; index < nodes_count; index++) {
        const Point<T> &point = nodes.get(index);
        if (predicate(point)) {
            new_points[write_index] = point;
            write_index++;
        }
    }

    if (matches >= 3) {
        result->build(new_points, matches);
        delete[] new_points;
    } else {
        result->take_raw(new_points, matches);
        delete[] new_points;
    }

    return result;
}

template <Field T>
Point<T> CubicSpline<T>::reduce(Point<T> (*func)(const Point<T> &first_elem,
                                                 const Point<T> &second_elem),
                                const Point<T> &initial_elem) {
    Point<T> accumulated = initial_elem;
    for (int index = 0; index < nodes_count; index++) {
        accumulated = func(accumulated, nodes.get(index));
    }
    return accumulated;
}

template <Field T>
Sequence<Point<T>> *CubicSpline<T>::slice(int index, int count,
                                          const Sequence<Point<T>> *replace_seq) {
    const int length = nodes_count;
    if (count < 0) {
        throw std::invalid_argument("Slice count cannot be negative");
    }
    if (length == 0) {
        throw std::out_of_range("Slice index out of range");
    }

    if (index < 0) {
        index += length;
    }
    if (index < 0 || index >= length) {
        throw std::out_of_range("Slice index out of range");
    }

    const int removed = (count > length - index) ? (length - index) : count;
    const int replacement_count = (replace_seq == nullptr) ? 0 : replace_seq->get_count();
    const int result_count = index + replacement_count + (length - index - removed);

    CubicSpline<T> *result = EmptyClone();
    if (result_count == 0) {
        return result;
    }

    Point<T> *new_points = new Point<T>[result_count];
    int write_index = 0;
    for (int current = 0; current < index; current++) {
        new_points[write_index] = nodes.get(current);
        write_index++;
    }

    if (replacement_count > 0) {
        for (int current = 0; current < replacement_count; current++) {
            new_points[write_index] = replace_seq->get(current);
            write_index++;
        }
    }

    for (int current = index + removed; current < length; current++) {
        new_points[write_index] = nodes.get(current);
        write_index++;
    }

    if (result_count >= 3) {
        result->build(new_points, result_count);
        delete[] new_points;
    } else {
        result->take_raw(new_points, result_count);
        delete[] new_points;
    }

    return result;
}

template <Field T> IEnumerator<Point<T>> *CubicSpline<T>::get_enumerator() const {
    return new Enumerator(nodes_count == 0 ? nullptr : &nodes.get(0), nodes_count);
}

template <Field T> void CubicSpline<T>::solve_tridiag(T **TDM, T *F, T *b) {
    // решение через метод Томаса, оптимально для трёхдиагональных систем
    // дает O(n) по времени
    T *alph = new T[nodes_count - 1];
    T *beta = new T[nodes_count - 1];

    // Прямая прогонка
    alph[0] = -TDM[2][0] / TDM[1][0];
    beta[0] = F[0] / TDM[1][0];

    for (int i = 1; i < nodes_count - 1; ++i) {
        T denom = TDM[1][i] + TDM[0][i] * alph[i - 1];
        alph[i] = -TDM[2][i] / denom;
        beta[i] = (F[i] - TDM[0][i] * beta[i - 1]) / denom;
    }

    b[nodes_count - 1] =
        (F[nodes_count - 1] - TDM[0][nodes_count - 1] * beta[nodes_count - 2]) /
        (TDM[1][nodes_count - 1] + TDM[0][nodes_count - 1] * alph[nodes_count - 2]);

    // Обратная прогонка
    for (int i = nodes_count - 2; i >= 0; --i) {
        b[i] = b[i + 1] * alph[i] + beta[i];
    }

    delete[] alph;
    delete[] beta;
}
