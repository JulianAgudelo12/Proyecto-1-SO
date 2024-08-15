#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

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

}


int main(int argc, char *argv[]) {
    char *filename = NULL;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) { 
                filename = argv[i + 1];
                i++;
            } 
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "Usage: %s -f filename\n", argv[0]);
        return EXIT_FAILURE;
    }

    //printf("Read: %s\n", filename);

    //Creamos la ruta completa
    char fullPath[256];
    sprintf(fullPath, "Archivos/%s",filename);

    read_csv(fullPath);
    return 0;
}