// SPDX-License-Identifier: GPL-2.0
/*
 * linux/fs/ext2/namei.c
 *
 * Rewrite to pagecache. Almost all code had been changed, so blame me
 * if the things go wrong. Please, send bug reports to
 * viro@parcelfarce.linux.theplanet.co.uk
 *
 * Stuff here is basically a glue between the VFS and generic UNIXish
 * filesystem that keeps everything in pagecache. All knowledge of the
 * directory layout is in fs/ext2/dir.c - it turned out to be easily separatable
 * and it's easier to debug that way. In principle we might want to
 * generalize that a bit and turn it into a library. Or not.
 *
 * The only non-static object here is ext2_dir_inode_operations.
 *
 * TODO: get rid of kmap() use, add readahead.
 *
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/fs/minix/namei.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  Big-endian to little-endian byte-swapping/bitmaps by
 *        David S. Miller (davem@caip.rutgers.edu), 1995
 */

#include <linux/pagemap.h>
#include <linux/quotaops.h>
#include "ext2.h"
#include "xattr.h"
#include "acl.h"

static inline int ext2_add_nondir(struct dentry *dentry, struct inode *inode)
{
	int err = ext2_add_link(dentry, inode);
#if debug_proj4
    pr_err("ext2_add_nondir");
#endif
	if (!err) {
		d_instantiate_new(dentry, inode);
		return 0;
	}
	inode_dec_link_count(inode);
	unlock_new_inode(inode);
	iput(inode);
	return err;
}

/*
 * Methods themselves.
 */

static struct dentry *ext2_lookup(struct inode * dir, struct dentry *dentry, unsigned int flags)
{
	struct inode * inode;
	ino_t ino;
	
	if (dentry->d_name.len > EXT2_NAME_LEN)
		return ERR_PTR(-ENAMETOOLONG);

	ino = ext2_inode_by_name(dir, &dentry->d_name);
	inode = NULL;
	if (ino) {
		inode = ext2_iget(dir->i_sb, ino);
		if (inode == ERR_PTR(-ESTALE)) {
			ext2_error(dir->i_sb, __func__,
					"deleted inode referenced: %lu",
					(unsigned long) ino);
			return ERR_PTR(-EIO);
		}
	}
	return d_splice_alias(inode, dentry);
}

struct dentry *ext2_get_parent(struct dentry *child)
{
	struct qstr dotdot = QSTR_INIT("..", 2);
	unsigned long ino = ext2_inode_by_name(d_inode(child), &dotdot);
	if (!ino)
		return ERR_PTR(-ENOENT);
	return d_obtain_alias(ext2_iget(child->d_sb, ino));
} 

/*
 * By the time this is called, we already have created
 * the directory cache entry for the new file, but it
 * is so far negative - it has no inode.
 *
 * If the create succeeds, we fill in the inode information
 * with d_instantiate(). 
 */
static int ext2_create (struct inode * dir, struct dentry * dentry, umode_t mode, bool excl)
{
	struct inode *inode;
	int err;
#if debug_proj4
    pr_err("ext2_create");
#endif
	err = dquot_initialize(dir);
	if (err)
		return err;

	inode = ext2_new_inode(dir, mode, &dentry->d_name);
	if (IS_ERR(inode))
		return PTR_ERR(inode);

	inode->i_op = &ext2_file_inode_operations;
	if (test_opt(inode->i_sb, NOBH)) {
		inode->i_mapping->a_ops = &ext2_nobh_aops;
		inode->i_fop = &ext2_file_operations;
	} else {
		inode->i_mapping->a_ops = &ext2_aops;
		inode->i_fop = &ext2_file_operations;
	}
	mark_inode_dirty(inode);
	return ext2_add_nondir(dentry, inode);
}

