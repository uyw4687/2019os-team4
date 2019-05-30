#include <stdio.h>
#include <unistd.h>
#include "gps.h"

#define SYS_GET_GPS_LOCATION 399

int main()
{
    char pathname[100];
    struct gps_location loc;

    syscall(SYS_GET_GPS_LOCATION, pathname, &loc);
    printf("lat_integer    : %d\n", loc.lat_integer);
    printf("lat_fractional : %d\n", loc.lat_fractional);
    printf("lng_integer    : %d\n", loc.lng_integer);
    printf("lng_fractional : %d\n", loc.lng_fractional);
    printf("accuracy       : %d\n", loc.accuracy);

    return 0;
}
