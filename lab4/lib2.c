#include "lib.h"

// Функция для вычисления факториала
static int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// Реализация №2: Сумма ряда 1/n!
float e(int x) {
    if (x < 0) return 0;
    
    float sum = 0;
    for (int n = 0; n <= x; n++) {
        sum += 1.0f / factorial(n);
    }
    return sum;
}

// Реализация №2: Площадь прямоугольного треугольника
float area(float a, float b) {
    if (a <= 0 || b <= 0) return 0;
    return a * b / 2.0f;
}