static int ext2_tmpfile(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	struct inode *inode = ext2_new_inode(dir, mode, NULL);
#if debug_proj4
    pr_err("ext2_tmpfile");
#endif
	if (IS_ERR(inode))
		return PTR_ERR(inode);

	inode->i_op = &ext2_file_inode_operations;
	if (test_opt(inode->i_sb, NOBH)) {
		inode->i_mapping->a_ops = &ext2_nobh_aops;
		inode->i_fop = &ext2_file_operations;
	} else {
		inode->i_mapping->a_ops = &ext2_aops;
		inode->i_fop = &ext2_file_operations;
	}
	mark_inode_dirty(inode);
	d_tmpfile(dentry, inode);
	unlock_new_inode(inode);
	return 0;
}

static int ext2_mknod (struct inode * dir, struct dentry *dentry, umode_t mode, dev_t rdev)
{
	struct inode * inode;
	int err;
#if debug_proj4
    pr_err("ext2_mknod");
#endif
	err = dquot_initialize(dir);
	if (err)
		return err;

	inode = ext2_new_inode (dir, mode, &dentry->d_name);
	err = PTR_ERR(inode);
	if (!IS_ERR(inode)) {
		init_special_inode(inode, inode->i_mode, rdev);
#ifdef CONFIG_EXT2_FS_XATTR
		inode->i_op = &ext2_special_inode_operations;
#endif
		mark_inode_dirty(inode);
		err = ext2_add_nondir(dentry, inode);
	}
	return err;
}

static int ext2_symlink (struct inode * dir, struct dentry * dentry,
	const char * symname)
{
	struct super_block * sb = dir->i_sb;
	int err = -ENAMETOOLONG;
	unsigned l = strlen(symname)+1;
	struct inode * inode;
#if debug_proj4
    pr_err("ext2_symlink");
#endif
	if (l > sb->s_blocksize)
		goto out;

	err = dquot_initialize(dir);
	if (err)
		goto out;

	inode = ext2_new_inode (dir, S_IFLNK | S_IRWXUGO, &dentry->d_name);
	err = PTR_ERR(inode);
	if (IS_ERR(inode))
		goto out;

	if (l > sizeof (EXT2_I(inode)->i_data)) {
		/* slow symlink */
		inode->i_op = &ext2_symlink_inode_operations;
		inode_nohighmem(inode);
		if (test_opt(inode->i_sb, NOBH))
			inode->i_mapping->a_ops = &ext2_nobh_aops;
		else
			inode->i_mapping->a_ops = &ext2_aops;
		err = page_symlink(inode, symname, l);
		if (err)
			goto out_fail;
	} else {
		/* fast symlink */
		inode->i_op = &ext2_fast_symlink_inode_operations;
		inode->i_link = (char*)EXT2_I(inode)->i_data;
		memcpy(inode->i_link, symname, l);
		inode->i_size = l-1;
	}
	mark_inode_dirty(inode);

	err = ext2_add_nondir(dentry, inode);
out:
	return err;

out_fail:
	inode_dec_link_count(inode);
	unlock_new_inode(inode);
	iput (inode);
	goto out;
}

static int ext2_link (struct dentry * old_dentry, struct inode * dir,
	struct dentry *dentry)
{
	struct inode *inode = d_inode(old_dentry);
	int err;
#if debug_proj4
    pr_err("ext2_link");
#endif
	err = dquot_initialize(dir);
	if (err)
		return err;

	inode->i_ctime = current_time(inode);
	inode_inc_link_count(inode);
	ihold(inode);

	err = ext2_add_link(dentry, inode);
	if (!err) {
		d_instantiate(dentry, inode);
		return 0;
	}
	inode_dec_link_count(inode);
	iput(inode);
	return err;
}

