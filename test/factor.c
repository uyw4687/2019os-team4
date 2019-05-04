#include <stdio.h>

/* Debug */
#define DEBUG  0
#define DBGVAL 324

/* Select */
#define SELECT  0 // 0 for PRIME32, 1 for PRIME64
#define PRIME32 2147483647
#define PRIME64 2305843009213693951

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
    
    if(!SELECT)
        num = PRIME32;
    else
        num = PRIME64;
    
    factor(num);

    return 0;
}
