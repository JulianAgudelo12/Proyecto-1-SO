#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char *filename = NULL;
    int i;

    if (argc == 1) {
        fprintf(stderr, "Usage: %s -f filename\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s -f filename\n", argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                filename = argv[i + 1]; // Take the next argument as filename
                i++; // Move past the filename
            } else { // No filename was provided
                fprintf(stderr, "Option -f requires a filename.\n");
                return EXIT_FAILURE;
            }
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "Usage: %s -f filename\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Filename: %s\n", filename);
    return 0;
}
