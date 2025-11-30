#include <stdio.h>
#include "lib.h"

int main() {
    int choice;
    printf("1 x - вычисление числа e при заданном x\n");
    printf("2 a b - вычисление площади фигуры\n");
    printf("0 - выход\n\n");
    
    scanf("%d", &choice);
    while (choice) {
        if (choice == 1) {
            int x;
            scanf("%d", &x);
            printf("Результат: %.6f\n", e(x));
        }
        if (choice == 2) {
            float a, b;
            scanf("%f %f", &a, &b);
            printf("Результат: %.6f\n", area(a, b));
        }
        scanf("%d", &choice);
    }
    return 0;
}