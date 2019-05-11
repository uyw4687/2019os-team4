#include<stdio.h>

int main()
{
        int i=0;
    while(1)
    {
            if(i++ == 2147483647){
                printf("hi\n");
                i=0;
            }
    }
    return 0;
}
