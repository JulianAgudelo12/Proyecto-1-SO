#include <stdio.h>
#include <omp.h>

int main() {
    // Establecer el número de hilos en 1 asegura la ejecución secuencial
    omp_set_num_threads(1);

    int sum = 0;
    int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // Aunque usamos una directiva de OpenMP, esto se ejecutará secuencialmente
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 10; i++) {
        sum += data[i];
        printf("Hilo %d suma %d, sum parcial = %d\n", omp_get_thread_num(), data[i], sum);
    }

    printf("Suma total: %d\n", sum);
    return 0;
}
