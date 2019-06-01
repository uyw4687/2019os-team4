#include<stdio.h>

int main(int argc, char **argv){
    FILE *f;
    char arr[100];
    
    if(argc == 1) {
        printf("input file name argument");
        return -1;
    }
    printf("%s\n", argv[1]);
    f = fopen(argv[1], "w");

    printf("input contents : ");
    scanf("%s", arr);
    
    fprintf(f, "%s\n", arr);
    fclose(f);
}
