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
#### error checking process
먼저 user space memory인 buf, nr에 대해서 NULL값인지 확인해 줍니다.
그리고 buf, nr의 값을 copy_from_user로 받거나, buf2, n의 값을 copy_to_user로 줄 때 오류가 발생하면 모두 -EINVAL을 return합니다.
buf, nr에 VERIFY_WRITE에 대해서도 access_ok로 확인한 후 문제가 있으면 -EFAULT를 return합니다.




### Process tree investigation

### Lessons learned
