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
* compare_rd : 두 struct rd를 포인터로 받아 pid, range[0], range[1], type을 비교합니다. return 1(true) or 0(false)
* compare_overlap : 두 struct rd를 포인터로 받아 두 lock의 range가 겹치는지 확인합니다. return 1(true) or 0(false)
* set_lower_upper : degree와 range를 받아 lower bound, upper bound를 구해 줍니다.
* check_range : rotation값과 struct rd를 받아 lock bound 내에 rotation이 있는지 체크합니다. return 1(true) or 0(false) 
* check_waiting : 해당하는 struct rd가 wait_queue 내에 있는지 확인합니다. return 1(true) or 0(false)
* my_enqueue : queue에 entry를 넣어줍니다.
* my_dequeue : queue에서 target entry를 찾아서 빼줍니다. (항상 맨 앞에서만 빼지는 않습니다.)
* delete_lock : match 되는 lock entry 하나를 queue에서 빼고 제거해 줍니다. return 1(success) or 0(fail)
* remove_all : 해당되는 queue에서 pid가 match되는 모든 lock entry를 제거해 줍니다.
* check_input : input이 가능한 범위 내의 값인지 확인합니다.
* change_queue : lock entry를 wait_queue에서 lock_queue로 옮겨줍니다.
* set_lock : lock entry의 값을 세팅합니다. (초기화에 사용)
* check_and_acquire_lock : 현재 rotation 상태에서 잡을 수 있는, wait_queue에 있는 모든 락을 잡아줍니다. 락을 잡을 때 해당 프로세스를 깨워서 unblocked 상태로 만듭니다. 그리고 잡은 락의 개수를 리턴합니다.
  * 현재 rotation을 범위에 포함하지 않는 waiting entry는 무조건 무시합니다. wait_queue에 있는 범위에 맞는 waiting entry들을 앞에서부터 순서대로 살펴봅니다.
  * acquired entry는 현재 rotation을 포함하지 않아도 모두 살펴봅니다.
  * wait 중인 writer가 lock을 잡고 있는 하나 이상의 reader 또는 writer와 겹치면, 본인은 대기하고 이후에 들어온 모든 reader가 lock을 잡을 수 없게 합니다. 겹치지 않으면 해당 프로세스를 wake_up()하고 lock을 잡습니다.
  * wait 중인 reader가 lock을 잡고 있는 하나 이상의 writer와 겹치거나 앞에서 어떤 writer가 대기하고 있었으면 본인도 대기합니다(현재 rotation과 맞지 않는 waiting writer는 고려하지 않습니다). 그렇지 않으면 해당 프로세스를 wake_up()하고 lock을 잡습니다.
  * 앞에서 하나의 writer라도 새로 lock을 잡게 되면, 뒤에 있는 모든 waiting entry는 lock을 잡을 수 없게 됩니다.
  * 나중에 들어온 writer가 다른 acquired entry들과 겹치지 않으면, 먼저 들어와서 대기하고 있는 writer를 앞질러 lock을 잡을 수 있습니다.

#### set_rotation
* degree가 범위 안에 있는지 확인해 줍니다.
* 그리고 rotation 값을 쓸 것이기에 write_lock()을 해 줍니다.
* check_and_acquire_lock을 실행해 준 후 그 리턴 값(새로 락을 잡은 프로세스 수)을 리턴해 줍니다.

#### rotlock_read / rotlock_write
* input의 validity를 체크해 줍니다.
* 새로운 lock을 만들어 주고 wait_queue에 넣어 줍니다.
* wait_queue에 추가하는 조작이므로 write_lock()을 잡아 줍니다.
* wait_queue에 새 entry가 추가되었으므로 check_and_acquire_lock을 실행해 줍니다.
* wait mechanism으로 lock을 요청한 프로세스를 block시킵니다.

##### wait(process block) mechanism
* DEFINE_WAIT으로 wait을 지정해줍니다.
* check_waiting(newlock)이라는 condition을 이용해서 이 프로세스가 요청한 lock이 waiting list에 있는지 확인하고
* 전후로 add_wait_queue(), prepare_to_wait()을 통해 커널의 wait queue *(우리의 wait_queue와 다름)* 에 넣어줍니다. schedule()로 wait시켜 줍니다.
* 이 프로세스가 요청한 lock이 더이상 waiting list에 있지 않으면 finish_wait()으로 wait 과정을 끝냅니다.

#### rotunlock_read / rotunlock_write
* input의 validity를 체크해 줍니다.
* 그리고 해당되는 lock이 lock_queue에 있는지 찾습니다. 못 찾았을 경우 -1을 return합니다.
* lock_queue를 체크하고 변경할 수 있으므로 held_lock에 대한 write_lock()을 잡아 줍니다.
* 그리고 entry가 delete된 후 다른 lock이 잡힐 수 있으므로 check_and_acquire_lock을 해 줍니다.

#### exit_rotlock
* do_exit()에서 호출됩니다.
* task_struct pointer를 받아서 pid를 얻습니다.
* wait_queue, lock_queue 모두에 접근하고 entry를 제거할 예정이므로, wait_lock과 held_lock에 대해 write_lock()을 잡아 줍니다.
* 그리고 remove_all 함수를 호출하여 해당되는 pid를 가진 lock을 각 queue에서 제거해 줍니다.
* 이것으로 인해 다른 wait entry가 새롭게 락을 잡을 수도 있으므로 check_and_acquire_lock을 실행해 줍니다.

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
* lock을 이용하여 새로운 lock을 구현한다는 것이 처음에는 이상하게 느껴졌지만, concurrency를 보장하기 위해 rotation lock 내부적으로도 lock이 필요하다는 것을 깨닫게 되었습니다.
* 여러 프로세스가 동시에 실행될 때 concurrency가 보장되지 않으면 무슨 일이 일어나는지 직접 테스트하면서 확인해 보았습니다.
* 여러 프로세스가 동시에 실행될 때 concurrency가 보장되면 결과가 어떻게 달라지는지 확인해 보았습니다.
* concurrency를 보장하기 위해 어느 부분에서 어떤 장치를 두어야 할지 고민하고 이를 섬세하게 구현하였습니다. 그 장치(rwlock)가 잘 구현되어 있더라도 그것을 사용하는 쪽(rotation lock)에서 제대로 사용하지 않으면 deadlock 등의 문제가 발생할 수 있음을 알게 되었습니다.
* 서로 다른 두 circular range의 범위를 비교하는 함수나 각 wait entry가 lock을 잡을 수 있는지 판단하는 함수를 구현하기가 살짝 까다로웠습니다. 이 과정에서 writer starvation을 방지하려면 어떻게 구현해야 하는지 알게 되었습니다.
* lock을 사용하면 필연적으로 blocking이 일어나 속도가 느려질 수밖에 없다는 것을 알게 되었습니다.
* 프로세스가 종료된 후에 커널에서 처리하는 과정(do_exit)이 있다는 것을 새롭게 알게 되었습니다.
* 시스템 콜을 새로 등록하는 방법이나 doubly linked list를 사용하는 방법에는 꽤 익숙해졌습니다.
