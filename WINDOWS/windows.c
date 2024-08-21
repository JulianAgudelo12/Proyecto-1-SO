#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <omp.h>

#define MAX_LINES 2000
#define MAX_LINE_LENGTH 256

// Se define la estructura de un arreglo de punteros a char y un contador 
typedef struct {
    char **lines; 
    int line_count;
} CSVFile;

//Lectura de archivos csv
int read_csv(char *filename){

    //Asigna un espacio en memoria para la estructura 
    CSVFile *file = malloc(sizeof(CSVFile));

    //Iniciaamos las variables para el contador
    struct timespec start, end;
    double interval;

    clock_gettime(CLOCK_MONOTONIC, &start);
    //Abrimos el archivo en modo lectura
    FILE *data = fopen(filename, "r");

    //Comprobamos que se haya abierto de forma correcta 
    if (data == NULL) {
        perror("Error al abrir el archivo");
        return EXIT_FAILURE;
    }
    //Asigna memoria para la lista de punteros
    file -> lines = malloc(MAX_LINES * sizeof(char *));
    file -> line_count = 0;

    //Lee el archivo linea por linea 
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, data) && file->line_count < MAX_LINES){
        line[strcspn(line, "\n")] = 0;
        //Guardamos cada linea en la estructura file
        file ->lines[file->line_count] = strdup(line);

        //Manejo de errores al guardar la informacion
        if (file->lines[file->line_count] == NULL){
            perror("Error al duplicar linea");
            //liberamos memoria
            for (int i = 0; i < file->line_count; i++) {
                free(file->lines[i]);
            }
            free(file->lines);
            free(file);
            fclose(data);
            return EXIT_FAILURE;
        }
        file -> line_count++;

    }
    
    fclose(data);

   /* 
    // Para demostración: Imprime todas las líneas leídas
    for (int i = 0; i < file->line_count; i++) {
        printf("%s\n", file->lines[i]);
    }

    // Liberar la memoria asignada
    for (int i = 0; i < file->line_count; i++) {
        free(file->lines[i]);
    }
    free(file->lines);
    free(file);
    */
    clock_gettime(CLOCK_MONOTONIC, &end);
    interval = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Hilo %d, El arcivo: %s se guardo\n", omp_get_thread_num(), filename);
    printf("Tiempo de ejecucion: %.8f segundos\n", interval);

}

// Obtenemos el numero fisico de cores
int get_physical_core_count() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);

    DWORD length = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    BOOL done = FALSE;
    int physicalProcessorCount = 0;

    while (!done) {
        DWORD returned = 0;
        BOOL success = GetLogicalProcessorInformation(buffer, &length);

        if (!success) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                if (buffer) free(buffer);
                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(length);

                if (!buffer) {
                    perror("Error: Allocation failure");
                    return 0;
                }
            } else {
                perror("Error: GetLogicalProcessorInformation");
                return 0;
            }
        } else {
            done = TRUE;
        }

        if (done) {
            DWORD byteOffset = 0;
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;

            while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= length) {
                if (ptr->Relationship == RelationProcessorCore) {
                    physicalProcessorCount++;
                }
                byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
                ptr++;
            }
        }
    }

    if (buffer) free(buffer);
    return physicalProcessorCount;
}

//Recolecta los nombres de todos los archivos csv
char** listFiles(const char *path, int *fileCount) {
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(path, &findData);
    char **fileNames = NULL;
    int count = 0;
    int capacity = 11;
    
    //Asigno espacio a la lista de nombres 
    fileNames = malloc(capacity * sizeof(char*));
        

    do {
        // Verifica si el elemento encontrado no es un directorio
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            // Agregamos el nombre a la lista
            fileNames[count] = malloc(strlen(findData.cFileName) + 1);
            strcpy(fileNames[count], findData.cFileName);
            count++;
            //printf("%s\n", findData.cFileName);
        }
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);

    *fileCount = count;
    return fileNames;
}

// Leemos los archivos de forma secuencial 
int sequential(char *path, int fileCount){

    //Configuramos la Memoria para crear los procesos   
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    //Se estaablece el numero de nucleos
    omp_set_num_threads(1);

    //obtenemos todos los nombres de los archivos csv
    char **files = listFiles(path, &fileCount);

    if (files) {
  
        #pragma omp parallel for
        for (int i = 0; i < fileCount; i++) {

            //Creamos la ruta completa
            char cmdLine[256];
            sprintf(cmdLine, "./read_windows -f %s",files[i]);
        
            if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError());
            } else {
                printf("Hilo %d, Proceso creado: %s, PID: %lu\n", omp_get_thread_num(), files[i], pi.dwProcessId);
            }

            // Espera a que el proceso hijo termine antes de continuar
            WaitForSingleObject(pi.hProcess, INFINITE);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            free(files[i]);
        }
        free(files);
    }    
    return 0;
}

