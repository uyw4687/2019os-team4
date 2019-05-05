#include <stdio.h>
#include <unistd.h>
#include <sched.h>

/* Debug */
#define DEBUG   0
#define DBGVAL  324

/* Select */
#define SELECT  0 // 0 for PRIME32, 1 for PRIME64
#define PRIME32 2147483647
#define PRIME64 2305843009213693951

#define SCHED_WRR           7
#define SCHED_SETWEIGHT     398
#define SCHED_GETWEIGHT     399

void factor(long num)
{
    long divisor;
    long dividend = num;

    if(DEBUG)
        dividend = DBGVAL;

    //invalid input
    if(dividend<2)
        return;

    printf("%ld = ", dividend);

    for(divisor=2;divisor<=dividend;divisor++)
    {
        while(dividend%divisor==0)
        {
            printf("%ld ", divisor);

            dividend /= divisor;

            if(dividend==1)
                break;

            printf("* ");
        }
    }
    printf("\n");
}

int main()
{
    long i;
    long num;
    int ret;
    
    if(!SELECT)
        num = PRIME32;
    else
        num = PRIME64;
    
    struct sched_param param;

    param.sched_priority = sched_get_priority_min(SCHED_WRR);

    ret = sched_setscheduler(0, SCHED_WRR, &param);
    if(ret < 0)
    {
        perror("sched_setscheduler failed");
        return -1;
    }

    ret = syscall(SCHED_SETWEIGHT, 0, 5);
    if(ret < 0)
    {
        perror("sched_setweight failed\n");
        return -1;
    }

    factor(num);

    ret = syscall(SCHED_GETWEIGHT, 0);
    if(ret < 0)
    {
        perror("sched_getweight failed\n");
        return -1;
    }

    return 0;
}
