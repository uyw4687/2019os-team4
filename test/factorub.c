#include <stdio.h>
#include <unistd.h>
#include <sched.h>

/* Debug */
#define DEBUG   0
#define DBGVAL  324

/* Select */
#define SELECT  0 // 0 for PRIME32, 1 for PRIME64
//#define PRIME32 10000019
#define PRIME32 179424361
//#define PRIME32 2147483647
#define PRIME64 2305843009213693951

#define SCHED_SETWEIGHT     398
#define SCHED_GETWEIGHT     399

#define SCHED_WRR 7

void factor(long num)
{
    long long divisor;
    long long dividend = num;

    if(DEBUG)
        dividend = DBGVAL;

    //invalid input
    if(dividend<2)
        return;

    printf("%lld = ", dividend);

    for(divisor=2;divisor<=dividend;divisor++)
    {
        while(dividend%divisor==0)
        {
            printf("%lld ", divisor);

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
    long long num;
    int ret;
    
    if(!SELECT)
        num = PRIME32;
    else
        num = PRIME64;
    
    printf("policy : %d\n", sched_getscheduler(0));

    factor(num);

    return 0;
}
