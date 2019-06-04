#include "../include/linux/gps.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

#define SYS_SET_GPS_LOCATION 398

int main(int argc, char **argv)
{
    double lat, lng;
    long accuracy, temp;
    struct gps_location new_loc;

    if(argc == 2) {
    switch(argv[1][0]) {
        case '3':
            printf("Engineerig building 301 selected\n");
            new_loc.lat_integer = 37;
            new_loc.lat_fractional = 449722;
            new_loc.lng_integer = 126;
            new_loc.lng_fractional = 952222;
            break;
        case 'm':
            printf("SNU main gate selected\n");
            new_loc.lat_integer = 37;
            new_loc.lat_fractional = 466111;
            new_loc.lng_integer = 126;
            new_loc.lng_fractional = 948333;
            break;
        case 'b':
            printf("Busan station selected\n");
            new_loc.lat_integer = 35;
            new_loc.lat_fractional = 115000;
            new_loc.lng_integer = 129;
            new_loc.lng_fractional = 422222;
            break;
        case 'e':
            printf("Eiffel tower selected\n");
            new_loc.lat_integer = 48;
            new_loc.lat_fractional = 858056;
            new_loc.lng_integer = 2;
            new_loc.lng_fractional = 294444;
            break;
        default:
            goto non_argument;
    }
    printf("please input new accuracy (>=0): ");
    scanf("%ld", &accuracy);
    
    new_loc.accuracy = accuracy;
    }
    else{
non_argument:
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
    }
    temp = syscall(SYS_SET_GPS_LOCATION, &new_loc);

    if (temp < 0)
        perror("system call failed : ");

    return 0;
}
