#include <dirent.h>
#include <getopt.h>
#include <sched.h>
#include <sys/mman.h>
#include "timeline.h"
#include "essentials.h"

const int MAX_FILES = 100;
const int MAX_LINES = 200000;
const int MAX_LINE_LENGTH = 512;

int total_files = 0;
int successful_reads = 0;
struct timespec (*process_times)[2];
int *position_pt;


int main(int argc, char *argv[]) {
    // Creación de los file descriptors y almacenamiento de datos de los archivos
    char *file_list[MAX_FILES];
    CSVFile csv_files[MAX_FILES];

    int opt;
    int parallel = 0; // Procesamiento paralelo
    int multi = 0;    // Procesamiento multi-core
    int simulation_number = 0;
    char *folder = NULL;

    /*
     * Procesa los argumento de la linea de comando.
     * Es un while que pasa por los argumentos del main, obteniendolos uno a uno (getopt).
     * De esta manera, si el usuario elige tanto -s como -m, entonces el programa se cerrará.
     * */
    while ((opt = getopt(argc, argv, "smf:i:")) != -1) {
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
            case 'i':
                if((simulation_number = atoi(optarg)) == 0){
                    printf("Numero de simulaciones inválido, utilice un número mayor a 0.\n");
                    exit(EXIT_FAILURE);
                }
                printf("Numero de simulaciones: %i\n", simulation_number);
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

    // Allocate shared memory for process_times and position_pt
    process_times = mmap(NULL, sizeof(struct timespec) * total_files * 2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    position_pt = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (process_times == MAP_FAILED || position_pt == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    *position_pt = 0; // Initialize position_pt

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    printf("Hora de inicio del programa: %ld.%09ld\n", start_time.tv_sec, start_time.tv_nsec);


    if (parallel) {
        printf("Procesando archivos en modo paralelo\n");
        process_files_parallel(file_list, csv_files);
    } else if (multi) {
        printf("Procesando archivos en modo multi-core\n");
        process_files_multi_core(file_list, csv_files);
    } else {
        printf("Procesando archivos secuencialmente\n");
        process_files_sequentially(file_list, csv_files);
    }

    // Registrar el tiempo de finalización del programa
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    printf("Hora de carga del último archivo: %ld.%09ld\n", end_time.tv_sec, end_time.tv_nsec);
    printf("Tiempo total de procesamiento: %.2f ms\n", time_diff(start_time, end_time));

    print_timeline(start_time, end_time, process_times, total_files);

    //show_cpu_usage();
    // Liberar la memoria asignada
    for (int i = 0; i < total_files; i++) {
        free(file_list[i]);
    }

    // Unmap the shared memory
    munmap(process_times, sizeof(struct timespec) * 11 * 2);
    munmap(position_pt, sizeof(int));

    // Imprimir el código de salida
    printf("Código de salida: %d\n", successful_reads == total_files ? 0 : 1);
    return successful_reads == total_files ? 0 : 1;
}


