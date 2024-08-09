#include "timeline.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

void print_timeline(struct timespec start, struct timespec end, struct timespec times[files_amount][2]){
    double global_start = start.tv_sec * 1000.0 + start.tv_nsec / 1000000.0;
    double global_end = end.tv_sec * 1000.0 + end.tv_nsec / 1000000.0;
    double difference = global_end - global_start;
    int slice = difference/TIME_SLICES;

    //printf("Global Start: %f\n", global_start);
    //printf("Global end: %f\n", global_end);
    //printf("Time difference: %f\n", difference);
    printf("Size of Slice : %i \n", slice);
    printf("\n");
    for(int k = 0; k < files_amount; k++)
    {
        printf("Process %i ", k);
        struct timespec p_struct_start = times[k][0];
        struct timespec p_struct_end = times[k][1];
        double p_start = p_struct_start.tv_sec* 1000.0 + p_struct_start.tv_nsec / 1000000.0;
        double p_end = p_struct_end.tv_sec * 1000.0 + p_struct_end.tv_nsec / 1000000.0;
        double dif_start = p_start - global_start;
        int dif_start_time_slices = floor(dif_start/slice);
        printf("%*c", dif_start_time_slices, ' ');
        int dif_time_slices = floor((p_end-p_start)/slice);
        for(int j = 0; j < dif_time_slices; j++){ printf("▓");}
        printf("\n");

    }
}

//Test Git