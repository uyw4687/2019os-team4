#include "../arch/arm64/include/asm/unistd.h"
#include <linux/kernel.h>
#include <linux/module.h>
int rotation;
EXPORT_SYMBOL(rotation);
long sys_set_rotation(int degree) {
    rotation = degree;
    return rotation;
}
int lock_queue[500];
EXPORT_SYMBOL(lock_queue);
int size;
EXPORT_SYMBOL(size);
int front=0, rear=0, out;
EXPORT_SYMBOL(front);
EXPORT_SYMBOL(rear);
EXPORT_SYMBOL(out);
int put(int val) {
    if( ( rear+1 ) % size == front ) {
        printk( KERN_ERR "Stack is full" );
        return -1;
    }
    lock_queue[rear] = val;
    rear = ( rear + 1 ) % size;
    return val;
}
EXPORT_SYMBOL(put);
int get(void) {
    if( rear == front ) {
        printk( KERN_ERR "Stack is empty" );
        return -1;
    }
    out = lock_queue[front];
    front = ( front + 1 ) % size;
    return out;
}
EXPORT_SYMBOL(get);
