#include "essentials.h"
#include "constants.h"
#include <math.h>
#include <stdio.h>

extern char *file_list[MAX_FILES];

#ifndef CODE_STATISTIC_FUNCTIONS_H
#define CODE_STATISTIC_FUNCTIONS_H

typedef struct {
    const char *filename;
    double total_time;    // Total processing time for the file across all attempts
    double avg_time_per_attempt;  // Average processing time per attempt
    double min_time;  // Minimum time taken for the file to be loaded
    double max_time;  // Maximum time taken for the file to be loaded
} FileReport;



void print_report(double *mean_values, int total_f);
void simulate_cases(int number_of_simulations, char* file_list[MAX_FILES], CSVFile csv_files[MAX_FILES], void (*f)(char**, CSVFile*));
#endif //CODE_STATISTIC_FUNCTIONS_H
