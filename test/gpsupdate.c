#include "../include/linux/gps.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

#define SYS_SET_GPS_LOCATION 398

int main()
{
    double lat, lng;
    long accuracy, temp;
    struct gps_location new_loc;

    printf("Please input new latitude in floating point [-90, +90]: ");
    scanf("%lf", &lat);
    printf("Please input new longitude in floating point [-180, +180]: ");
    scanf("%lf", &lng);
    printf("Please input new accuracy (>= 0): ");
    scanf("%ld", &accuracy);

    new_loc.lat_integer = (int)lat;
    printf("lat_integer = %d\n", new_loc.lat_integer);

    temp = (long)(lat * 1000000);
    if (temp < 0)
        temp *= -1;
    new_loc.lat_fractional = temp % 1000000;
    printf("lat_fractional = %d\n", new_loc.lat_fractional);

    new_loc.lng_integer = (int)lng;
    printf("lng_integer = %d\n", new_loc.lng_integer);

    temp = (long)(lng * 1000000);
    if (temp < 0)
        temp *= -1;
    new_loc.lng_fractional = temp % 1000000;
    printf("lng_fractional = %d\n", new_loc.lng_fractional);

    new_loc.accuracy = accuracy;
    printf("accuracy = %d\n", new_loc.accuracy);

    temp = syscall(SYS_SET_GPS_LOCATION, &new_loc);

    if (temp < 0)
        perror("system call failed : ");

    return 0;
}
