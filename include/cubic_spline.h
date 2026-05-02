#pragma once

#include <concepts>
#include <stdexcept>

/// Концепт упорядоченного поля.
template <typename T>
concept Field = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
    { a - b } -> std::same_as<T>;
    { a * b } -> std::same_as<T>;
    { a / b } -> std::same_as<T>;
    { -a }   -> std::same_as<T>;
    { a < b } -> std::convertible_to<bool>;
    T{0};
    T{1};
} && std::totally_ordered<T>;

/**
 * @brief Кубический сплайн-интерполятор для любого упорядоченного поля T.
 *
 * @tparam T тип, удовлетворяющий концепту Field
 */
template <Field T>
class cubic_spline {
public:
    cubic_spline();
    ~cubic_spline();

    /**
     * @brief Построить сплайн по массивам узлов.
     *
     * @param x массив x-координат (размер n, строго возрастающие)
     * @param f массив значений функции (размер n)
     * @param n количество узлов (n >= 3)
     */
    void build(const T* x, const T* f, int n);

    /**
     * @brief Вычислить значение сплайна в точке x.
     *
     * @param x точка интерполяции
     * @return значение S(x)
     */
    T evaluate(const T& x) const;

private:
    int n_;          // количество узлов
    T* x_;           // копии x-координат
    T* f_;           // копии значений функции
    T** Coef_;       // матрица коэффициентов: 4 строки (a, b, c, d)

    /**
     * @brief Решение трёхдиагональной системы методом прогонки.
     *
     * @param TDM матрица 3×n (индексы: 0 – поддиагональ, 1 – главная, 2 – наддиагональ)
     * @param F   правая часть
     * @param b   выходной вектор (вторые производные)
     */
    void solve_tridiag(T** TDM, T* F, T* b);
};

#include "detail/cubic_spline.tpp"