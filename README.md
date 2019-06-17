1. 점수 : 52/55 
2. 감점 사유:
* Memory leak (-3)
3. 통계:
* 평균 : 49
* 표준편차 : 4.43250029
* Q1 (하위 25%) : 47.5
* Q2 (중간값) : 52
* Q3 (상위 25%) : 52

# os-team4
OS Spring Team4
## Project 1

### How to build our kernel
Project0의 순서로 build하시면 됩니다.
thread를 제외하고 출력해야 한다는 조건을 너무 늦게 안 나머지 thread와 process 구분 없이 출력했습니다.
thread를 제외하는 방법은 tgid와 pid가 같을 때만 유효한 process로 하고 
count를 늘려나가는 방식으로 하면 되겠습니다.

### High-level design and implementation
#### System call registration
* kernel 디렉토리에 시스템 콜 함수를 구현합니다.
* arch/arm64/include/asm/unistd.h 에서 시스템 콜 개수를 1 늘려줍니다.
* arch/arm64/include/asm/unistd32.h 에서 ptree 시스템 콜을 398번에 등록해 줍니다.
* include/linux/syscalls.h 에서 sys_ptree의 prototype을 적어 줍니다.
* kernel/Makefile에서 ptree.o를 추가해 줍니다.

---

> 다음은 kernel/ptree.c에 대한 설명입니다.

#### Error checking process / After DFS
* 먼저 user space memory인 buf, nr에 대해서 NULL값인지 확인해 줍니다. 만약 그렇다면 -EINVAL을 return합니다. 
* 그리고 buf, nr의 값을 copy_from_user로 받을 때 또는 buf2, n의 값을 copy_to_user로 줄 때 오류가 발생하면 모두 -EINVAL을 return합니다.
* buf, nr이 접근 가능한 영역인지 access_ok로 VERIFY_WRITE에 대해서 확인한 후 문제가 있으면 -EFAULT를 return합니다.
* buf, nr에 직접 접근하면 문제가 생길 수 있으므로, 지역 변수인 buf2, n을 사용합니다.
* buf2에 값을 할당해 준 후 나중에 처리가 다 되면 buf에 넣어줍니다.
* nr의 경우에도 실제로 할당된 값을 count로 센 후 nr값을 받았던 n과 비교하여 count가 더 작으면 `*nr`에 count값을 넣어줍니다.
* 앞의 처리 과정에서 문제가 없었다면, 총 프로세스 수인 count값을 return합니다.
#### DFS
##### Initial Condition
* 먼저 &init_task를 `struct task_struct *task`에 할당해 줍니다.
* 그리고 난 후 swapper에서 DFS를 시작하기 위해, task pid가 0인 task_struct를 가리키도록 while loop을 돌려줍니다.
  * 현재 pid가 0이 아니면 parent로 가서 swapper를 찾습니다.
* `get_value`를 호출하여
  <pre><code>read_lock(&tasklist_lock);
  read_unlock(&tasklist_lock);
  </code></pre>
  사이에서 task_struct 정보를 buf2에 넣어주는 작업을 합니다.
##### Function get_value
* recursive version으로 짰기 때문에 매번 시작하자마자 `*count`가 n보다 작은지 확인합니다.
  * 그렇다면 바로 `assign_value`를 호출하여 task의 process 정보를 buf2에 넣어줍니다.
  * 그렇지 않다면 프로세스 개수만 셉니다.
* child process가 있는지 확인한 후 있다면 list_for_each loop을 돌려 주고, 아니면 return합니다.

###### - Check if there is a child
* `(task->children).next == &(task->children)`이면 child process가 있는 경우입니다.
* task->children은 list_head로 child가 없으면 next값이 자기 자신의 children이라는 list_head의 주소를 가리키기 때문에 이를 이용해서 확인합니다.

###### - list_for_each
* task의 child process가 있는지 확인했으니 그 child process의 모든 sibling processes에 대해, 하나씩 task2에 넣어주고 get_value를 재귀호출합니다.

##### Function assign_value
* task의 comm, state, pid, parent의 pid, uid의 값을 그대로 `buf2[*count]`의 각 member에 넣습니다.
  * uid값은 `*task`의 `cred`멤버가 가리키는 uid의 val 값입니다.
