#include "timeline.h"
#include <stdio.h>
#include <time.h>
#include <math.h>


#define MS_IN_SEC 1000.0
#define NS_IN_MS 1000000.0

void print_timeline(struct timespec start, struct timespec end, struct timespec times[][2], int files_amount){
    // Convertir los tiempos de inicio y fin globales a milisegundos
    double global_start = start.tv_sec * MS_IN_SEC + start.tv_nsec / NS_IN_MS;
    double global_end = end.tv_sec * MS_IN_SEC + end.tv_nsec / NS_IN_MS;
    double total_time = global_end - global_start;
    if (files_amount <= 0) {
        printf("Error: files_amount debe ser mayor que 0\n");
        return;
    }

    // Calcular el tamaño de cada segmento de tiempo
    int slice = total_time / TIME_SLICES;
    if (slice <= 0) {
        printf("Error: TIME_SLICES es demasiado grande o la diferencia de tiempo es demasiado pequeña\n");
        return;
    }

    printf("Tamaño del Segmento : %i ms\n", slice);

    for(int k = 0; k < files_amount; k++) {
        printf("Proceso %i ", k);
        struct timespec process_start_time = times[k][0];
        struct timespec process_end_time = times[k][1];

        // Convertir los tiempos de inicio y fin del proceso a milisegundos
        double process_start = process_start_time.tv_sec * MS_IN_SEC + process_start_time.tv_nsec / NS_IN_MS;
        double process_end = process_end_time.tv_sec * MS_IN_SEC + process_end_time.tv_nsec / NS_IN_MS;

        // Calcular el número de espacios antes de la barra y la longitud de la barra
        double process_offset = process_start - global_start;
        int start_offset_slices = floor(process_offset / slice);
        double process_duration = process_end - process_start;
        int process_duration_slices = floor(process_duration / slice);

        // Imprimir espacios antes de la barra
        printf("%*c", start_offset_slices, ' ');

        // Imprimir la barra que representa la duración del proceso
        for(int j = 0; j < process_duration_slices; j++) {
            printf("▓");
        }

        printf(" %.2f ms\n", process_duration);
    }
}


//Test Git