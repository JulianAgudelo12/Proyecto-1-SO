#define _GNU_SOURCE
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include "auxiliaries.h"


#ifndef CODE_READ_FUNCTIONS_H
#define CODE_READ_FUNCTIONS_H

typedef struct {
    char **lines;
    int line_count;
} CSVFile;

extern const int MAX_FILES;
extern const int MAX_LINES;
extern const int MAX_LINE_LENGTH;
extern int total_files;
extern struct timespec (*process_times)[2];
extern int *position_pt;

extern int successful_reads;

double time_diff(struct timespec start, struct timespec end);
void read_csv(const char *filename, CSVFile *csv_file);
void process_files_sequentially(char* file_list[MAX_FILES], CSVFile csv_files[MAX_FILES]);
void process_files_parallel(char* file_list[MAX_FILES], CSVFile csv_files[MAX_FILES]);
void process_files_multi_core(char* file_list[MAX_FILES], CSVFile csv_files[MAX_FILES]);

#endif //CODE_READ_FUNCTIONS_H
