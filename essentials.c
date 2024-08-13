#include "essentials.h"

double time_diff(struct timespec start, struct timespec end) {
    double start_ms = start.tv_sec * 1000.0 + start.tv_nsec / 1000000.0;
    double end_ms = end.tv_sec * 1000.0 + end.tv_nsec / 1000000.0;
    return end_ms - start_ms;
}

void read_csv(const char *filename, CSVFile *csv_file) {
    struct timespec start_time, end_time;
    // Obtener el tiempo de inicio
    clock_gettime(CLOCK_MONOTONIC, &start_time);

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

    // Obtener el tiempo de finalización
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    // Creación de array para almacenamiento
    process_times[*position_pt][0] = start_time;
    process_times[*position_pt][1] = end_time;

    (*position_pt)++;

    //Limpieza de la memoria
    for (int i = 0; i < csv_file->line_count; i++) {
        free(csv_file->lines[i]);
    }

    free(csv_file->lines);

    // Calcular y mostrar el tiempo de ejecución
    double elapsed_time = time_diff(start_time, end_time);
    printf("Tiempo de ejecución para leer %s: %.2f ms\n", filename, elapsed_time);
}

void process_files_sequentially(char* file_list[MAX_FILES], CSVFile csv_files[MAX_FILES]) {
    // Establecer afinidad del proceso principal al núcleo 0
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset); // Establecer afinidad al núcleo 0
    if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpuset) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
    show_cpu_affinity(getpid()); // Mostrar afinidad del proceso principal

    for (int i = 0; i < total_files; i++) {
        read_csv(file_list[i], &csv_files[i]);
        show_cpu_affinity(getpid()); // Mostrar afinidad del proceso principal
    }
}

// Procesamiento paralelo de archivos usando fork con afinidad a un solo núcleo
void process_files_parallel(char* file_list[MAX_FILES], CSVFile csv_files[MAX_FILES]) {
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

// Procesamiento multi-core de archivos usando fork con afinidad a núcleos específicos
void process_files_multi_core(char* file_list[MAX_FILES], CSVFile csv_files[MAX_FILES]) {
    pid_t pids[MAX_FILES];
    cpu_set_t cpuset;
    int p_number = sysconf(_SC_NPROCESSORS_ONLN);
    struct sched_param param;
    param.sched_priority = 50;

    for (int i = 0; i < total_files; i++) {

        if ((pids[i] = fork()) == 0) {
            CPU_ZERO(&cpuset);
            CPU_SET(i % p_number, &cpuset); // Assign the core cyclically
            if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpuset) == -1) {
                perror("sched_setaffinity");
                exit(EXIT_FAILURE);
            }

            if(sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1)
            {
                perror("sched_setscheduler failed");
            }
            else{
                printf("Process %d set to SCHED_FIFO with priority %d\n", getpid(), param.sched_priority);
            }

            //int policy = sched_getscheduler(getpid());
            //print_scheduler(policy);
            show_cpu_affinity(getpid()); // Show affinity of the child process
            read_csv(file_list[i], &csv_files[i]);
            _exit(EXIT_SUCCESS);
        } else if (pids[i] > 0) {
            // Parent process: Continue
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < total_files; i++) {
        waitpid(pids[i], NULL, 0);
    }

}