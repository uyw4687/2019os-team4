# os-team4
OS Spring Team4
## Project 4

### How to build our kernel
* Same as before.

### High-level design and implementation
#### Inserting location info
* Inserted i_lat_integer, i_lat_fractional, i_lng_integer, i_lng_fractional, accuracy info as `__le32` format into struct ext2_inode in tizen/fs/ext2/ext2.h
* Inserted i_lat_integer, i_lat_fractional, i_lng_integer, i_lng_fractional, accuracy info as `__u32` format into struct ext2_inode_info in tizen/fs/ext2/ext2.h

#### Implementing set_gps_location/get_gps_location
* First, add two elements in struct inode_operations in tizen/include/linux/fs.h
  * `int (*set_gps_location)(struct inode *)`
  * `int (*get_gps_location)(struct inode *, struct gps_location *)`
* Then, match two corresponding functions(ext2_set_gps_location/ext2_get_gps_location) in `ext2_file_inode_operations` in tizen/fs/ext2/file.c
* Function prototypes of the two functions are in tizen/fs/ext2/ext2.h
* Functions are defined in tizen/fs/ext2/inode.c
##### ext2_set_gps_location
* First find corresponding ext2_inode and ext2_inode_info using inode parameter
* Update the location information after read_lock since it's reading current location information
* use cpu_to_le32 when updating ext2_inode information since it will be stored in disk
* read_unlock after finishing the update

#### ext2_get_gps_location
* First find corresponding ext2_inode using inode parameter
* Get the location information using le32_to_cpu since the information is stored in disk

#### Change location info(on modifying ctime/mtime)
* `ext2_update_time` in tizen/fs/ext2/inode.c 
* Function prototypes of the two functions are in tizen/fs/ext2/ext2.h
* If ctime or mtime is going to be modified, location info is also modified.

---

> 다음은 kernel/gps.c에 대한 설명입니다.

#### sys_set_gps_location
* Check if the user location pointer as parameter is valid
* copy the information and if the information is in the valid range, update the current location information.
* else, return proper errnos

#### sys_get_gps_location
* Check if the pathname and user location pointer as parameters are valid
* copy the pathname and if the information is valid and if reading the file is possible for the current user, copy the location information to the pointer given as a parameter
* else, return proper errnos

#### global values
<pre><code>
struct gps_location curr_loc        // stores current location info
</code></pre>
#### helper functions
* 
---

> 다음은 test 프로그램에 대한 설명입니다.

#### gpsupdate
* Get latitude, longitude and accuracy info as input.
* Divide integer part and remaining fractional part.
* call SYS_GET_GPS_LOCATION with an updated gps_location struct information
* On failure, print a detailed error message.

#### file_loc
* get a path as command line argument
* then call SYS_GET_GPS_LOCATION with the pathname and get location info through the pointer given as a parameter
* On failure, print a detailed error message.
* Otherwise, print the location info and google maps URL.

### Lessons learned

