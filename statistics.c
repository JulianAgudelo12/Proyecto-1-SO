#include "statistics.h"



void print_report(double *mean_values, int total_f)
{
    double sum = 0.0;
    double sum_of_squares = 0.0;
    double min_value = mean_values[0];
    double max_value = mean_values[0];
    int min_index = 0;
    int max_index = 0;

    // Calcular suma, suma de cuadrados, y encontrar min/máx con índices
    for (int i = 0; i < total_f; i++)
    {
        double value = mean_values[i];
        sum += value;
        sum_of_squares += value * value;

        if (value < min_value)
        {
            min_value = value;
            min_index = i;
        }
        if (value > max_value)
        {
            max_value = value;
            max_index = i;
        }
    }

    // Calcular promedio
    double overall_mean = sum / total_f;

    // Calcular varianza
    double variance = (sum_of_squares / total_f) - (overall_mean * overall_mean);

    // Calcular desviación estándar
    double stddev = sqrt(variance);

    // Imprimir el reporte
    printf("Reporte de Tiempos de Procesamiento de Archivos:\n");
    printf("====================================================\n");
    printf("Número de Archivos             : %d\n", total_f);
    printf("Suma Total de Tiempos Promedio : %.4f segundos\n", sum);
    printf("Tiempo Promedio General        : %.4f segundos\n", overall_mean);
    printf("Varianza de los Tiempos        : %.4f\n", variance);
    printf("Desviación Estándar de los Tiempos: %.4f\n", stddev);
    printf("----------------------------------------------------\n");
    printf("Archivo con el Tiempo Promedio Mínimo: %s (%.4f segundos)\n", file_list[min_index], min_value);
    printf("Archivo con el Tiempo Promedio Máximo: %s (%.4f segundos)\n", file_list[max_index], max_value);
    printf("----------------------------------------------------\n");
    printf("Detalles por Archivo:\n");
    printf("----------------------------------------------------\n");

    for (int i = 0; i < total_files; i++)
    {
        printf("Archivo: %-20s | Tiempo Promedio: %.4f segundos\n", file_list[i], mean_values[i]);
    }

    printf("====================================================\n");
}


void simulate_cases(int number_of_simulations, char* file_list[MAX_FILES], CSVFile csv_files[MAX_FILES], void (*f)(char**, CSVFile*))
{
    double mean_values[total_files];

    for (int j = 0; j < total_files; j++) {
        mean_values[j] = 0.0;
    }

    for(int k = 0; k < number_of_simulations; k++)
    {
        clear_cache();
        (*f)(file_list, csv_files);

        for(int j = 0; j < total_files; j++)
        {
            mean_values[j] = mean_values[j] + time_diff(process_times[j][0], process_times[j][1]);
        }
        *position_pt = 0;
    }

    for (int i = 0; i < total_files; ++i) {
        mean_values[i] = mean_values[i]/number_of_simulations;
    }
    print_report(mean_values, total_files);
}