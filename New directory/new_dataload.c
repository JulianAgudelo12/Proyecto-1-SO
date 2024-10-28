#include <dirent.h>
#include <getopt.h>
#include <sched.h>
#include <sys/mman.h>
#include "timeline.h"
#include "essentials.h"
#include "statistics.h"

int total_files = 0;
int successful_reads = 0;
struct timespec (*process_times)[2];
int *position_pt;
char *file_list[MAX_FILES];

int main(int argc, char *argv[]) {
    clear_cache();

    // Creación de los file descriptors y almacenamiento de datos de los archivos
    CSVFile csv_files[MAX_FILES];

    int opt;
    int parallel = 0; // Procesamiento paralelo
    int multi = 0;    // Procesamiento multi-core
    int simulation_number = 0;
    void (*f)(char**, CSVFile*) = NULL;
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
                if(multi == 1){f = &process_files_multi_core;}
                else if(parallel == 1){f = &process_files_parallel;}
                else {f = &process_files_sequentially;}
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

    *position_pt = 0;

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    if(simulation_number > 0){
        simulate_cases(simulation_number,file_list, csv_files, f);
    }
    else if (parallel) {
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

    //show_cpu_usage();
    // Liberar la memoria asignada
    for (int i = 0; i < total_files; i++) {
        free(file_list[i]);
    }

    printf("Hora de inicio del programa: %ld.%09ld\n", start_time.tv_sec, start_time.tv_nsec);
    printf("Hora de inicio de la carga del primer archivo: %ld.%09ld\n", process_times[0][0].tv_sec, process_times[0][0].tv_nsec);
    printf("Hora de finalización de la carga del último archivo: %ld.%09ld\n", end_time.tv_sec, end_time.tv_nsec);
    if(simulation_number == 0)
    {
        print_timeline(start_time, end_time, process_times, total_files);

    }
    printf("Tiempo total de procesamiento: %02ld:%02ld\n", (end_time.tv_sec - start_time.tv_sec) / 60, (end_time.tv_sec - start_time.tv_sec) % 60);


    // Unmap the shared memory
    munmap(process_times, sizeof(struct timespec) * 11 * 2);
    munmap(position_pt, sizeof(int));

    long total_time_ms = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                         (end_time.tv_nsec - start_time.tv_nsec) / 1000000;

    printf("Tiempo total de procesamiento: %ld ms\n", total_time_ms);

    return successful_reads == total_files ? 0 : 1;
}