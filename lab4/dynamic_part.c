#include <stdio.h>
#include <dlfcn.h>

typedef float (*e_t)(int);
typedef float (*area_t)(float, float);

char *paths[] = {"./libmy1.so", "./libmy2.so"};

int main() {
    int ind = 0;
    void *lib = dlopen(paths[ind], RTLD_LAZY);
    if (!lib) {
        printf("Ошибка загрузки библиотеки\n");
        return 1;
    }
    
    e_t e_func = dlsym(lib, "e");
    area_t area_func = dlsym(lib, "area");
    
    if (!e_func || !area_func) {
        printf("Ошибка загрузки функций\n");
        return 1;
    }

    printf("0 - переключить библиотеку\n");
    printf("1 x - вычисление числа e\n");
    printf("2 a b - вычисление площади фигуры\n");
    printf("-1 - выход\n\n");
    
    int choice;
    scanf("%d", &choice);
    while (choice != -1) {
        if (choice == 0) {
            dlclose(lib);
            ind = !ind;
            lib = dlopen(paths[ind], RTLD_LAZY);

            if (!lib) { 
                printf("Ошибка переключения библиотеки\n");
                return 1;
            }

            e_func = dlsym(lib, "e");
            area_func = dlsym(lib, "area");
            
            if (!e_func || !area_func) {
                printf("Ошибка загрузки функций\n");
                return 1;
            }

            printf("Библиотека переключена (Реализация №%d)\n", ind + 1);
        }

        if (choice == 1) {
            int x;
            scanf("%d", &x);
            printf("Результат: %.6f\n", e_func(x));
        }
        if (choice == 2) {
            float a, b;
            scanf("%f %f", &a, &b);
            printf("Результат: %.6f\n", area_func(a, b));
        }
        scanf("%d", &choice);
    }
    dlclose(lib);
    return 0;
}