static int ext2_mkdir(struct inode * dir, struct dentry * dentry, umode_t mode)
{
	struct inode * inode;
	int err;
#if debug_proj4
    pr_err("ext2_mkdir");
#endif
	err = dquot_initialize(dir);
	if (err)
		return err;

	inode_inc_link_count(dir);

	inode = ext2_new_inode(dir, S_IFDIR | mode, &dentry->d_name);
	err = PTR_ERR(inode);
	if (IS_ERR(inode))
		goto out_dir;

	inode->i_op = &ext2_dir_inode_operations;
	inode->i_fop = &ext2_dir_operations;
	if (test_opt(inode->i_sb, NOBH))
		inode->i_mapping->a_ops = &ext2_nobh_aops;
	else
		inode->i_mapping->a_ops = &ext2_aops;

	inode_inc_link_count(inode);

	err = ext2_make_empty(inode, dir);
	if (err)
		goto out_fail;

	err = ext2_add_link(dentry, inode);
	if (err)
		goto out_fail;

	d_instantiate_new(dentry, inode);
out:
	return err;

out_fail:
	inode_dec_link_count(inode);
	inode_dec_link_count(inode);
	unlock_new_inode(inode);
	iput(inode);
out_dir:
	inode_dec_link_count(dir);
	goto out;
}

static int ext2_unlink(struct inode * dir, struct dentry *dentry)
{
	struct inode * inode = d_inode(dentry);
	struct ext2_dir_entry_2 * de;
	struct page * page;
	int err;
#if debug_proj4
    pr_err("ext2_unlink");
#endif
	err = dquot_initialize(dir);
	if (err)
		goto out;

	de = ext2_find_entry (dir, &dentry->d_name, &page);
	if (!de) {
		err = -ENOENT;
		goto out;
	}

	err = ext2_delete_entry (de, page);
	if (err)
		goto out;

	inode->i_ctime = dir->i_ctime;
	inode_dec_link_count(inode);
	err = 0;
out:
	return err;
}

static int ext2_rmdir (struct inode * dir, struct dentry *dentry)
{
	struct inode * inode = d_inode(dentry);
	int err = -ENOTEMPTY;
#if debug_proj4
    pr_err("ext2_rmdir");
#endif
	if (ext2_empty_dir(inode)) {
		err = ext2_unlink(dir, dentry);
		if (!err) {
			inode->i_size = 0;
			inode_dec_link_count(inode);
			inode_dec_link_count(dir);
		}
	}
	return err;
}

static int ext2_rename (struct inode * old_dir, struct dentry * old_dentry,
			struct inode * new_dir,	struct dentry * new_dentry,
			unsigned int flags)
{
	struct inode * old_inode = d_inode(old_dentry);
	struct inode * new_inode = d_inode(new_dentry);
	struct page * dir_page = NULL;
	struct ext2_dir_entry_2 * dir_de = NULL;
	struct page * old_page;
	struct ext2_dir_entry_2 * old_de;
	int err;
#if debug_proj4
    pr_err("ext2_raname");
#endif
	if (flags & ~RENAME_NOREPLACE)
		return -EINVAL;

	err = dquot_initialize(old_dir);
	if (err)
		goto out;

	err = dquot_initialize(new_dir);
	if (err)
		goto out;

	old_de = ext2_find_entry (old_dir, &old_dentry->d_name, &old_page);
	if (!old_de) {
		err = -ENOENT;
		goto out;
	}

	if (S_ISDIR(old_inode->i_mode)) {
		err = -EIO;
		dir_de = ext2_dotdot(old_inode, &dir_page);
		if (!dir_de)
			goto out_old;
	}

	if (new_inode) {
		struct page *new_page;
		struct ext2_dir_entry_2 *new_de;

		err = -ENOTEMPTY;
		if (dir_de && !ext2_empty_dir (new_inode))
			goto out_dir;

		err = -ENOENT;
		new_de = ext2_find_entry (new_dir, &new_dentry->d_name, &new_page);
		if (!new_de)
			goto out_dir;
		ext2_set_link(new_dir, new_de, new_page, old_inode, 1);
		new_inode->i_ctime = current_time(new_inode);
		if (dir_de)
			drop_nlink(new_inode);
		inode_dec_link_count(new_inode);
	} else {
		err = ext2_add_link(new_dentry, old_inode);
		if (err)
			goto out_dir;
		if (dir_de)
			inode_inc_link_count(new_dir);
	}

	/*
	 * Like most other Unix systems, set the ctime for inodes on a
 	 * rename.
	 */
	old_inode->i_ctime = current_time(old_inode);
	mark_inode_dirty(old_inode);

	ext2_delete_entry (old_de, old_page);

	if (dir_de) {
		if (old_dir != new_dir)
			ext2_set_link(old_inode, dir_de, dir_page, new_dir, 0);
		else {
			kunmap(dir_page);
			put_page(dir_page);
		}
		inode_dec_link_count(old_dir);
	}
	return 0;


out_dir:
	if (dir_de) {
		kunmap(dir_page);
		put_page(dir_page);
	}
out_old:
	kunmap(old_page);
	put_page(old_page);
out:
	return err;
}

