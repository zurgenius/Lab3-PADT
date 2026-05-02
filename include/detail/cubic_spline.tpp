#pragma once

#include "cubic_spline.h"
#include <stdexcept>

template <Field T> cubic_spline<T>::cubic_spline() : n_(0), x_(nullptr), f_(nullptr), Coef_(nullptr) {}

template <Field T>
cubic_spline<T>::~cubic_spline() {
    delete[] x_;
    delete[] f_;
    if (Coef_) {
        delete[] Coef_[0];
        delete[] Coef_[1];
        delete[] Coef_[2];
        delete[] Coef_[3];
        delete[] Coef_;
    }
}

template <Field T>
void cubic_spline<T>::build(const T* x, const T* f, int n) {
    if (n < 3) {
        throw std::invalid_argument("cubic_spline::build: at least 3 points required");
    }

    n_ = n;
    x_ = new T[n];
    f_ = new T[n];
    for (int i = 0; i < n; ++i) {
        x_[i] = x[i];
        f_[i] = f[i];
    }

    // Вспомогательные массивы
    T* a     = new T[n - 1];
    T* c     = new T[n - 1];
    T* d     = new T[n - 1];
    T* delta = new T[n - 1];
    T* h     = new T[n - 1];

    // Трёхдиагональная матрица (размер 3 x n)
    T** TriDiagMatrix = new T*[3];
    TriDiagMatrix[0] = new T[n];   // поддиагональ
    TriDiagMatrix[1] = new T[n];   // главная диагональ
    TriDiagMatrix[2] = new T[n];   // наддиагональ

    T* f_vec = new T[n];           // правая часть
    T* b     = new T[n];           // решение (вторые производные)

    // Граничные величины
    T x3 = x_[2] - x_[0];
    T xn = x_[n - 1] - x_[n - 3];

    // Заполнение шагов и предварительных разностей
    for (int i = 0; i < n - 1; ++i) {
        a[i]     = f_[i];
        h[i]     = x_[i + 1] - x_[i];
        delta[i] = (f_[i + 1] - f_[i]) / h[i];

        TriDiagMatrix[0][i] = (i > 0) ? h[i] : x3;
        f_vec[i] = (i > 0) ? T{3} * (h[i] * delta[i - 1] + h[i - 1] * delta[i])
                           : T{0};
    }

    TriDiagMatrix[1][0] = h[0];
    TriDiagMatrix[2][0] = h[0];

    for (int i = 1; i < n - 1; ++i) {
        TriDiagMatrix[1][i] = T{2} * (h[i] + h[i - 1]);
        TriDiagMatrix[2][i] = h[i];
    }

    TriDiagMatrix[1][n - 1] = h[n - 2];
    TriDiagMatrix[2][n - 1] = xn;
    TriDiagMatrix[0][n - 1] = h[n - 2];

    // Правая часть для граничных условий
    int last = n - 1;
    f_vec[0] = ((h[0] + T{2} * x3) * h[1] * delta[0] + h[0] * h[0] * delta[1]) / x3;
    f_vec[last] = (h[last - 1] * h[last - 1] * delta[last - 2]
                   + (T{2} * xn + h[last - 1]) * h[last - 2] * delta[last - 1])
                  / xn;

    // Решение системы
    solve_tridiag(TriDiagMatrix, f_vec, b);

    // Матрица коэффициентов [a, b, c, d] для каждого интервала
    Coef_ = new T*[4];
    Coef_[0] = new T[n - 1];
    Coef_[1] = new T[n - 1];
    Coef_[2] = new T[n - 1];
    Coef_[3] = new T[n - 1];

    for (int j = 0; j < n - 1; ++j) {
        d[j] = (b[j + 1] + b[j] - T{2} * delta[j]) / (h[j] * h[j]);
        c[j] = T{2} * (delta[j] - b[j]) / h[j] - (b[j + 1] - delta[j]) / h[j];

        Coef_[0][j] = a[j];
        Coef_[1][j] = b[j];
        Coef_[2][j] = c[j];
        Coef_[3][j] = d[j];
    }

    // Освобождение временных массивов
    delete[] a;
    delete[] c;
    delete[] d;
    delete[] delta;
    delete[] h;
    delete[] TriDiagMatrix[0];
    delete[] TriDiagMatrix[1];
    delete[] TriDiagMatrix[2];
    delete[] TriDiagMatrix;
    delete[] f_vec;
    delete[] b;
}

template <Field T>
T cubic_spline<T>::evaluate(const T& x) const {
    // Поиск нужного сегмента (линейный; можно заменить бинарным при необходимости)
    int i = 0;
    while (i < n_ && x_[i] < x) {
        ++i;
    }
    if (i == 0) {
        i = 0;
    } else if (i == n_) {
        i = n_ - 2;
    } else {
        --i;
    }

    T dx = x - x_[i];
    // S_i(x) = a_i + b_i*dx + c_i*dx^2 + d_i*dx^3
    return Coef_[0][i]
           + Coef_[1][i] * dx
           + Coef_[2][i] * dx * dx
           + Coef_[3][i] * dx * dx * dx;
}

template <Field T>
void cubic_spline<T>::solve_tridiag(T** TDM, T* F, T* b) {
    T* alph = new T[n_ - 1];
    T* beta = new T[n_ - 1];

    // Прямая прогонка
    alph[0] = -TDM[2][0] / TDM[1][0];
    beta[0] = F[0] / TDM[1][0];

    for (int i = 1; i < n_ - 1; ++i) {
        T denom = TDM[1][i] + TDM[0][i] * alph[i - 1];
        alph[i] = -TDM[2][i] / denom;
        beta[i] = (F[i] - TDM[0][i] * beta[i - 1]) / denom;
    }

    b[n_ - 1] = (F[n_ - 1] - TDM[0][n_ - 1] * beta[n_ - 2])
                / (TDM[1][n_ - 1] + TDM[0][n_ - 1] * alph[n_ - 2]);

    // Обратная прогонка
    for (int i = n_ - 2; i >= 0; --i) {
        b[i] = b[i + 1] * alph[i] + beta[i];
    }

    delete[] alph;
    delete[] beta;
}