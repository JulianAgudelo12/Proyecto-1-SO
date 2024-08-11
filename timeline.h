#ifndef TIMELINE
#define TIMELINE
#include <bits/types/struct_timespec.h>
#include <stdio.h>

#define TIME_SLICES 70

void print_timeline(struct timespec start, struct timespec end, struct timespec times[][2], int files_amount);
#endif
