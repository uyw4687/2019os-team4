#include "../arch/arm64/include/asm/unistd.h"
#include <linux/kernel.h>
#include <linux/module.h>
int rotation;
EXPORT_SYMBOL(rotation);
int sizeq = 500;
int queue[500];
int frontq = 0, rearq = 0;
EXPORT_SYMBOL(queue);
EXPORT_SYMBOL(frontq);
EXPORT_SYMBOL(rearq);
EXPORT_SYMBOL(sizeq);

long sys_set_rotation(int degree) {
    rotation = degree;
    return rotation;
}

int putq(int val) {
    if( ( rearq+1 ) % sizeq == frontq ) {
        printk( KERN_ERR "Stack is full" );
        return -1;
    }
    queue[rearq] = val;
    rearq = ( rearq + 1 ) % sizeq;
    return val;
}
EXPORT_SYMBOL(putq);

int getq(void) {
    if( rearq == frontq ) {
        printk( KERN_ERR "Stack is empty" );
        return -1;
    }
    int a;
    a = queue[frontq];
    frontq = ( frontq + 1 ) % sizeq;
    return a;
}
EXPORT_SYMBOL(getq);
