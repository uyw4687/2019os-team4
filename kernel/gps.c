#include <linux/gps.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/rwlock.h>
#include <linux/rwlock_types.h>

struct gps_location curr_loc;

EXPORT_SYMBOL(curr_loc);

DEFINE_RWLOCK(lock);

long sys_set_gps_location(struct gps_location __user *loc)
{
    int err;
    struct gps_location temp_loc;

    if (!loc)
        return -EINVAL;

    if (!access_ok(VERIFY_READ, loc, sizeof(struct gps_location)))
        return -EFAULT;

    err = copy_from_user(&temp_loc, loc, sizeof(struct gps_location));
    if (err != 0) {
        return -EINVAL;
    }

    if (temp_loc.lat_integer < -90 || temp_loc.lat_integer > 90)
        return -EINVAL;

    if (temp_loc.lng_integer < -180 || temp_loc.lng_integer > 180)
        return -EINVAL;

    if (temp_loc.lat_integer == -90 || temp_loc.lat_integer == 90)
        if (temp_loc.lat_fractional != 0)
            return -EINVAL;

    if (temp_loc.lng_integer == -180 || temp_loc.lng_integer == 180)
        if (temp_loc.lng_fractional != 0)
            return -EINVAL;

    if (temp_loc.lat_fractional < 0 || temp_loc.lat_fractional > 999999)
        return -EINVAL;

    if (temp_loc.lng_fractional < 0 || temp_loc.lng_fractional > 999999)
        return -EINVAL;

    if (temp_loc.accuracy < 0)
        return -EINVAL;

    write_lock(&lock);
    curr_loc = temp_loc;
    write_unlock(&lock);

    return 0;
}

long sys_get_gps_location(const char __user *pathname, struct gps_location __user *loc) {
    // TODO
    return 0;
}
