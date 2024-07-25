#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

int main() {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    char *token;
    char *arr[10000];

    // Abrir el archivo CSV
    file = fopen("Archivos/18.01.11_BR_videos.csv", "r");
    if (file == NULL) {
        perror("No se pudo abrir el archivo");
        return EXIT_FAILURE;
    }

    // Leer cada línea del archivo
    while (fgets(line, sizeof(line), file)) {
        printf("%s",line);
        // Eliminar el salto de línea al final de la línea
       // line[strcspn(line, "\n")] = '\0';

        // Separar los valores usando la coma como delimitador
        token = strtok(line, ",");
        int i = 0;

        while (token != NULL && i < 11){
            arr[i] = token;
            i = i + 1;
        }

        //printf("%s", token); // Imprimir cada valor separado por tabulaciones
        
        while (token != NULL) {
            arr[i] = token;
            token = strtok(NULL, ",");
            i = i+1;
        }

        /*for (int j = 0; j < 11; j++) {
        printf("%s ", arr[j]);
        }*/

        //printf("\n"); // Nueva línea después de cada fila
    }

    // Cerrar el archivo
    fclose(file);
    return EXIT_SUCCESS;
}
