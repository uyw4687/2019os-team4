#include<stdio.h>

int main(int argc, char **argv) {
    FILE *f;
    char c[100];
    char start[100];

    if(argc == 1) {
        printf("input file name argument");
        return -1;
    }

    f = fopen(argv[1], "r");
    fscanf(f, "%s", start);
    printf("%s\n", start);
    /*int k=1;
    while(k) {
        fscanf(f, "%s", c);
        for(int i=0; i<100; i++){
            if(c[i] != start[i]) break;
            else if(i == 99)
                k = 0;
        }
        printf("%s\n", c);
    }*/
    fclose(f);
    return 1;
}    
