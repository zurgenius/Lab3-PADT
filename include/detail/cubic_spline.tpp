#pragma once

#include "cubic_spline.h"
#include <stdexcept>

template <Field T>
cubic_spline<T>::cubic_spline() : nodes_count(0), x_coordinates(nullptr), function_values(nullptr), piecewise_given_func_coefs(nullptr) {}

template <Field T> cubic_spline<T>::~cubic_spline() {
    delete[] x_coordinates;
    delete[] function_values;
    if (piecewise_given_func_coefs) {
        delete[] piecewise_given_func_coefs[0];
        delete[] piecewise_given_func_coefs[1];
        delete[] piecewise_given_func_coefs[2];
        delete[] piecewise_given_func_coefs[3];
        delete[] piecewise_given_func_coefs;
    }
}

template <Field T> void cubic_spline<T>::build(const T *x, const T *f, int n) {
    if (n < 3) {
        throw std::invalid_argument("cubic_spline::build: at least 3 points required");
    }

    if (x_coordinates) {
        delete[] x_coordinates;
        x_coordinates = nullptr;
    }
    if (function_values) {
        delete[] function_values;
        function_values = nullptr;
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
    x_coordinates = new T[n];
    function_values = new T[n];
    for (int i = 0; i < n; ++i) {
        x_coordinates[i] = x[i];
        function_values[i] = f[i];
    }

    // вычисление h_i и delta_i
    T *delta = new T[n - 1];
    T *h = new T[n - 1];
    for (int i = 0; i < n - 1; ++i) {
        h[i] = x_coordinates[i + 1] - x_coordinates[i];
        delta[i] = (function_values[i + 1] - function_values[i]) / h[i];
    }

    // построение трёхдиагональной системы Am = F
    //  здесь неизвестные - вторыке производные в узлах
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
        piecewise_given_func_coefs[0][j] = function_values[j];                                             // a_j
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

template <Field T> void cubic_spline<T>::take_raw(T *x, T *f, int n) {
    if (x_coordinates) {
        delete[] x_coordinates;
        x_coordinates = nullptr;
    }
    if (function_values) {
        delete[] function_values;
        function_values = nullptr;
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
    x_coordinates = x;
    function_values = f;
}

template <Field T> T cubic_spline<T>::evaluate(const T &x) const {
    // Бинарный поиск интервала
    int left = 0;
    int right = nodes_count - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (x < x_coordinates[mid]) {
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

    T dx = x - x_coordinates[i];

    // S_i(x) = a_i + b_i*dx + c_i*dx^2 + d_i*dx^3
    return piecewise_given_func_coefs[0][i] + piecewise_given_func_coefs[1][i] * dx + piecewise_given_func_coefs[2][i] * dx * dx + piecewise_given_func_coefs[3][i] * dx * dx * dx;
}

template <Field T> const T &cubic_spline<T>::get_first() const {
    if (nodes_count == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    return function_values[0];
}

template <Field T> const T &cubic_spline<T>::get_last() const {
    if (nodes_count == 0) {
        throw std::out_of_range("Sequence is empty");
    }
    return function_values[nodes_count - 1];
}

template <Field T> const T &cubic_spline<T>::get(int index) const {
    if (index < 0 || index >= nodes_count) {
        throw std::out_of_range("Index out of range");
    }
    return function_values[index];
}

template <Field T> Option<T> cubic_spline<T>::try_get_first() const {
    return nodes_count == 0 ? Option<T>::None() : Option<T>::Some(function_values[0]);
}

template <Field T> Option<T> cubic_spline<T>::try_get_last() const {
    return nodes_count == 0 ? Option<T>::None() : Option<T>::Some(function_values[nodes_count - 1]);
}

template <Field T> Option<T> cubic_spline<T>::try_get(int index) const {
    return (index < 0 || index >= nodes_count) ? Option<T>::None() : Option<T>::Some(function_values[index]);
}

template <Field T> int cubic_spline<T>::get_count() const { return nodes_count; }

template <Field T> Sequence<T> *cubic_spline<T>::get_sub_sequence(int start, int end) {
    if (start < 0 || end < 0 || start >= nodes_count || end >= nodes_count || start > end) {
        throw std::out_of_range("Index out of range");
    }

    const int length = end - start + 1;
    T *new_x = new T[length];
    T *new_f = new T[length];
    for (int offset = 0; offset < length; offset++) {
        new_x[offset] = x_coordinates[start + offset];
        new_f[offset] = function_values[start + offset];
    }

    cubic_spline<T> *result = new cubic_spline<T>();
    if (length >= 3) {
        result->build(new_x, new_f, length);
        delete[] new_x;
        delete[] new_f;
    } else {
        result->take_raw(new_x, new_f, length);
    }

    return result;
}

template <Field T> Sequence<T> *cubic_spline<T>::append(const T &item) {
    const int new_count = nodes_count + 1;
    T *new_x = new T[new_count];
    T *new_f = new T[new_count];
    for (int index = 0; index < nodes_count; index++) {
        new_x[index] = x_coordinates[index];
        new_f[index] = function_values[index];
    }

    if (nodes_count == 0) {
        new_x[0] = T{0};
    } else {
        new_x[nodes_count] = x_coordinates[nodes_count - 1] + T{1};
    }
    new_f[nodes_count] = item;

    if (new_count >= 3) {
        build(new_x, new_f, new_count);
        delete[] new_x;
        delete[] new_f;
    } else {
        take_raw(new_x, new_f, new_count);
    }

    return this;
}

template <Field T> Sequence<T> *cubic_spline<T>::prepend(const T &item) {
    const int new_count = nodes_count + 1;
    T *new_x = new T[new_count];
    T *new_f = new T[new_count];

    if (nodes_count == 0) {
        new_x[0] = T{0};
    } else {
        new_x[0] = x_coordinates[0] - T{1};
    }
    new_f[0] = item;

    for (int index = 0; index < nodes_count; index++) {
        new_x[index + 1] = x_coordinates[index];
        new_f[index + 1] = function_values[index];
    }

    if (new_count >= 3) {
        build(new_x, new_f, new_count);
        delete[] new_x;
        delete[] new_f;
    } else {
        take_raw(new_x, new_f, new_count);
    }

    return this;
}

template <Field T> Sequence<T> *cubic_spline<T>::insert_at(const T &item, int index) {
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
    T *new_x = new T[new_count];
    T *new_f = new T[new_count];

    for (int current = 0; current < index; current++) {
        new_x[current] = x_coordinates[current];
        new_f[current] = function_values[current];
    }

    new_x[index] = (x_coordinates[index - 1] + x_coordinates[index]) / T{2};
    new_f[index] = item;

    for (int current = index; current < nodes_count; current++) {
        new_x[current + 1] = x_coordinates[current];
        new_f[current + 1] = function_values[current];
    }

    if (new_count >= 3) {
        build(new_x, new_f, new_count);
        delete[] new_x;
        delete[] new_f;
    } else {
        take_raw(new_x, new_f, new_count);
    }

    return this;
}

template <Field T> Sequence<T> *cubic_spline<T>::concat(const Sequence<T> *other) {
    if (other == nullptr) {
        throw std::invalid_argument("Cannot concat with nullptr");
    }

    const int other_count = other->get_count();
    const int new_count = nodes_count + other_count;
    cubic_spline<T> *result = new cubic_spline<T>();
    if (new_count == 0) {
        return result;
    }

    T *new_x = new T[new_count];
    T *new_f = new T[new_count];
    for (int index = 0; index < nodes_count; index++) {
        new_x[index] = x_coordinates[index];
        new_f[index] = function_values[index];
    }

    T start_x = (nodes_count == 0) ? T{0} : x_coordinates[nodes_count - 1] + T{1};
    for (int index = 0; index < other_count; index++) {
        new_x[nodes_count + index] = start_x + static_cast<T>(index);
        new_f[nodes_count + index] = other->get(index);
    }

    if (new_count >= 3) {
        result->build(new_x, new_f, new_count);
        delete[] new_x;
        delete[] new_f;
    } else {
        result->take_raw(new_x, new_f, new_count);
    }

    return result;
}

template <Field T> Sequence<T> *cubic_spline<T>::map(T (*func)(const T &elem)) {
    if (nodes_count == 0) {
        return new cubic_spline<T>();
    }

    T *new_x = new T[nodes_count];
    T *new_f = new T[nodes_count];
    for (int index = 0; index < nodes_count; index++) {
        new_x[index] = x_coordinates[index];
        new_f[index] = func(function_values[index]);
    }

    cubic_spline<T> *result = new cubic_spline<T>();
    if (nodes_count >= 3) {
        result->build(new_x, new_f, nodes_count);
        delete[] new_x;
        delete[] new_f;
    } else {
        result->take_raw(new_x, new_f, nodes_count);
    }

    return result;
}

template <Field T> Sequence<T> *cubic_spline<T>::where(bool (*predicate)(const T &elem)) {
    int matches = 0;
    for (int index = 0; index < nodes_count; index++) {
        if (predicate(function_values[index])) {
            matches++;
        }
    }

    cubic_spline<T> *result = new cubic_spline<T>();
    if (matches == 0) {
        return result;
    }

    T *new_x = new T[matches];
    T *new_f = new T[matches];
    int write_index = 0;
    for (int index = 0; index < nodes_count; index++) {
        if (predicate(function_values[index])) {
            new_x[write_index] = x_coordinates[index];
            new_f[write_index] = function_values[index];
            write_index++;
        }
    }

    if (matches >= 3) {
        result->build(new_x, new_f, matches);
        delete[] new_x;
        delete[] new_f;
    } else {
        result->take_raw(new_x, new_f, matches);
    }

    return result;
}

template <Field T>
T cubic_spline<T>::reduce(T (*func)(const T &first_elem, const T &second_elem),
                          const T &initial_elem) {
    T accumulated = initial_elem;
    for (int index = 0; index < nodes_count; index++) {
        accumulated = func(accumulated, function_values[index]);
    }
    return accumulated;
}

template <Field T>
Sequence<T> *cubic_spline<T>::slice(int index, int count, const Sequence<T> *replace_seq) {
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

    cubic_spline<T> *result = new cubic_spline<T>();
    if (result_count == 0) {
        return result;
    }

    T *new_x = new T[result_count];
    T *new_f = new T[result_count];
    int write_index = 0;
    for (int current = 0; current < index; current++) {
        new_x[write_index] = x_coordinates[current];
        new_f[write_index] = function_values[current];
        write_index++;
    }

    if (replacement_count > 0) {
        const bool has_left = index > 0;
        const bool has_right = (index + removed) < length;
        T left_x = has_left ? x_coordinates[index - 1] : T{0};
        T right_x = has_right ? x_coordinates[index + removed] : T{0};

        T step = T{1};
        if (has_left && has_right) {
            step = (right_x - left_x) / static_cast<T>(replacement_count + 1);
        }

        for (int current = 0; current < replacement_count; current++) {
            if (has_left && has_right) {
                new_x[write_index] = left_x + step * static_cast<T>(current + 1);
            } else if (has_left) {
                new_x[write_index] = left_x + static_cast<T>(current + 1);
            } else if (has_right) {
                new_x[write_index] = right_x - static_cast<T>(replacement_count - current);
            } else {
                new_x[write_index] = static_cast<T>(current);
            }
            new_f[write_index] = replace_seq->get(current);
            write_index++;
        }
    }

    for (int current = index + removed; current < length; current++) {
        new_x[write_index] = x_coordinates[current];
        new_f[write_index] = function_values[current];
        write_index++;
    }

    if (result_count >= 3) {
        result->build(new_x, new_f, result_count);
        delete[] new_x;
        delete[] new_f;
    } else {
        result->take_raw(new_x, new_f, result_count);
    }

    return result;
}

template <Field T> IEnumerator<T> *cubic_spline<T>::get_enumerator() const {
    return new Enumerator(function_values, nodes_count);
}

template <Field T> void cubic_spline<T>::solve_tridiag(T **TDM, T *F, T *b) {
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