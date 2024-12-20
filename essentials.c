#include "essentials.h"

double time_diff(struct timespec start, struct timespec end) {
    double start_ms = start.tv_sec * 1000.0 + start.tv_nsec / 1000000.0;
    double end_ms = end.tv_sec * 1000.0 + end.tv_nsec / 1000000.0;
    return end_ms - start_ms;
}

void print_table_header() {
    printf("+---------------------------+-----------------------------+\n");
    printf("| Metric                    | Value                       |\n");
    printf("+---------------------------+-----------------------------+\n");
}

void print_table_row(const char* metric, const char* value) {
    printf("| %-25s | %-27s |\n", metric, value);
    printf("+---------------------------+-----------------------------+\n");
}

void read_csv(const char *filename, CSVFile *csv_file) {
    struct rusage usage_start, usage_end;
    struct timespec start_time, end_time;

    getrusage(RUSAGE_SELF, &usage_start);
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

    if (csv_file->lines == NULL) {
        fprintf(stderr, "Error allocating memory for csv_file->lines\n");
        exit(EXIT_FAILURE);
    }

    // Leer y almacenar cada línea del archivo
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        csv_file->lines[csv_file->line_count] = strdup(line);
        csv_file->line_count++;
    }

    getrusage(RUSAGE_SELF, &usage_end);
    fclose(file);

    // Indicar que la lectura fue exitosa
    successful_reads++;

    // Obtener el tiempo de finalización
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // Creación de array para almacenamiento
    process_times[*position_pt][0] = start_time;
    process_times[*position_pt][1] = end_time;
    (*position_pt)++;

    // Limpieza de la memoria
    for (int i = 0; i < csv_file->line_count; i++) {
        free(csv_file->lines[i]);
    }
    free(csv_file->lines);

    // Calcular y mostrar el tiempo de ejecución total
    double elapsed_time = time_diff(start_time, end_time);
    printf("Tiempo de ejecución para leer %s: %f ms ------------------------------------------------------------------------------------------\n", filename, elapsed_time);


    // Calcular el tiempo de usuario y sistema utilizados
    double user_time_used = ((usage_end.ru_utime.tv_sec - usage_start.ru_utime.tv_sec) +
                             (usage_end.ru_utime.tv_usec - usage_start.ru_utime.tv_usec) / 1e6) * 1000;

    double sys_time_used = ((usage_end.ru_stime.tv_sec - usage_start.ru_stime.tv_sec) +
                            (usage_end.ru_stime.tv_usec - usage_start.ru_stime.tv_usec) / 1e6) * 1000;

    printf("Tiempo de CPU (usuario): %f ms\n", user_time_used);
    printf("Tiempo de CPU (sistema): %f ms\n", sys_time_used);


    // Imprimir información adicional
    printf("Maximum Resident Set Size: %ld KB\n", usage_end.ru_maxrss);
    printf("Page faults (soft): %ld\n", usage_end.ru_minflt);
    printf("Page faults (hard): %ld\n", usage_end.ru_majflt);
    printf("I/O input operations: %ld\n", usage_end.ru_inblock);
    printf("I/O output operations: %ld\n", usage_end.ru_oublock);
    printf("Swaps: %ld\n", usage_end.ru_nswap);
}


void process_files_sequentially(char* file_list[MAX_FILES], CSVFile csv_files[MAX_FILES])
{
    struct sched_param param;
    param.sched_priority = 50;
    // Creamos la lista de PIDs en función de la cantidad de procesos (Un proceso por cada archivo)
    pid_t pid;
    // Creación de la representación de las CPUs
    cpu_set_t cpuset;
    // Limpieza en caso de alguna fuga de memoria
    CPU_ZERO(&cpuset);
    // Establecemos la afinidad a un solo núcleo
    CPU_SET(1, &cpuset);

    for (int i = 0; i < total_files; i++) {
        //Si el resultado de esta operación es 0, entonces es un proceso hijo, es decir, usable.
        if((pid = fork()) == 0){
            if(sched_setaffinity(getpid(),sizeof(cpu_set_t), &cpuset) == -1){
                perror("sched_setaffinity");
                exit(EXIT_FAILURE);
            }
            //show_cpu_affinity(getpid());

            if((sched_setscheduler(getpid(),SCHED_FIFO, &param)) == -1){
                perror("sched_setscheduler");
            }
            read_csv(file_list[i], &csv_files[i]);
            exit(0);
        }
        else if(pid > 0)
        {
            // Wait for the child process to complete before continuing
            waitpid(pid, NULL, 0);
        }
        else
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
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
            //show_cpu_affinity(getpid()); // Mostrar afinidad del proceso hijo
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
                perror("sched_setscheduler falló");
            }

            //int policy = sched_getscheduler(getpid());x
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