/*
 * Compute sin, cos for 32-bit int theta
 *
 * Input
 * theta:    in range of [-pi/2 * (1<<30), pi/2 * (1<<30)]
 * n:        # of iteration (32 is recommended)
 *
 * Output
 * sin, cos: in range of [-1 * (1<<30), 1 * (1<<30)]
*/
void cordic(int theta, int *sin, int *cos, int n)
{
    // int mul = 1073741824;
    // int half_pi = 0x6487ED51;
    int cordic_ctab[] = {
        0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6,
        0x03FEAB76, 0x01FFD55B, 0x00FFFAAA, 0x007FFF55,
        0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF,
        0x0003FFFF, 0x0001FFFF, 0x0000FFFF, 0x00007FFF,
        0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF,
        0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F,
        0x0000003F, 0x0000001F, 0x0000000F, 0x00000008,
        0x00000004, 0x00000002, 0x00000001, 0x00000000, };
    // cordic_ctab[i] is computed by (atan(pow(2, -i)) * mul);

    int cordic_1K = 0x26DD3B6A;
    int k, d, tx, ty, tz;
    int x = cordic_1K, y = 0, z = theta;
    n = (n > 32) ? 32 : n;

    for (k = 0; k < n; k++) {
        d = z >> 31;
        tx = x - (((y >> k) ^ d) - d);
        ty = y + (((x >> k) ^ d) - d);
        tz = z - ((cordic_ctab[k] ^ d) - d);
        x = tx;
        y = ty;
        z = tz;
    }
    *cos = x;
    *sin = y;

    /* usage:
        double theta = 3.1415926535897932384626 / 2;
        int mul = 1073741824;
        int sin, cos;
        cordic(theta * mul, &sin, &cos, 32);
        printf("%f\n", sin / mul);
    */
}

int cordic_arctan(int x, int y)
{
    int cordic_ctab[] = {
        0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6,
        0x03FEAB76, 0x01FFD55B, 0x00FFFAAA, 0x007FFF55,
        0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF,
        0x0003FFFF, 0x0001FFFF, 0x0000FFFF, 0x00007FFF,
        0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF,
        0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F,
        0x0000003F, 0x0000001F, 0x0000000F, 0x00000008,
        0x00000004, 0x00000002, 0x00000001, 0x00000000, };
    // cordic_ctab[i] is computed by (atan(pow(2, -i)) << 30);

    int sum_angle = 0;
    int k = 0;
    int tx, ty;
    for (k = 0; k < 32; k++) {
        if (y > 0) {
            tx = x + (y >> k);
            ty = y - (x >> k);
            sum_angle = sum_angle + cordic_ctab[k];
        }
        else {
            tx = x - (y >> k);
            ty = y + (x >> k);
            sum_angle = sum_angle - cordic_ctab[k];
        }
        x = tx;
        y = ty;
    }

    return sum_angle;
}

/*
 * Compute square root for numbers between 0 and 1<<60
 */