* 첫 번째 child의 pid는 child가 있는지 확인한 후 없으면 0, 있으면 children의 next로 list_entry를 한 후 해당 task struct의 pid값을 넣어줍니다.
* sibling 역시 sibling의 next가 parent의 children의 next(첫 번째 child)인지 확인합니다.
  * 만약 그렇다면 다음 sibling이 존재하지 않으므로 0을 넣어줍니다.
  * 그렇지 않다면 sibling의 next에 대해 list_entry를 한 후 그 task struct의 pid를 넣어줍니다.

---

> 다음은 test/test_ptree.c에 대한 설명입니다.

#### Test Program
* test 디렉토리의 test를 실행합니다.
  * command line의 인자로 nr값을 받습니다. nr값을 반드시 입력해야 합니다.
  * 실행 예) ./test 50
* 변수 nr, buf에 각각 malloc으로 메모리를 할당합니다.
  * 만약 nr값이 음수이거나 메모리가 부족하여 malloc에 실패하면 오류를 출력하고 -1을 return합니다.
  * 프로그램이 종료될 때 할당한 메모리를 모두 free합니다.
* sys_ptree 시스템 콜을 호출합니다. 인자로 buf와 nr을 넘깁니다.
  * 시스템 콜의 return 값이 -1이면 오류를 처리합니다. 이때 errno의 값을 바로 저장했다가 오류 처리에 사용합니다.
  * 예를 들어 nr값이 0이면 errno가 EINVAL이 되므로, "Invalid argument"를 출력하고 -1을 return합니다.
* 시스템 콜을 호출하여 얻은 buf의 prinfo들을 가지고 프로세스 트리 모양을 복원하여 출력합니다.
  * 직접 구현한 스택을 이용해서 DFS의 결과를 트리로 만듭니다. 출력 시 들여쓰는 정도가 스택의 원소 개수와 관련이 있습니다.
  * 첫 번째 프로세스(swapper)를 스택에 넣고, 다음 프로세스부터 아래의 내용을 반복합니다.
    * 스택의 가장 위에 있는 원소가 현재 프로세스의 parent와 같은 동안 스택을 계속 pop합니다.
    * 만약 현재 프로세스의 child 프로세스가 존재하면 현재 프로세스를 스택에 push하고 출력합니다.
    * 그렇지 않으면 그냥 현재 프로세스를 출력합니다.
* 마지막으로, 시스템 콜에서 return된 총 프로세스 수와 변화한 nr값을 출력합니다.

---

### Process tree investigation
* swapper
  * pid가 0이며, 모든 프로세스의 조상이 되는 프로세스입니다.
  * sleep 상태가 된 프로세스들을 메모리에서 디스크로 swap in하고, 반대로 디스크에서 메모리로 swap out하기도 하면서 메모리를 관리합니다.
* systemd
  * pid가 1이며, swapper를 parent로 갖습니다.
  * user space를 부트스트랩하고 user processes를 관리하는 init 시스템 프로세스입니다.
  * systemd 외에도 여러 init 시스템이 있지만, 요즘 배포되는 Linux에서는 대부분 systemd를 채택합니다.
* kthreadd
  * pid가 2이며, swapper를 parent로 갖습니다.
  * 최상위 커널 쓰레드로, 요청이 있을 때마다 새 커널 쓰레드를 만들어 줍니다.

### Lessons learned
* 먼저 가장 크게 와닿았던 부분은 리눅스 커널에서 프로세스 트리가 어떤 형태로 저장되어 있는지 알 수 있었다는 점입니다.
* 복잡하다면 복잡한 구조인 doubly linked list 구조로 되어있서 list_for_each, list_entry를 사용해야 하는 것도 새로웠습니다.
* 그리고 이제껏 user level에서 programming을 해 왔었는데 kernel에서의 programming은 처음이라 재미있었습니다.
* malloc대신 kmalloc을 써야 한다는 것과 printf 대신 printk를 써야 한다는 등의 제약 조건은 물론이고, user memory에 대해 바로 접근해서는 안 된다는 것(대신 copy_to_user, copy_from_user 사용), read_lock의 개념 등에 대해서도 새롭게 알았습니다.
* 뿐만 아니라 return value가 errno로 어떻게 연결이 되는지도 흥미로웠습니다.
* 이런 경험을 통해 리눅스 커널에서는 신기하고 재밌는 일이 일어나고 있다는 걸 깨달았고, 앞으로의 프로젝트를 통해 더욱 많은 것을 알 수 있을 것 같아 기대가 됩니다.
