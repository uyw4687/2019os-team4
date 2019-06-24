1. 점수 : 75/75
2. 감점 사유:
* None! Congratulation :)
3. 통계:
* 평균 : 70.33
* 표준편차 : 4.54
* Q1 (하위 25%) : 70
* Q2 (중간값) : 71.5
* Q3 (상위 25%) : 73

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

#### proj4.fs
* test 과정을 통해서 만든 파일들이 저장되어있는 파일시스템입니다.
* 모든 파일들은 'Hello, proj4!'라는 문구가 저장되어있습니다.
* test를 하면서 `301_1`, `301_1000`, `301_2000`, `301_200000`, `301_500000`, `301_5000000`, `301_1000000`, `maingate_1`, `maingate_1000`, `busan_1`, `busan_300000`, `eiffel_1`, `eiffel_5000000` 이라는 파일을 만들었습니다.
* 각 파일명의 앞부분은 위치를 가리킵니다. (301은 301동 공학관, maingate는 서울대학교 정문, busan은 부산역, eiffel은 프랑스 에펠탑을 가리킵니다.)
* 각 파일명의 뒷부분은 accuracy를 가리킵니다.
* 각 파일의 file_loc 값은 다음과 같습니다.
  <pre>root:~> ./file_loc ./proj4/301_1
  latitude    : 37.449722
  longitude   : 126.952222
  accuracy    : 1(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=37.449722,126.952222
  root:~> ./file_loc ./proj4/301_1000
  latitude    : 37.449722
  longitude   : 126.952222
  accuracy    : 1000(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=37.449722,126.952222
  root:~> ./file_loc ./proj4/301_10000000
  latitude    : 37.449722
  longitude   : 126.952222
  accuracy    : 10000000(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=37.449722,126.952222
  root:~> ./file_loc ./proj4/301_2000
  latitude    : 37.449722
  longitude   : 126.952222
  accuracy    : 2000(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=37.449722,126.952222
  root:~> ./file_loc ./proj4/301_200000
  latitude    : 37.449722
  longitude   : 126.952222
  accuracy    : 200000(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=37.449722,126.952222
  root:~> ./file_loc ./proj4/301_500000
  latitude    : 37.449722
  longitude   : 126.952222
  accuracy    : 500000(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=37.449722,126.952222
  root:~> ./file_loc ./proj4/301_5000000
  latitude    : 37.449722
  longitude   : 126.952222
  accuracy    : 5000000(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=37.449722,126.952222
  root:~> ./file_loc ./proj4/busan_1
  latitude    : 35.115000
  longitude   : 129.422222
  accuracy    : 1(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=35.115000,129.422222
  root:~> ./file_loc ./proj4/busan_300000
  latitude    : 35.115000
  longitude   : 129.422222
  accuracy    : 300000(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=35.115000,129.422222
  root:~> ./file_loc ./proj4/eiffel_1
  latitude    : 48.858056
  longitude   : 2.294444
  accuracy    : 1(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=48.858056,2.294444
  root:~> ./file_loc ./proj4/eiffel_5000000
  latitude    : 48.858056
  longitude   : 2.294444
  accuracy    : 5000000(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=48.858056,2.294444
  root:~> ./file_loc ./proj4/maingate_1
  latitude    : 37.466111
  longitude   : 126.948333
  accuracy    : 1(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=37.466111,126.948333
  root:~> ./file_loc ./proj4/maingate_1000
  latitude    : 37.466111
  longitude   : 126.948333
  accuracy    : 1000(m)
  (LINK) https://www.google.com/maps/search/?api=1&query=37.466111,126.948333</pre>

### Lessons learned
* ext2 파일 시스템이 어떻게 구성되고 관리되는지 알게 되었습니다.
* 같은 inode에 대해 두 가지 구조체가 존재한다는 것을 알게 되었습니다.
  * `struct ext2_inode`는 디스크에서 쓰이고 `struct ext2_inode_info`는 메모리에서 쓰입니다.
  * 디스크의 `ext2_inode`에서는 little endian(`__le32`)이 사용되고, 메모리의 `ext2_inode_info`에서는 big endian(`__u32`)이 사용됩니다.
  * 따라서 `le32_to_cpu()`와 `cpu_to_le32()` 매크로를 사용하여 둘 사이를 변환해 주어야 합니다.
* 저희가 수정한 ext2 파일 시스템으로 구성된 폴더를 생성하는 방법을 알게 되었습니다.
* 새 시스템 콜을 등록하고, 전역 변수에 대해 lock을 잡는 것은 이제 수월하게 할 수 있습니다.
* ext2 파일 시스템 안에서 새 파일이 생성되거나 수정될 때 ctime 또는 mtime이 바뀐다는 것을 알게 되었습니다.
* 지구 위 두 지점의 위도와 경도가 주어질 때, 두 지점 사이의 거리를 구하는 방법을 알게 되었습니다.
* floating point operation이 제공되지 않는 환경에서 유효숫자를 소숫점 아래 6자리 이상으로 유지하면서 계산하는 방법을 알게 되었습니다.
* shift, 덧셈, 뺄셈 연산만으로도 sin, cos, arctan 함수의 값을 원하는 정밀도까지 구하는 방법을 알게 되었습니다.
  * 곱셈 연산이 지원되지 않거나, floating point가 지원되지 않거나, 메모리가 매우 작은 마이크로프로세서 환경에서 유용하게 사용할 수 있을 것입니다.
* 학교 301동 건물과 학교 정문 사이의 거리가 1861m라는 것을 알게 되었습니다.
