#include "../arch/arm64/include/asm/unistd.h"
#include <linux/kernel.h>
#include <linux/module.h>
int rotation;
EXPORT_SYMBOL(rotation);

int queue[size];
int frontq = 0, rearq = 0;
int sizeq = 500;
EXPORT_SYMBOL(queue);
EXPORT_SYMBOL(frontq);
EXPORT_SYMBOL(rearq);
EXPORT_SYMBOL(sizeq);

long sys_set_rotation(int degree) {
    rotation = degree;
    return rotation;
}

int putq(int val) {
    if( ( rearq+1 ) % sizeq == front ) {
        printk( KERN_ERR "Stack is full" );
        return -1;
    }
    queue[rearq] = val;
    rear = ( rear + 1 ) % sizeq;
    return val;
}
EXPORT_SYMBOL(putq);

int getq(void) {
    if( rearq == frontq ) {
        printk( KERN_ERR "Stack is empty" );
        return -1;
    }
    int result;
    result = queue[frontq];
    frontq = ( frontq + 1 ) % sizeq;
    return result;
}
EXPORT_SYMBOL(getq);
