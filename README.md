# os-team4
OS Spring Team4
## Project 2

### How to build our kernel
project 기본 build 방법으로 하시면 됩니다.

### High-level design and implementation
#### System call registration
* kernel/rotation.c 에 시스템 콜 함수를 구현합니다.
* arch/arm64/include/asm/unistd.h 에서 시스템 콜 개수를 5 늘려줍니다. 398 -> 403
* arch/arm64/include/asm/unistd32.h 에서 각 시스템 콜을 스펙에 등록해 줍니다.
* include/linux/syscalls.h 에서 각 system call의 prototype을 적어 줍니다.
* kernel/Makefile에서 rotation.o를 추가해 줍니다.

---

> 다음은 kernel/rotation.c에 대한 설명입니다.

#### global values

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
각 lock을 표현하는 struct입니다.
pid, range(lower bound, upper bound), type(READ/WRITE)를 저장합니다.

LIST_HEAD(lock_queue); //lock_queue doubly linked list의 list_head입니다.
LIST_HEAD(wait_queue); //lock_queue doubly linked list의 list_head입니다.

DECLARE_WAIT_QUEUE_HEAD(wait_queue_head); // wait_queue의 head를 선언해 줍니다.

#### helper functions
compare_rd : 두 struct rd를 포인터로 받아 pid, range[0], range[1], type을 비교합니다. return 1(true) or 0(false)
set_lower_upper : degree와 range를 받아 lower bound, upper bound를 구해 줍니다.
check_range : rotation값과 struct rd를 받아 lock bound 내에 rotation이 있는지 체크합니다. return 1(true) or 0(false) 
check_waiting : 해당하는 struct rd가 wait_queue 내에 있는지 확인합니다. return 1(true) or 0(false)
my_enqueue : queue에 entry를 넣어줍니다.
my_dequeue : queue에서 entry를 빼줍니다.
delete_lock : match 되는 lock entry를 없애줍니다. return 1(success) or 0 (fail)
remove_all : 해당되는 queue에서 pid가 match되는 모든 lock entry를 제거해 줍니다.
check_input : input이 범위 내의 값인지 확인합니다.
change_queue : wait_queue에서 lock_queue로 옮겨줍니다.
set_lock : lock entry의 값을 세팅합니다.
check_and_acquire_lock : 현재 상태에서 잡을 수 있는 모든 락을 잡아줍니다. 그리고 잡은 락의 개수를 리턴합니다.

##### solving writer starvation
check_and_acquire_lock에서 현재 rotation에 대해 잡고 있는 lock의 종류를 확인합니다.
read lock 밖에 없을 경우 READ, write lock밖에 없을 경우 write 아무것도 없을 경우 EMPTY입니다.


#### set_rotation
degree가 범위 안에 있는지 확인해 줍니다.
그리고 rotation 값을 쓸 것이기에 write_lock을 해 줍니다.
check_and_acquire_lock을 실행해 준 후 리턴 값을 리턴해 줍니다.

#### set_rotlock_read/set_rotlock_write
새로운 lock을 만들어 주고 wait_queue에 넣어 줍니다.
wait_queue에 추가하는 조작이므로 write_lock을 잡아 줍니다.
wait_queue에 entry가 추가가 되었으므로 check_and_acquire_lock을 실행해 줍니다.

##### wait mechanism
DEFINE_WAIT으로 wait을 지정해줍니다.
condition은 check_waiting(newlock)이라는 condition을 이용해서 waiting list에 있는지 확인하고
전후로 add_wait_queue, prepare_to_wait을 통해 wait queue에 넣어줍니다. schedule()로 wait시켜 줍니다.
finish_wait으로 wait 과정을 끝냅니다.

#### set_rotunlock_read/set_rotunlock_write
input의 validity를 체크해 줍니다.
그리고 해당되는 lock이 있는지 찾은 후 못 찾았을 경우 -1을 return합니다.
잡고 있는 lock을 체크하므로 held_lock에 대한 write_lock을 잡아 줍니다.
그리고 entry가 delete된 후 다른 lock이 잡힐 수 있으므로 check_and_acquire_lock을 해 줍니다.

##### wait mechanism
DEFINE_WAIT으로 wait을 지정해줍니다.
condition은 check_waiting(newlock)이라는 condition을 이용해서 waiting list에 있는지 확인하고
전후로 add_wait_queue, prepare_to_wait을 통해 wait queue에 넣어줍니다. schedule()로 wait시켜 줍니다.
finish_wait으로 wait 과정을 끝냅니다.

#### exit_rotlock
task_struct pointer를 받아서 pid를 얻습니다.
그리고 wait_queue, lock_queue 모두에 접근하고 entry를 제거할 예정이므로, wait_lock과 held_lock에 대해 write_lock을 잡아 줍니다.
그리고 remove_all 함수로 해당되는 pid를 가진 lock을 제거해 줍니다.
이것으로 인해 새롭게 락을 잡을 수도 있으므로 check_and_acquire_lock을 실행해 줍니다.

---

> 다음은 test 프로그램에 대한 설명입니다.

#### selector
input으로 정수 하나를 받습니다.
아래 과정을 반복합니다.
degree = 90, range = 90으로 rotlock_write를 잡아준 후 integer라는 파일에 input으로 받은 정수를 넣습니다.
정수값을 1 늘려줍니다.
syscall과 fopen에서 실패한 경우 프로그램을 종료합니다.

#### trial
input으로 정수 하나를 받습니다.
이를 출력할 때 trial- 다음에 춣력하여 index로 역할을 하게 합니다.
아래 과정을 반복합니다.
rotlock_read를 degree=90, range=90으로 잡아준 후 integer 파일에서 정수 하나를 읽습니다.
그 정수를 인수분해해서 출력해 줍니다.
syscall과 fopen에서 실패한 경우 프로그램을 종료합니다.

### Lessons learned
