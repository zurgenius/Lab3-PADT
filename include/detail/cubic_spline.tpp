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

    if (x_) {
        delete[] x_;
        x_ = nullptr;
    }
    if (f_) {
        delete[] f_;
        f_ = nullptr;
    }
    if (Coef_) {
        delete[] Coef_[0];
        delete[] Coef_[1];
        delete[] Coef_[2];
        delete[] Coef_[3];
        delete[] Coef_;
        Coef_ = nullptr;
    }

    n_ = n;
    x_ = new T[n];
    f_ = new T[n];
    for (int i = 0; i < n; ++i) {
        x_[i] = x[i];
        f_[i] = f[i];
    }

    // вычисление h_i и delta_i
    T* delta = new T[n - 1];
    T* h     = new T[n - 1];
    for (int i = 0; i < n - 1; ++i) {
        h[i] = x_[i + 1] - x_[i];
        delta[i] = (f_[i + 1] - f_[i]) / h[i];
    }

    //построение трёхдиагональной системы Am = F
    // здесь неизвестные - вторыке производные в узлах
    T** TriDiagMatrix = new T*[3];
    TriDiagMatrix[0] = new T[n];
    TriDiagMatrix[1] = new T[n];
    TriDiagMatrix[2] = new T[n];

    T* f_vec = new T[n];
    T* m     = new T[n];

    for (int i = 0; i < n; ++i) {
        TriDiagMatrix[0][i] = T{0};
        TriDiagMatrix[1][i] = T{0};
        TriDiagMatrix[2][i] = T{0};
        f_vec[i] = T{0};
    }

    TriDiagMatrix[1][0] = T{1};
    TriDiagMatrix[1][n - 1] = T{1};

    // в этом цикле строим уравнение для внутренних узлов; для крайних задаём m[0] = m[n-1] = 0 (естественный сплайн)
    //такое уравнение гарантирует непрерывность первой производной в узлах
    for (int i = 1; i < n - 1; ++i) {
        TriDiagMatrix[0][i] = h[i - 1];
        TriDiagMatrix[1][i] = T{2} * (h[i - 1] + h[i]);
        TriDiagMatrix[2][i] = h[i];
        f_vec[i] = T{6} * (delta[i] - delta[i - 1]);
    } 
    

    //решение системы методом прогонки (Томаса)
    solve_tridiag(TriDiagMatrix, f_vec, m);

    // вычисление коэффициентов сплайна
    Coef_ = new T*[4];
    Coef_[0] = new T[n - 1];
    Coef_[1] = new T[n - 1];
    Coef_[2] = new T[n - 1];
    Coef_[3] = new T[n - 1];

    //храним матрицу коээфициентов 4*n-1, где n-1 - кол-во кусков в сплайне
    // по этой матрице можем однозначно задать кусочно-заданную функцию из кубических полиномов
    for (int j = 0; j < n - 1; ++j) {
        Coef_[0][j] = f_[j]; // a_j
        Coef_[1][j] = delta[j] - h[j] * (T{2} * m[j] + m[j + 1]) / T{6}; // b_j
        Coef_[2][j] = m[j] / T{2}; // c_j
        Coef_[3][j] = (m[j + 1] - m[j]) / (T{6} * h[j]); // d_j
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

template <Field T>
T cubic_spline<T>::evaluate(const T& x) const {
    // Бинарный поиск интервала
    int left = 0;
    int right = n_ - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (x < x_[mid]) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    int i = right;
    if (i < 0) {
        i = 0;
    } else if (i >= n_ - 1) {
        i = n_ - 2;
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
    // решение через метод Томаса, оптимально для трёхдиагональных систем
    // дает O(n) по времени
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