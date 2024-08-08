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

//Metodo principal
int main(){

    char *path = "C:\\Users\\agude\\OneDrive - Universidad EAFIT\\Universidad\\Semestre #6\\Sistemas Operativos\\Proyecto #1\\Archivos\\*";
    int fileCount = 0;

    //Iniciaamos las variables para el contador
    struct timespec start, end;
    double interval;

    //Se estaablece el numero de nucleos
    omp_set_num_threads(8); 

    clock_gettime(CLOCK_MONOTONIC, &start);

    //obtenemos todos los nombres de los archivos csv
    char **files = listFiles(path, &fileCount);

    //Asigna un espacio en memoria para la estructura 
    //CSVFile *file = malloc(sizeof(CSVFile));

   

    if (files) {
        printf("Archivos encontrados: %d\n", fileCount);
        #pragma omp parallel for
        for (int i = 0; i < fileCount; i++) {
            printf("Procesando archivo: %s\n", files[i]);

            //Creamos la ruta completa
            char fullPath[256];
            sprintf(fullPath, "Archivos/%s",files[i]);

            //Leémos el arachivo
            read_csv(fullPath);
            free(files[i]);
        }
        free(files);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    interval = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Tiempo de ejecucion de todo el programa: %.8f segundos\n", interval);

    
    return EXIT_SUCCESS;
}

