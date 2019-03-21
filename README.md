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
먼저 &init_task를 `struct task_struct *task`에 할당해 줍니다.
그리고 난 후 swapper로 DFS를 시작하기 위해 task pid=0인 task_struct를 가리키도록 while loop을 돌려줍니다.
현재 pid가 0이 아니면 parent로 가서 swapper를 찾습니다.

read_lock(&tasklist_lock);
read_unlock(&tasklist_lock);
사이에서 task_struct 정보를 buf2에 넣어주는 작업을 합니다.(get_value)

### Process tree investigation

### Lessons learned
