#include "lib.h"
#include <math.h>

// Реализация №1: (1 + 1/x)^x
float e(int x) {
    if (x <= 0) return 0;
    return powf(1.0f + 1.0f / x, x);
}

// Реализация №1: Площадь прямоугольника
float area(float a, float b) {
    if (a <= 0 || b <= 0) return 0;
    return a * b;
}