long long cordic_sqrt(long long x)
{
    long long base, y;
    int i;

    base = 1 << 30;
    y = 0;
    
    for (i = 0; i < 30; i++) {
        y += base;
        if ((y * y) > x) {
            y -= base;
        }
        base = base >> 1;
    }
    return y;
}

// Output: rad * (1<<29) in range of [-pi * (1<<29), pi * (1<<29)]
int deg_to_rad(int deg_integer, int deg_fractional)
{
    // 9370165 = pi * ((1<<29) / 180)
    return (deg_integer * 9370165) + ((deg_fractional * 937) / 100);
}

/*
 * Input:   the result of deg_to_rad()
 *
 * Output
 * n_x, n_y, n_z: in range of [-1 * (1<<30), 1 * (1<<30)]
 */

void compute_normal_vector(int lat_radian, int lng_radian,
                          long long *n_x, long long *n_y, long long *n_z)
{
    // compute sin(latitude), cos(latitude), sin(longitude), cos(longitude)
    int sin_lat, cos_lat, sin_lng, cos_lng;
    int sign = 1;

    cordic(lat_radian * 2, &sin_lat, &cos_lat, 32);

    // 843314856 = pi * ((1<<29) / 2)
    // 1686629713 = pi * (1<<29)
    if (lng_radian > 843314856) {       // longitude > pi/2
        lng_radian -= 1686629713;
        sign = -1;
    }
    else if (lng_radian < -843314856) { // longitude < -pi/2
        lng_radian += 1686629713;
        sign = -1;
    }
    cordic(lng_radian * 2, &sin_lng, &cos_lng, 32);
    sin_lng = sin_lng * sign;
    cos_lng = cos_lng * sign;

    /* TODO more precise
     *
     * Assume that there is no overflow,
     *  (cos_lat * cos_lng) / (1<<30)
     * is differ from
     *  (int)(cos_lat / (1<<15)) * (int)(cos_lng / (1<<15))
     */
    // *n_x = (cos_lat / 32768) * (cos_lng / 32768);
    // *n_y = (cos_lat / 32768) * (sin_lng / 32768);
    *n_x = ((long long)cos_lat * (long long)cos_lng) >> 30;
    *n_y = ((long long)cos_lat * (long long)sin_lng) >> 30;
    *n_z = (long long)sin_lat;
}

extern struct gps_location curr_loc;
extern rwlock_t curr_loc_lock;

