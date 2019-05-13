#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>

#define SCHED_WRR 7

#define SETWEIGHT 398
#define GETWEIGHT 399

void print()
{
    long long divisor;
    long long dividend;
    long long input;
    clock_t begin, end;

    printf("what to factor? ");
    scanf("%lld", &input);

    //invalid input
    if(input<2){
            printf("wrong");
        return;
    }

    while(1){
            dividend = input;
    printf("%lld = ", dividend);

    begin = clock();
    for(divisor=2;divisor<=dividend;divisor++)
    {
            ;
    }
    printf("\n");
    end = clock();
    printf("time spent(second) : %lf\n", (double)(end-begin)/CLOCKS_PER_SEC);
    }
}

void factor3()
{
    long long divisor;
    long long dividend;
    long long input;
    clock_t begin, end;
    int r, w, i= 0;

    printf("what to factor?");
    scanf("%lld", &input);

    //invalid input
    if(input<2){
            printf("wrong");
        return;
    }

    while(1){
            dividend = input;
            r = i++ % 20 + 1;
            w = syscall(SETWEIGHT, 0,r);
            if(w<0)
                perror("setweight error");
            printf("policy = %d, weight:%d\n", sched_getscheduler(0),r);


    printf("%lld = ", dividend);

    begin = clock();
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
    end = clock();
    printf("time spent(second) : %lf\n", (double)(end-begin)/CLOCKS_PER_SEC);
    fflush(stdout);
    }
}

void factor2()
{
    long long divisor;
    long long dividend;
    long long input;
    clock_t begin, end;

    printf("what to factor?");
    scanf("%lld", &input);

    //invalid input
    if(input<2){
            printf("wrong");
        return;
    }

    while(1){
            dividend = input;
    printf("%lld = ", dividend);

    begin = clock();
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
    end = clock();
    printf("time spent(second) : %lf\n", (double)(end-begin)/CLOCKS_PER_SEC);
    fflush(stdout);
    }
}


void factor(long long num)
{
    long long divisor;
    long long dividend = num;
    clock_t begin, end;

    printf("what to factor?");
    scanf("%lld", &dividend);

    //invalid input
    if(dividend<2)
        return;

    printf("%lld = ", dividend);

    begin = clock();
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
    end = clock();
    printf("time spent(second) : %lf\n", (double)(end-begin)/CLOCKS_PER_SEC);
}

int main()

{
    struct sched_param param;
    cpu_set_t mask;
    int cpu;
    int q, w, e, r, t;
    int i;

    param.sched_priority = sched_get_priority_min(SCHED_WRR);

    if(sched_setscheduler(0, SCHED_WRR, &param) != 0)
    {
        perror("failed");
        return -1;
    }

    printf("end policy = %d\n", sched_getscheduler(0));
    
    while(1) {

            printf("1:sleep\n2: loop from 0 to input value without printing\n3: loop from 0 to input value with printing by the input value increment\n4:setweight\n5:getweight\n6:fork\n7:use only 0, 1, 2 cpu\n8:getscheduler\n9:break and factor\n10:finish program return\n11: clear screen\n12: use only one cpu\n13: cpu setting not zeroing mask value\n14: repeat factor\n15: repeat factor with only one value\n16: from 0 to input value loop and print time continuously\n17: weight change automatically from 1 to 20 and do the factor repeatedly\n");
            scanf("%d", &q);
           switch(q)
           {
            case 1:
                  printf("input amount(sec)\n");
                 scanf("%d",&q);
                sleep(q);
               continue;
            case 2:
              printf("input\n");
             scanf("%d", &q);
            for(i=0;i<q;i++) 
                    ;
            continue;
            case 3:
            printf("input two ex)1 2\n");
             scanf("%d %d", &q, &r);
            for(i=0;i<q;i++) 
                    if(i%r==0)
                            printf("i = %d\n", i);
            continue;

            case 4:
            printf("input pid :  (0 for myself)");
            scanf("%d", &q);
            printf("input weight :  (1~20)");
            scanf("%d", &r);
            r = syscall(SETWEIGHT, q,r);
        if(r<0)
                perror("setweight error");
                printf("policy = %d, weight:%d\n", sched_getscheduler(0),r);

            continue;
            case 5:
            printf("input pid :  (0 for myself)");
            scanf("%d", &q);
            r = syscall(GETWEIGHT, q);
        if(r<0)
                perror("getweight error");
                printf("policy = %d, weight:%d\n", sched_getscheduler(0),r);
            continue;
            case 6:        
    if(fork()){
        r = syscall(GETWEIGHT, 0);
        if(r<0)
                perror("getweight error");
        printf("policy(parent) = %d, weight:%d\n", sched_getscheduler(0),r);
        }
    else{
        r = syscall(GETWEIGHT, 0);
        if(r<0)
                perror("getweight error");
        printf("policy(child) = %d, weight:%d\n", sched_getscheduler(0),r);
        exit(0);
        }
            continue;
            case 7:

    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    CPU_SET(1, &mask);
    CPU_SET(2, &mask);
    
    if(sched_setaffinity(0, sizeof(mask), &mask) < 0)
    {
        perror("setaffinity");
    }
    continue;
            case 8:
            printf("pid : ");
            scanf("%d", &q);
            if(sched_getscheduler(q)<0)
                    perror("getscheduler error");
            else
                printf("scheduler : %d\n", sched_getscheduler(q));
            continue;
            case 9:
    break;
            case 10:
       return 0;
            case 11:
       printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
            continue;
            case 12:

            printf("input cpu to use: 0~3\n");
            scanf("%d", &q);
            if(q>3 || q<0){
                    printf("bad value");
                    continue;
            }
    CPU_ZERO(&mask);
    CPU_SET(q, &mask);
    
    if(sched_setaffinity(0, sizeof(mask), &mask) < 0)
    {
        perror("setaffinity");
    }
    continue;

            case 13:

            printf("input cpu to use: 0~3\n");
            scanf("%d", &q);
            if(q>3 || q<0){
                    printf("bad value\n");
                    continue;

            }
    CPU_SET(q, &mask);
    
    if(sched_setaffinity(0, sizeof(mask), &mask) < 0)
    {
        perror("setaffinity");
    }
    continue;

                    case 14:
                    while(1)
                    {
                        factor(3);
                    }
                    case 15:
                    printf("value to factor continuously\n");
                        factor2();
                        continue;
                    case 16:
                        print();
                        continue;
                    case 17:
                        factor3();
                        continue;
            default:
    continue;
           }
           factor(3);

    }
    return 0;
}
