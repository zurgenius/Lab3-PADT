#pragma once

#include "cubic_spline.h"
#include <stdexcept>

template <Field T>
CubicSpline<T>::CubicSpline() : nodes_count(0), nodes(nullptr), piecewise_given_func_coefs(nullptr) {}

template <Field T> CubicSpline<T>::~CubicSpline() {
    delete[] nodes;
    if (piecewise_given_func_coefs) {
        delete[] piecewise_given_func_coefs[0];
        delete[] piecewise_given_func_coefs[1];
        delete[] piecewise_given_func_coefs[2];
        delete[] piecewise_given_func_coefs[3];
        delete[] piecewise_given_func_coefs;
    }
}


template <Field T> void CubicSpline<T>::build(const Point<T> *points, int n) {
    if (n < 3) {
        throw std::invalid_argument("CubicSpline::build: at least 3 points required");
    }

    if (nodes) {
        delete[] nodes;
        nodes = nullptr;
    }
    if (piecewise_given_func_coefs) {
        delete[] piecewise_given_func_coefs[0];
        delete[] piecewise_given_func_coefs[1];
        delete[] piecewise_given_func_coefs[2];
        delete[] piecewise_given_func_coefs[3];
        delete[] piecewise_given_func_coefs;
        piecewise_given_func_coefs = nullptr;
    }

    nodes_count = n;
    nodes = new Point<T>[n];
    for (int i = 0; i < n; ++i) {
        nodes[i] = points[i];
    }

    // вычисление h_i и delta_i
    T *delta = new T[n - 1];
    T *h = new T[n - 1];
    for (int i = 0; i < n - 1; ++i) {
        h[i] = nodes[i + 1].x - nodes[i].x;
        delta[i] = (nodes[i + 1].y - nodes[i].y) / h[i];
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
    piecewise_given_func_coefs = new T *[4];
    piecewise_given_func_coefs[0] = new T[n - 1];
    piecewise_given_func_coefs[1] = new T[n - 1];
    piecewise_given_func_coefs[2] = new T[n - 1];
    piecewise_given_func_coefs[3] = new T[n - 1];

    // храним матрицу коээфициентов 4*n-1, где n-1 - кол-во кусков в сплайне
    //  по этой матрице можем однозначно задать кусочно-заданную функцию из кубических полиномов
    for (int j = 0; j < n - 1; ++j) {
        piecewise_given_func_coefs[0][j] = nodes[j].y;                                             // a_j
        piecewise_given_func_coefs[1][j] = delta[j] - h[j] * (T{2} * m[j] + m[j + 1]) / T{6}; // b_j
        piecewise_given_func_coefs[2][j] = m[j] / T{2};                                       // c_j
        piecewise_given_func_coefs[3][j] = (m[j + 1] - m[j]) / (T{6} * h[j]);                 // d_j
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

template <Field T> void CubicSpline<T>::take_raw(Point<T> *points, int n) {
    if (nodes) {
        delete[] nodes;
        nodes = nullptr;
    }
    if (piecewise_given_func_coefs) {
        delete[] piecewise_given_func_coefs[0];
        delete[] piecewise_given_func_coefs[1];
        delete[] piecewise_given_func_coefs[2];
        delete[] piecewise_given_func_coefs[3];
        delete[] piecewise_given_func_coefs;
        piecewise_given_func_coefs = nullptr;
    }

    nodes_count = n;
    nodes = points;
}

template <Field T> T CubicSpline<T>::evaluate(const T &x) const {
    // Бинарный поиск интервала
    int left = 0;
    int right = nodes_count - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (x < nodes[mid].x) {
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

    T dx = x - nodes[i].x;

    // S_i(x) = a_i + b_i*dx + c_i*dx^2 + d_i*dx^3
    return piecewise_given_func_coefs[0][i] + piecewise_given_func_coefs[1][i] * dx + piecewise_given_func_coefs[2][i] * dx * dx + piecewise_given_func_coefs[3][i] * dx * dx * dx;
}

template <Field T> const Point<T> &CubicSpline<T>::get_first() const {
    if (nodes_count == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    return nodes[0];
}

template <Field T> const Point<T> &CubicSpline<T>::get_last() const {
    if (nodes_count == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    return nodes[nodes_count - 1];
}

template <Field T> const Point<T> &CubicSpline<T>::get(int index) const {
    if (index < 0 || index >= nodes_count) {
        throw std::out_of_range("Index out of range");
    }
    return nodes[index];
}

template <Field T> Option<Point<T>> CubicSpline<T>::try_get_first() const {
    return nodes_count == 0 ? Option<Point<T>>::None() : Option<Point<T>>::Some(nodes[0]);
}

template <Field T> Option<Point<T>> CubicSpline<T>::try_get_last() const {
    return nodes_count == 0 ? Option<Point<T>>::None() : Option<Point<T>>::Some(nodes[nodes_count - 1]);
}

template <Field T> Option<Point<T>> CubicSpline<T>::try_get(int index) const {
    return (index < 0 || index >= nodes_count) ? Option<Point<T>>::None() : Option<Point<T>>::Some(nodes[index]);
}

template <Field T> int CubicSpline<T>::get_count() const { return nodes_count; }

template <Field T> Sequence<Point<T>> *CubicSpline<T>::get_sub_sequence(int start, int end) {
    if (start < 0 || end < 0 || start >= nodes_count || end >= nodes_count || start > end) {
        throw std::out_of_range("Index out of range");
    }

    const int length = end - start + 1;
    Point<T> *new_points = new Point<T>[length];
    for (int offset = 0; offset < length; offset++) {
        new_points[offset] = nodes[start + offset];
    }

    CubicSpline<T> *result = new CubicSpline<T>();
    if (length >= 3) {
        result->build(new_points, length);
        delete[] new_points;
    } else {
        result->take_raw(new_points, length);
    }

    return result;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::append(const Point<T> &item) {
    const int new_count = nodes_count + 1;
    Point<T> *new_points = new Point<T>[new_count];
    for (int index = 0; index < nodes_count; index++) {
        new_points[index] = nodes[index];
    }

    new_points[nodes_count] = item;

    if (new_count >= 3) {
        build(new_points, new_count);
        delete[] new_points;
    } else {
        take_raw(new_points, new_count);
    }

    return this;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::prepend(const Point<T> &item) {
    const int new_count = nodes_count + 1;
    Point<T> *new_points = new Point<T>[new_count];
    new_points[0] = item;

    for (int index = 0; index < nodes_count; index++) {
        new_points[index + 1] = nodes[index];
    }

    if (new_count >= 3) {
        build(new_points, new_count);
        delete[] new_points;
    } else {
        take_raw(new_points, new_count);
    }

    return this;
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

    const int new_count = nodes_count + 1;
    Point<T> *new_points = new Point<T>[new_count];

    for (int current = 0; current < index; current++) {
        new_points[current] = nodes[current];
    }

    new_points[index] = item;

    for (int current = index; current < nodes_count; current++) {
        new_points[current + 1] = nodes[current];
    }

    if (new_count >= 3) {
        build(new_points, new_count);
        delete[] new_points;
    } else {
        take_raw(new_points, new_count);
    }

    return this;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::concat(const Sequence<Point<T>> *other) {
    if (other == nullptr) {
        throw std::invalid_argument("Cannot concat with nullptr");
    }

    const int other_count = other->get_count();
    const int new_count = nodes_count + other_count;
    CubicSpline<T> *result = new CubicSpline<T>();
    if (new_count == 0) {
        return result;
    }

    Point<T> *new_points = new Point<T>[new_count];
    for (int index = 0; index < nodes_count; index++) {
        new_points[index] = nodes[index];
    }

    for (int index = 0; index < other_count; index++) {
        new_points[nodes_count + index] = other->get(index);
    }

    if (new_count >= 3) {
        result->build(new_points, new_count);
        delete[] new_points;
    } else {
        result->take_raw(new_points, new_count);
    }

    return result;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::map(Point<T> (*func)(const Point<T> &elem)) {
    if (nodes_count == 0) {
        return new CubicSpline<T>();
    }

    Point<T> *new_points = new Point<T>[nodes_count];
    for (int index = 0; index < nodes_count; index++) {
        new_points[index] = func(nodes[index]);
    }

    CubicSpline<T> *result = new CubicSpline<T>();
    if (nodes_count >= 3) {
        result->build(new_points, nodes_count);
        delete[] new_points;
    } else {
        result->take_raw(new_points, nodes_count);
    }

    return result;
}

template <Field T> Sequence<Point<T>> *CubicSpline<T>::where(bool (*predicate)(const Point<T> &elem)) {
    int matches = 0;
    for (int index = 0; index < nodes_count; index++) {
        if (predicate(nodes[index])) {
            matches++;
        }
    }

    CubicSpline<T> *result = new CubicSpline<T>();
    if (matches == 0) {
        return result;
    }

    Point<T> *new_points = new Point<T>[matches];
    int write_index = 0;
    for (int index = 0; index < nodes_count; index++) {
        if (predicate(nodes[index])) {
            new_points[write_index] = nodes[index];
            write_index++;
        }
    }

    if (matches >= 3) {
        result->build(new_points, matches);
        delete[] new_points;
    } else {
        result->take_raw(new_points, matches);
    }

    return result;
}

template <Field T>
Point<T> CubicSpline<T>::reduce(Point<T> (*func)(const Point<T> &first_elem,
                                                  const Point<T> &second_elem),
                                 const Point<T> &initial_elem) {
    Point<T> accumulated = initial_elem;
    for (int index = 0; index < nodes_count; index++) {
        accumulated = func(accumulated, nodes[index]);
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

    CubicSpline<T> *result = new CubicSpline<T>();
    if (result_count == 0) {
        return result;
    }

    Point<T> *new_points = new Point<T>[result_count];
    int write_index = 0;
    for (int current = 0; current < index; current++) {
        new_points[write_index] = nodes[current];
        write_index++;
    }

    if (replacement_count > 0) {
        for (int current = 0; current < replacement_count; current++) {
            new_points[write_index] = replace_seq->get(current);
            write_index++;
        }
    }

    for (int current = index + removed; current < length; current++) {
        new_points[write_index] = nodes[current];
        write_index++;
    }

    if (result_count >= 3) {
        result->build(new_points, result_count);
        delete[] new_points;
    } else {
        result->take_raw(new_points, result_count);
    }

    return result;
}

template <Field T> IEnumerator<Point<T>> *CubicSpline<T>::get_enumerator() const {
    return new Enumerator(nodes, nodes_count);
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

    b[nodes_count - 1] = (F[nodes_count - 1] - TDM[0][nodes_count - 1] * beta[nodes_count - 2]) /
                (TDM[1][nodes_count - 1] + TDM[0][nodes_count - 1] * alph[nodes_count - 2]);

    // Обратная прогонка
    for (int i = nodes_count - 2; i >= 0; --i) {
        b[i] = b[i + 1] * alph[i] + beta[i];
    }

    delete[] alph;
    delete[] beta;
}