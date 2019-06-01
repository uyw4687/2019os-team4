#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/rwlock.h>
#include <linux/rwlock_types.h>
#include <linux/fs.h>
#include <linux/namei.h>

struct gps_location curr_loc;
DEFINE_RWLOCK(curr_loc_lock);

EXPORT_SYMBOL(curr_loc);
EXPORT_SYMBOL(curr_loc_lock);

long sys_set_gps_location(struct gps_location __user *loc)
{
    int ret;
    struct gps_location temp_loc;

    if (!access_ok(VERIFY_READ, loc, sizeof(struct gps_location)))
        return -EFAULT;

    ret = copy_from_user(&temp_loc, loc, sizeof(struct gps_location));
    if (ret != 0)
        return -EFAULT;

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

    write_lock(&curr_loc_lock);
    curr_loc = temp_loc;
    write_unlock(&curr_loc_lock);

    return 0;
}

long sys_get_gps_location(const char __user *pathname, struct gps_location __user *loc) {

    int ret;
    char path_kern[100];
    struct inode *inode;
    struct path path;
    struct gps_location loc_kern;

    if (!access_ok(VERIFY_READ, pathname, sizeof(char)*100))
        return -EFAULT;

    if (!access_ok(VERIFY_WRITE, loc, sizeof(struct gps_location)))
        return -EFAULT;

    ret = copy_from_user(path_kern, pathname, sizeof(char)*100);
    if (ret != 0)
        return -EFAULT;

    kern_path(path_kern, LOOKUP_FOLLOW, &path);
    inode = path.dentry->d_inode;

    ret = generic_permission(inode, MAY_READ);

    if(ret < 0)
        return -EACCES;

    if(inode->i_op->get_gps_location)
        ret = inode->i_op->get_gps_location(inode, &loc_kern);
    else
        return -ENODEV;

    if(ret < 0)
        return ret;
    
    ret = copy_to_user(loc, &loc_kern, sizeof(struct gps_location));
    if (ret != 0) 
        return -EFAULT;
    
    return 0;
}
