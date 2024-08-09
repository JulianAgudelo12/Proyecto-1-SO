#ifndef TIMELINE
#define TIMELINE
#include <bits/types/struct_timespec.h>
#include <stdio.h>

#define TIME_SLICES 70
#define files_amount 11

void print_timeline(struct timespec start, struct timespec end, struct timespec times[files_amount][2]);

#endif
