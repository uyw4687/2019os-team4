#include "../arch/arm64/include/asm/unistd.h"
#include <linux/kernel.h>

long sys_rotlock_read(int degree, int range) {
    return -1;
}
