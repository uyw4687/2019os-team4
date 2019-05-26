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
    int accuracy, temp, err;
    struct gps_location new_loc;

    printf("Please input new latitude in floating point [-90, +90]: ");
    scanf("%lf", &lat);
    printf("Please input new longitude in floating point [-180, +180]: ");
    scanf("%lf", &lng);
    printf("Please input new accuracy (>= 0): ");
    scanf("%d", &accuracy);

    new_loc.lat_integer = (int)lat;
    printf("lat_integer = %d\n", new_loc.lat_integer);

    temp = (int)(lat * 1000000);
    if (temp < 0) temp *= -1;
    new_loc.lat_fractional = temp % 1000000;
    printf("lat_fractional = %d\n", new_loc.lat_fractional);

    new_loc.lng_integer = (int)lng;
    printf("lng_integer = %d\n", new_loc.lng_integer);
    
    temp = (int)(lng * 1000000);
    if (temp < 0) temp *= -1;
    new_loc.lng_fractional = temp % 1000000;
    printf("lng_fractional = %d\n", new_loc.lng_fractional);

    new_loc.accuracy = accuracy;
    printf("accuracy = %d\n", new_loc.accuracy);

    temp = syscall(SYS_SET_GPS_LOCATION, &new_loc);
    err = errno;

    if (temp == -1) {
        if (err == EINVAL) {
            perror("Invalid argument");
            return -1;
        }
        else if (err == EFAULT) {
            perror("Bad address");
            return -1;
        }
        else {
            perror("Unknown error occured");
            return -1;
        }
    }

    return 0;
}
