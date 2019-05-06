#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sched.h>
#include <unistd.h>

#define SCHED_WRR 7

#define DEBUG1 1 // 1 : WRR, 0 : RR
#define DEBUG2 1
#define DEBUG3 1 // setscheduler
#define DEBUG4 1 // fork
#define DEBUG5 1 // setaffinity with argument & infinite loop

int main(
#if DEBUG5
        int argc, char **argv
#endif
)

{
    struct sched_param param;
    cpu_set_t mask;
    int cpu;

    int scheduler = DEBUG1 ? SCHED_WRR : SCHED_RR;

#if DEBUG2
    printf("start policy = %d\n", sched_getscheduler(0));
#endif
    param.sched_priority = sched_get_priority_min(scheduler);
#if DEBUG2
    printf("max priority = %d, min priority = %d, my priority = %d\n", sched_get_priority_max(scheduler), sched_get_priority_min(scheduler), param.sched_priority);
#endif

#if DEBUG3
    if(sched_setscheduler(0, scheduler, &param) != 0)
    {
        perror("failed");
        return -1;
    }
#endif

    printf("end policy = %d\n", sched_getscheduler(0));

#if DEBUG4
    if(fork())
        printf("end policy(child) = %d\n", sched_getscheduler(0));
    else
        printf("end policy(parent) = %d\n", sched_getscheduler(0));
#endif

#if DEBUG5
    CPU_ZERO(&mask);
    CPU_SET(atoi(argv[1]), &mask);
    
    if(sched_setaffinity(0, sizeof(mask), &mask) < 0)
    {
        perror("setaffinity");
        return -1;
    }
    while(1)
        ;
#endif
    return 0;
}
