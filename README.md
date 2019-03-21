# os-team4
OS Spring Team4
## Project 1

### How to build our kernel
Project0의 순서로 build하시면 됩니다.

### High-level design and implementation
#### System call registration
시스템 콜을 등록해 줍니다.
arch/arm64/include/asm/unistd.h 에서 시스템 콜 개수를 1 늘려줍니다.
arch/arm64/include/asm/unistd32.h 에서 ptree 시스템 콜을 398번에 등록해 줍니다.
include/linux/syscalls.h 에서 sys_ptree의 prototype을 적어 줍니다.
kernel/Makefile에서 ptree.o를 추가해 줍니다.
#### Error checking process
먼저 user space memory인 buf, nr에 대해서 NULL값인지 확인해 줍니다.
그리고 buf, nr의 값을 copy_from_user로 받거나, buf2, n의 값을 copy_to_user로 줄 때 오류가 발생하면 모두 -EINVAL을 return합니다.
buf, nr에 VERIFY_WRITE에 대해서도 access_ok로 확인한 후 문제가 있으면 -EFAULT를 return합니다.
buf, nr에 직접 접근할 수 없으므로, buf2, n을 사용합니다.
buf2에 값을 할당해 준 후 나중에 처리가 다 되면 buf에 넣어줍니다.
nr의 경우에도 실제로 할당된 값을 count로 센 후 nr값을 받았던 n과 비교하여 count가 더 작으면 `*nr`에 count값을 넣어줍니다.
#### DFS
##### Initial Condition
먼저 &init_task를 `struct task_struct *task`에 할당해 줍니다.
그리고 난 후 swapper로 DFS를 시작하기 위해 task pid=0인 task_struct를 가리키도록 while loop을 돌려줍니다.
현재 pid가 0이 아니면 parent로 가서 swapper를 찾습니다.

read_lock(&tasklist_lock);
read_unlock(&tasklist_lock);
사이에서 task_struct 정보를 buf2에 넣어주는 작업을 합니다.(`get_value`)
##### Function get_value
recursive version으로 짰기 때문에 매번 시작하자마자 `*count`가 n 이상인지 확인을 합니다.
아니면 바로 `assign_value`로 task의 process 정보를 buf2에 넣어줍니다.
child process가 있는지 확인한 후 있다면 list_for_each loop을 돌려 주고 아니면 return합니다.

##### Check if there is a child
(task->children).next == &(task->children)이면 child process가 있는 경우입니다.
task->children은 list_head로 child가 없으면 next값이 자기 자신의 children이라는 list_head의 주소를 가리키기 때문에 이를 이용해서 확인합니다.

##### list_for_each
task의 child process가 있는지 확인했으니 그 child process의 모든 sibling process에 대해 task2에 넣어주고 get_value를 실행해 줍니다.

##### Function assign_value
task의 comm, state, pid, uid의 값을 그대로 `buf2 + *count`의 각 member에 넣는다.
uid값은 `*task`의 `cred`멤버가 가리키는 uid의 val 값이다.
child의 pid는 child가 있는지 확인한 후 없으면 0, 있으면 children의 next로 list_entry를 한 후 해당 task struct의 pid값을 넣어준다.
sibling 역시 sibling의 next가 parent의 children의 next인지 확인하고 만약 그렇다면 다음 sibling은 이미 출력한 process이므로 0을 넣어준다.
그렇지 않을 경우에는 sibling의 next에 대해 list_entry를 한 후 그 task struct의 pid를 넣어준다.

### Process tree investigation

### Lessons learned
