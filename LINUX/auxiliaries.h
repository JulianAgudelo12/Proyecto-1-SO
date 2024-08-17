#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef CODE_AUXILIARIES_H
#define CODE_AUXILIARIES_H

void clear_cache();
void print_scheduler(int policy);
void show_cpu_affinity(pid_t pid);


#endif //CODE_AUXILIARIES_H
