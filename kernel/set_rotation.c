#include "../arch/arm64/include/asm/unistd.h"
#include <linux/kernel.h>
#include <linux/module.h>
int rotation;
EXPORT_SYMBOL(rotation);

long sys_set_rotation(int degree) {
    rotation = degree;
    return rotation;
}