// Leemos los archivos en paralelo
int parallel(char *path, int fileCount){

    // Creamos una lista de Handles
    HANDLE *handles = malloc(fileCount * sizeof(HANDLE));
    
    //Se estaablece el numero de nucleos
    omp_set_num_threads(1);

    //obtenemos todos los nombres de los archivos csv
    char **files = listFiles(path, &fileCount);

    if (files) {

        #pragma omp parallel for
        for (int i = 0; i < fileCount; i++) {
            //Configuramos la Memoria para crear los procesos   
            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            //Creamos la ruta completa
            char cmdLine[256];
            sprintf(cmdLine, "./read_windows -f %s",files[i]);
        
            if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError());
            } else {
                printf("Hilo %d, Proceso creado: %s, PID: %lu\n", omp_get_thread_num(), files[i], pi.dwProcessId);
                handles[i] = pi.hProcess;  
            }

            free(files[i]);
        }

        WaitForMultipleObjects(fileCount, handles, TRUE, INFINITE);

        free(files);

    }
    return 0;
}

// Leemos los archivos en Multi-CCore
int multi_core(char *path, int fileCount){
     
    //Configuramos la Memoria para crear los procesos   
    STARTUPINFO *siArray = malloc(fileCount * sizeof(STARTUPINFO));
    if (!siArray) {
        fprintf(stderr, "Failed to allocate memory for STARTUPINFO array.\n");
        return 1;
    }
    PROCESS_INFORMATION *piArray = malloc(fileCount * sizeof(PROCESS_INFORMATION));
    if (!piArray) {
        fprintf(stderr, "Failed to allocate memory for PROCESS_INFORMATION array.\n");
        return 1;
    }

    // Inicializar las estructuras de STARTUPINFO y PROCESS_INFORMATION
    for (int i = 0; i < fileCount; i++) {
        ZeroMemory(&siArray[i], sizeof(STARTUPINFO));
        siArray[i].cb = sizeof(STARTUPINFO);
        ZeroMemory(&piArray[i], sizeof(PROCESS_INFORMATION));
    }

    //Iniciaamos las variables para el contador
    struct timespec start, end;
    double interval;
    
    //Se estaablece el numero de nucleos
    int physical_cores = get_physical_core_count();
    omp_set_num_threads(physical_cores);

    clock_gettime(CLOCK_MONOTONIC, &start);

    //obtenemos todos los nombres de los archivos csv
    char **files = listFiles(path, &fileCount);

    if (files) {

        #pragma omp parallel for
        for (int i = 0; i < fileCount; i++) {
            printf("Procesando archivo: %s\n", files[i]);

            //Creamos la ruta completa
            char cmdLine[256];
            sprintf(cmdLine, "./read_windows -f %s",files[i]);

            //Creamos el proceso 
            if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &siArray[i], &piArray[i])) {
                fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError());
            } else {
                printf("Hilo %d, Proceso creado: %s, PID: %lu\n", omp_get_thread_num(), files[i], piArray[i].dwProcessId);
            }

            //free(files[i]);
        }

        // Esperar que todos los procesos terminen (opcional)
        for (int i = 0; i < fileCount; i++) {
            WaitForSingleObject(piArray[i].hProcess, INFINITE);
            CloseHandle(piArray[i].hProcess);
            CloseHandle(piArray[i].hThread);
        }

        free(files);
    }    
    return 0;
}

//Metodo principal
int main(int argc, char *argv[]){

    char *path = "../Archivos/*";
    int fileCount = 0;

    //Iniciaamos las variables para el contador
    struct timespec start, end;
    double interval;

    // Iniciamos el cronometro de todo el codigo
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (argc == 1) {
        printf("Secuencial\n");
        sequential(path,fileCount);
    } else if (strcmp(argv[1], "-s") == 0){
        printf("Paralelo\n");
        parallel(path,fileCount);
    } else if (strcmp(argv[1], "-m") == 0){
        printf("Multi-core\n");
        multi_core(path,fileCount);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    interval = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Tiempo de ejecucion de todo el programa: %.8f segundos\n", interval);
        
    return EXIT_SUCCESS;
}