static int check_distance(struct ext2_inode *inode)
{
    int i_lat_int, i_lat_fr, i_lng_int, i_lng_fr, i_accuracy;
    int i_lat_radian, i_lng_radian;
    long long i_nx, i_ny, i_nz;
    int c_lat_radian, c_lng_radian, c_accuracy;
    long long c_nx, c_ny, c_nz;
    long long dot, cross_x, cross_y, cross_z, cross;
    int central_angle;
    long long allowed_distance;

    i_lat_int = le32_to_cpu(inode->i_lat_integer);
    i_lat_fr = le32_to_cpu(inode->i_lat_fractional);
    i_lng_int = le32_to_cpu(inode->i_lng_integer);
    i_lng_fr = le32_to_cpu(inode->i_lng_fractional);
    i_accuracy = le32_to_cpu(inode->i_accuracy);

    // convert degree to radian (for inode location)
    i_lat_radian = deg_to_rad(i_lat_int, i_lat_fr);
    i_lng_radian = deg_to_rad(i_lng_int, i_lng_fr);

    // compute normal vector (for inode location)
    compute_normal_vector(i_lat_radian, i_lng_radian, &i_nx, &i_ny, &i_nz);

    read_lock(&curr_loc_lock);
    
    // convert degree to radian (for current location)
    c_lat_radian = deg_to_rad(curr_loc.lat_integer, curr_loc.lat_fractional);
    c_lng_radian = deg_to_rad(curr_loc.lng_integer, curr_loc.lng_fractional);
    c_accuracy = curr_loc.accuracy;

    read_unlock(&curr_loc_lock);
    
    // compute normal vector (for current location)
    compute_normal_vector(c_lat_radian, c_lng_radian, &c_nx, &c_ny, &c_nz);

    // compute dot product and cross product
    dot = (i_nx * c_nx + i_ny * c_ny + i_nz * c_nz) >> 30;
    cross_x = (i_ny * c_nz - i_nz * c_ny) >> 30;
    cross_y = (i_nz * c_nx - i_nx * c_nz) >> 30;
    cross_z = (i_nx * c_ny - i_ny * c_nx) >> 30;
    cross = cordic_sqrt(cross_x * cross_x + cross_y * cross_y + cross_z * cross_z);

    /*
    if (dot == 0) {
        // 1005309649 = 6400 * (pi / 2) * 100000
        distance = 1005309649;
    }
    else if (dot == 1 && (cross > 1>>30 || cross < -(1>>30))) {
        // no difference with above case
        distance = 1005309649;
    }
    else {
        // now ((cross << 30) / dot) is in range of [-1 * (1<<30), 1 * (1<<30)]
        cordic_arctan((int)dot, (int)cross);
    }
    */
    central_angle = cordic_arctan((int)dot, (int)cross);

    allowed_distance = ((long long)i_accuracy + (long long)c_accuracy) << 30;
#if debug_proj4
    pr_err("check distance.\nfile location : %d.%d, %d.%d, accuracy : %d\npresent location : %d.%d, %d.%d, accuracy : %d\ndistance is %lld, allowed_distance is %lld",i_lat_int, i_lat_fr, i_lng_int, i_lng_fr, i_accuracy, curr_loc.lat_integer, curr_loc.lat_fractional, curr_loc.lng_integer, curr_loc.lng_fractional, c_accuracy, ((long long)(central_angle)*6400000) >> 30, allowed_distance >> 30);
#endif
    if ((long long)central_angle <= allowed_distance / 6400000)
        return 1;   // true
    else
        return 0;   // false
}

extern struct ext2_inode *ext2_get_inode(struct super_block *sb, ino_t ino, struct buffer_head **p);

struct ext2_inode *get_ext2_inode(struct inode *inode) {
    struct buffer_head *bh;
    return ext2_get_inode(inode->i_sb, inode->i_ino, &bh);
}

int ext2_permission(struct inode *inode, int mask)
{
	int ret = generic_permission(inode, mask);
    struct ext2_inode *i = get_ext2_inode(inode);

    if (ret < 0)
        return ret;

    if (!check_distance(i))
        return -EACCES;
#if debug_proj4
    pr_err("check permission. ino = %lu, lat = %d.%d, lng = %d.%d, accuracy = %d",inode->i_ino, i->i_lat_integer, i->i_lat_fractional, i->i_lng_integer, i->i_lng_fractional, i->i_accuracy);
#endif
    return 0;
}

const struct inode_operations ext2_dir_inode_operations = {
	.create		= ext2_create,
	.lookup		= ext2_lookup,
	.link		= ext2_link,
	.unlink		= ext2_unlink,
	.symlink	= ext2_symlink,
	.mkdir		= ext2_mkdir,
	.rmdir		= ext2_rmdir,
	.mknod		= ext2_mknod,
	.rename		= ext2_rename,
#ifdef CONFIG_EXT2_FS_XATTR
	.listxattr	= ext2_listxattr,
#endif
	.setattr	= ext2_setattr,
	.get_acl	= ext2_get_acl,
	.set_acl	= ext2_set_acl,
	.tmpfile	= ext2_tmpfile,
};

const struct inode_operations ext2_special_inode_operations = {
#ifdef CONFIG_EXT2_FS_XATTR
	.listxattr	= ext2_listxattr,
#endif
	.setattr	= ext2_setattr,
	.get_acl	= ext2_get_acl,
	.set_acl	= ext2_set_acl,
};
