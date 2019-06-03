# os-team4
OS Spring Team4
## Project 4

### How to build our kernel
* Same as before.

### High-level design and implementation
#### Inserting location info
* Inserted `i_lat_integer`, `i_lat_fractional`, `i_lng_integer`, `i_lng_fractional`, `i_accuracy` info as `__le32` format into `struct ext2_inode` in fs/ext2/ext2.h
* Inserted `i_lat_integer`, `i_lat_fractional`, `i_lng_integer`, `i_lng_fractional`, `i_accuracy` info as `__u32` format into `struct ext2_inode_info` in fs/ext2/ext2.h

#### Implementing `set_gps_location`/`get_gps_location`
* First, add two elements in `struct inode_operations` in include/linux/fs.h
  * `int (*set_gps_location)(struct inode *)`
  * `int (*get_gps_location)(struct inode *, struct gps_location *)`
* Then, match two corresponding functions(`ext2_set_gps_location`/`ext2_get_gps_location`) in `ext2_file_inode_operations` in fs/ext2/file.c
* Function prototypes of the two functions are in fs/ext2/ext2.h
* Functions are defined in fs/ext2/inode.c

##### `ext2_set_gps_location`
* First find corresponding `ext2_inode` and `ext2_inode_info` using `inode` parameter
* Update the location information after `read_lock` since it's reading current location information
* Use `cpu_to_le32` when updating `ext2_inode` information since it will be stored in disk
* `read_unlock` after finishing the update

##### `ext2_get_gps_location`
* First find corresponding `ext2_inode` using `inode` parameter
* Get the location information using `le32_to_cpu` since the information is stored in disk

#### Change location info(on modifying `ctime`/`mtime`)
* `ext2_update_time` in fs/ext2/inode.c 
* Function prototypes of the two functions are in fs/ext2/ext2.h
* If `ctime` or `mtime` is going to be modified, location info is also modified.

#### Access control
* 파일의 inode에 있는 location info와 `sys_set_gps_location`에서 설정한 현재 location info를 비교하여, 현재 위치가 파일을 수정한 위치와 충분히 가까운 경우에만 파일에 접근할 수 있도록 합니다.
* 물론 위치와 상관 없이 일반적으로 permission이 허용되지 않는 경우에는 파일에 접근할 수 없습니다. 일반적인 permission이 허용된 경우에만 위치 정보를 확인합니다.
* fs/ext2/namei.c의 `ext2_permission`에 정의되어 있습니다.

##### `check_distance`
* 현재 location info와 인자 `inode`로 주어진 파일의 location info 사이의 거리를 구해서, 이것이 두 location info의 `accuracy`의 합보다 작으면 1(true)을, 아니면 0(false)을 반환합니다.
* 두 위치의 위도와 경도를 이용하여 거리를 구할 때, 각 위치의 구 위에서의 법선벡터를 구한 다음 두 법선벡터 사이의 중심각의 크기를 구해 거리를 계산합니다.
* 지구 반지름은 6,400,000m로 가정하였습니다.
* 커널 안에서는 floating point operation이 제공되지 않기 때문에, 대부분의 연산에서 원래 float 값에 2^30(`1<<30`)만큼 곱해 int로 변환하여 계산합니다. 즉, 이진법으로 소숫점 아래 29자리(십진법으로 소숫점 아래 8자리)까지 유효숫자가 유지됩니다.
* 커널 안에서는 math.h의 각종 수학 함수가 제공되지 않습니다. 계산 과정에서 필요한 sin, cos, atan2, sqrt를 계산하기 위해 [CORDIC](https://en.wikipedia.org/wiki/CORDIC)이라는 알고리즘을 사용하였습니다. 이 알고리즘을 사용하면 shift, 덧셈, 뺄셈, 곱셈 연산만 사용하여, 삼각함수의 계산값을 이진법으로 소숫점 아래 30자리(십진법으로 소숫점 아래 9자리)까지 정확하게 계산해 낼 수 있습니다.

###### Testing `check_distance`
* **TODO**

---

> 다음은 kernel/gps.c에 대한 설명입니다.

#### `sys_set_gps_location`
* Check if the user location pointer as parameter is valid
* Copy the information and if the information is in the valid range, update the current location information.
* Else, return proper errnos

#### `sys_get_gps_location`
* Check if the pathname and user location pointer as parameters are valid
* Copy the pathname and if the information is valid and if reading the file is possible for the current user, copy the location information to the pointer given as a parameter
* Else, return proper errnos

#### global values
<pre><code>struct gps_location curr_loc;       // stores current location info
DEFINE_RWLOCK(curr_loc_lock);       // read-write lock for curr_loc
</code></pre>
---

> 다음은 test 프로그램에 대한 설명입니다.

#### gpsupdate
* Get latitude, longitude and accuracy info as input.
* Divide integer part and remaining fractional part.
* Call `SYS_GET_GPS_LOCATION` with an updated `gps_location` struct information
* On failure, print a detailed error message.

#### file_loc
* get a path as command line argument
* Then call `SYS_GET_GPS_LOCATION` with the pathname and get location info through the pointer given as a parameter
* On failure, print a detailed error message.
* Otherwise, print the location info and google maps URL.

### Lessons learned

