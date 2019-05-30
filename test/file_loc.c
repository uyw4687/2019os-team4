#include <stdio.h>
#include <unistd.h>
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

    syscall(SYS_GET_GPS_LOCATION, pathname, &loc);
    printf("latitude    : %d.%d\n", loc.lat_integer, loc.lat_fractional);
    printf("longitude   : %d.%d\n", loc.lng_integer, loc.lng_fractional);
    printf("accuracy    : %d(m)\n", loc.accuracy);
    printf("(LINK) https://www.google.com/maps/search/?api=1&query=%d.%d,%d.%d\n", loc.lat_integer, loc.lat_fractional, loc.lng_integer, loc.lng_fractional);

    return 0;
}
