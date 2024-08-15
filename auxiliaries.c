#include "auxiliaries.h"

void clear_cache() {
    int result = system("sudo sh -c \"echo 3 > /proc/sys/vm/drop_caches\"");

    if (result == -1) {
        perror("Error executing the system command");
    } else {
        printf("Cache cleared successfully.\n");
    }
}

void print_scheduler(int policy)
{
    switch (policy) {
        case SCHED_OTHER:
            printf("Scheduling policy is SCHED_OTHER (default)\n");
            break;
        case SCHED_FIFO:
            printf("Scheduling policy is SCHED_FIFO (first-in, first-out)\n");
            break;
        case SCHED_RR:
            printf("Scheduling policy is SCHED_RR (round-robin)\n");
            break;
        case SCHED_BATCH:
            printf("Scheduling policy is SCHED_BATCH (for batch processes)\n");
            break;
        case SCHED_IDLE:
            printf("Scheduling policy is SCHED_IDLE (for very low priority background tasks)\n");
            break;
        case SCHED_DEADLINE:
            printf("Scheduling policy is SCHED_DEADLINE (real-time tasks with deadlines)\n");
            break;
        default:
            printf("Unknown scheduling policy\n");
    }
}

void show_cpu_affinity(pid_t pid) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    if (sched_getaffinity(pid, sizeof(cpu_set_t), &cpuset) == -1) {
        perror("sched_getaffinity");
        return;
    }

    printf("Proceso %d afinidad a CPUs: ", pid);
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &cpuset)) {
            printf("%d ", i);
        }
    }
    printf("\n");
}