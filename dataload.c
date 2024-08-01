#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <getopt.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <sys/resource.h> // Para getrusage

#define MAX_FILES 100
#define MAX_LINES 2000
#define MAX_LINE_LENGTH 256

typedef struct {
    char **lines;
    int line_count;
} CSVFile;

int total_files = 0;
char *file_list[MAX_FILES];
CSVFile csv_files[MAX_FILES];
int successful_reads = 0;

// Función para leer archivos CSV y almacenar el contenido en memoria
void read_csv(const char *filename, CSVFile *csv_file) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error al abrir el archivo %s\n", filename);
        exit(EXIT_FAILURE);
    }
    // Asignar memoria para las líneas del archivo
    csv_file->lines = malloc(MAX_LINES * sizeof(char *));
    csv_file->line_count = 0;

    // Leer y almacenar cada línea del archivo
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        csv_file->lines[csv_file->line_count] = strdup(line);
        csv_file->line_count++;
    }
    fclose(file);

    // Indicar que la lectura fue exitosa
    successful_reads++;
}

// Función para mostrar la afinidad de la CPU de un proceso
void show_cpu_affinity(pid_t pid) {
    cpu_set_t cpuset;
    //CPU_ZERO(&cpuset);

    if (sched_getaffinity(pid, sizeof(cpu_set_t), &cpuset) == -1) {
        perror("sched_getaffinity");
        return;
    }

    printf("Proceso %d afinidad a CPUs: ", pid);
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &cpuset)) {
            printf("%d ", i);
        }
    }
    printf("\n");
}

// Procesamiento secuencial de archivos
void process_files_sequentially() {
    for (int i = 0; i < total_files; i++) {
        read_csv(file_list[i], &csv_files[i]);
    }
}

// Procesamiento paralelo de archivos usando fork con afinidad a un solo núcleo
void process_files_parallel() {
    pid_t pids[MAX_FILES];
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset); // Establecer afinidad al núcleo 0

    for (int i = 0; i < total_files; i++) {
        if ((pids[i] = fork()) == 0) {
            // Proceso hijo: establecer afinidad y mostrarla
            if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpuset) == -1) {
                perror("sched_setaffinity");
                exit(EXIT_FAILURE);
            }
            show_cpu_affinity(getpid()); // Mostrar afinidad del proceso hijo
            read_csv(file_list[i], &csv_files[i]);
            exit(0); // Asegurarse de que el proceso hijo termine después de procesar
        } else if (pids[i] > 0) {
            // Proceso padre: continuar
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < total_files; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

// Procesamiento multi-core de archivos usando fork
void process_files_multi_core() {
    pid_t pids[MAX_FILES];
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (int cpu = 0; cpu < CPU_SETSIZE; cpu++) {
        CPU_SET(cpu, &cpuset); // Permitir el uso de todos los núcleos
    }

    for (int i = 0; i < total_files; i++) {
        if ((pids[i] = fork()) == 0) {
            // Proceso hijo: establecer afinidad y mostrarla
            if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpuset) == -1) {
                perror("sched_setaffinity");
                exit(EXIT_FAILURE);
            }
            show_cpu_affinity(getpid()); // Mostrar afinidad del proceso hijo
            read_csv(file_list[i], &csv_files[i]);
            exit(0); // Asegurarse de que el proceso hijo termine después de procesar
        } else if (pids[i] > 0) {
            // Proceso padre: continuar
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < total_files; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

// Función para calcular la diferencia de tiempo en milisegundos
double time_diff(struct timespec start, struct timespec end) {
    double start_ms = start.tv_sec * 1000.0 + start.tv_nsec / 1000000.0;
    double end_ms = end.tv_sec * 1000.0 + end.tv_nsec / 1000000.0;
    return end_ms - start_ms;
}

// Función para medir y mostrar el uso de CPU
void show_cpu_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Uso de CPU: %ld.%06ld segundos de usuario, %ld.%06ld segundos de sistema\n",
           usage.ru_utime.tv_sec, usage.ru_utime.tv_usec,
           usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
}

int main(int argc, char *argv[]) {
    int opt;
    int parallel = 0; // Procesamiento paralelo
    int multi = 0;    // Procesamiento multi-core
    char *folder = NULL;

    /*
     * Procesa los argumento de la linea de comando.
     * Es un while que pasa por los argumentos del main, obteniendolos uno a uno (getopt).
     * De esta manera, si el usuario elige tanto -s como -m, entonces el programa se cerrará.
     * */
    while ((opt = getopt(argc, argv, "smf:")) != -1) {
        switch (opt) {
            case 's':
                if (multi) {
                    fprintf(stderr, "Las opciones -s y -m son mutuamente excluyentes\n");
                    exit(EXIT_FAILURE);
                }
                parallel = 1;
                break;
            case 'm':
                if (parallel) {
                    fprintf(stderr, "Las opciones -s y -m son mutuamente excluyentes\n");
                    exit(EXIT_FAILURE);
                }
                multi = 1;
                break;
            case 'f':
                folder = optarg;
                break;
            default:
                fprintf(stderr, "Uso: %s [-s] [-m] -f carpeta\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (folder == NULL) {
        fprintf(stderr, "Carpeta no especificada\n");
        exit(EXIT_FAILURE);
    }

    // Abrir el directorio especificado
    struct dirent *entry;
    DIR *dp = opendir(folder);
    if (dp == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // Leer las entradas del directorio y almacenar las rutas completas de los archivos CSV
    while ((entry = readdir(dp))) {
        if (strstr(entry->d_name, ".csv")) {
            file_list[total_files] = malloc(strlen(folder) + strlen(entry->d_name) + 2);
            sprintf(file_list[total_files], "%s/%s", folder, entry->d_name);
            total_files++;
        }
    }
    closedir(dp);

    if (total_files == 0) {
        fprintf(stderr, "No se encontraron archivos CSV\n");
        exit(EXIT_FAILURE);
    }

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    printf("Hora de inicio del programa: %ld.%09ld\n", start_time.tv_sec, start_time.tv_nsec);

    // Procesar los archivos según el modo especificado
    if (parallel) {
        printf("Procesando archivos en modo paralelo\n");
        process_files_parallel();
    } else if (multi) {
        printf("Procesando archivos en modo multi-core\n");
        process_files_multi_core();
    } else {
        printf("Procesando archivos secuencialmente\n");
        process_files_sequentially();
    }

    // Registrar el tiempo de finalización del programa
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    printf("Hora de carga del último archivo: %ld.%09ld\n", end_time.tv_sec, end_time.tv_nsec);
    printf("Tiempo total de procesamiento: %.2f ms\n", time_diff(start_time, end_time));

    show_cpu_usage();
    // Liberar la memoria asignada
    for (int i = 0; i < total_files; i++) {
        for (int j = 0; j < csv_files[i].line_count; j++) {
            free(csv_files[i].lines[j]);
        }
        free(csv_files[i].lines);
        free(file_list[i]);
    }

    // Imprimir el código de salida
    printf("Código de salida: %d\n", successful_reads == total_files ? 0 : 1);
    return successful_reads == total_files ? 0 : 1;
}