#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "gps.h"

#define SYS_GET_GPS_LOCATION 399

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("[usage] ./file_loc {path_name}\n");
        exit(-1);
    }

    char pathname[100];
    struct gps_location loc;
    int ret;

    memset(pathname, 0, sizeof(char)*100);
    
    if(strlen(argv[1]) > sizeof(pathname)-1)
        strncpy(pathname, argv[1], sizeof(char)*99);
    else
        strcpy(pathname, argv[1]);

    ret = syscall(SYS_GET_GPS_LOCATION, pathname, &loc);
    if(ret < 0)
    {
        perror("error : ");
        exit(-1);
    }

    printf("latitude    : %d.%d\n", loc.lat_integer, loc.lat_fractional);
    printf("longitude   : %d.%d\n", loc.lng_integer, loc.lng_fractional);
    printf("accuracy    : %d(m)\n", loc.accuracy);
    printf("(LINK) https://www.google.com/maps/search/?api=1&query=%d.%d,%d.%d\n", loc.lat_integer, loc.lat_fractional, loc.lng_integer, loc.lng_fractional);

    return 0;
}
