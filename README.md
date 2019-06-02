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
  * `int (*get_gps_location)(struct inode *, struct gps_location *)
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

#### Terminating routine
* kernel/exit.c의 do_exit() 함수에 kernel/rotation.c에서 정의된 exit_rotlock을 호출하는 코드를 넣습니다.
  * rotation lock을 잡은 프로세스가 unlock하지 않고 종료될 때, 잡고 있던 rotation lock을 놓게 합니다.

#### Synchronization
* 내부적으로 rwlock을 이용하여, 공유 메모리에 대한 critical section을 만들어 주는 방식으로 구현하였습니다.

---

> 다음은 kernel/rotation.c에 대한 설명입니다.

#### global values
<pre><code>
int rotation                   // rotation값을 저장하는 변수입니다.

DEFINE_RWLOCK(rot_lock);       // rotation값에 대한 접근을 제한하는 lock입니다.
DEFINE_RWLOCK(held_lock);      // lock_queue에 대한 접근을 제한하는 lock입니다.
DEFINE_RWLOCK(wait_lock);      // wait_queue에 대한 접근을 제한하는 lock입니다.

struct rd{

 pid_t pid;
 
 int range[2];
 
 int type;
 
 struct list_head list;
 
}
</code></pre>
각 lock을 표현하는 struct입니다.
pid, range(lower bound, upper bound), type(READ/WRITE)를 저장합니다.
<pre><code>
LIST_HEAD(lock_queue); // lock_queue(acquired list) doubly linked list의 list_head입니다.
LIST_HEAD(wait_queue); // wait_queue(waiting list) doubly linked list의 list_head입니다.

DECLARE_WAIT_QUEUE_HEAD(wait_queue_head); // wait_queue의 head를 선언해 줍니다.
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

