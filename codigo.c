#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

char** listFiles(const char *path, int *fileCount);
void read_csv(const char *filename);

// Leemos los archivos en paralelo
int parallel(char *path, int fileCount) {
    
    // Configuramos la Memoria para crear los procesos   
    STARTUPINFO si;
    PROCESS_INFORMATION *piArray = malloc(sizeof(PROCESS_INFORMATION) * fileCount);
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    // Iniciaamos las variables para el contador
    struct timespec start, end;
    double interval;
    
    // Se establece el numero de nucleos
    omp_set_num_threads(4); // Ajustar según el número deseado de hilos

    clock_gettime(CLOCK_MONOTONIC, &start);

    // Obtenemos todos los nombres de los archivos csv
    char **files = listFiles(path, &fileCount);
    if (files) {
        printf("Archivos encontrados: %d\n", fileCount);
        #pragma omp parallel for
        for (int i = 0; i < fileCount; i++) {
            printf("Procesando archivo: %s\n", files[i]);

            // Creamos la ruta completa
            char cmdLine[256];
            sprintf(cmdLine, "read_csv.exe %s", files[i]); // Asumiendo que read_csv.exe es el ejecutable
            
            ZeroMemory(&piArray[i], sizeof(PROCESS_INFORMATION));
            if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &piArray[i])) {
                fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError());
            }
        }

        // Esperamos a que todos los procesos terminen
        for (int i = 0; i < fileCount; i++) {
            WaitForSingleObject(piArray[i].hProcess, INFINITE);
            printf("Hilo %d, El archivo %s ha terminado.\n", omp_get_thread_num(), files[i]);
            CloseHandle(piArray[i].hProcess);
            CloseHandle(piArray[i].hThread);
        }

        // Liberar los nombres de archivos
        for (int i = 0; i < fileCount; i++) {
            free(files[i]);
        }
        free(files);
    }
    free(piArray);

    clock_gettime(CLOCK_MONOTONIC, &end);
    interval = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Tiempo total de ejecución: %.8f segundos\n", interval);
    return 0;
}


Mi CommConfigDialog

    //Configuramos la Memoria para crear los procesos   
    STARTUPINFO si;
    PROCESS_INFORMATION *piArray = malloc(sizeof(PROCESS_INFORMATION) * fileCount);
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    //Iniciaamos las variables para el contador
    struct timespec start, end;
    double interval;
    
    //Se estaablece el numero de nucleos
    omp_set_num_threads(1);

    clock_gettime(CLOCK_MONOTONIC, &start);

    //obtenemos todos los nombres de los archivos csv
    char **files = listFiles(path, &fileCount);

    // BORRRAR
    //fileCount = 1;

    if (files) {
        printf("Archivos encontrados: %d\n", fileCount);
        #pragma omp parallel for
        for (int i = 0; i < fileCount; i++) {
            printf("Procesando archivo: %s\n", files[i]);

            //Creamos la ruta completa
            char cmdLine[256];
            sprintf(cmdLine, "./read_windows -f %s",files[i]);
        
            if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &piArray[i])) {
                fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError());
            }

            // Espera a que el proceso hijo termine antes de continuar
            printf("Hilo %d, El arcivo: %s se guardo\n", omp_get_thread_num(), files[i]);


        }

        for (int i = 0; i < fileCount; i++) {
            WaitForSingleObject(piArray[i].hProcess, INFINITE);
            printf("Hilo %d, El archivo %s ha terminado.\n", omp_get_thread_num(), files[i]);
            CloseHandle(piArray[i].hProcess);
            CloseHandle(piArray[i].hThread);
        }

        printf("Acabe\n");

       for (int j = 0; j < fileCount; j++) {
            printf("Holi%d\n",j);
            free(files[j]);
        }
        
        free(files);
    }

    //free(piArray);
    printf("Llegue?\n");
    return 0;
    

 // Esperar a todos los procesos
WaitForMultipleObjects(fileCount, handles, TRUE, INFINITE);