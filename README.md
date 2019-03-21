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
### Process tree investigation

### Lessons learned
