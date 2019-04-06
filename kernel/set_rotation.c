#include "../arch/arm64/include/asm/unistd.h"
#include <linux/kernel.h>

int rotation;
long sys_set_rotation(int degree) {
    if( degree < 0 || degree > 360) {
        printk( KERN_ERR "Out of range" );
        return -1;
    }
    rotation = degree;
    return 0;
}
long sys_rotlock_read(int degree, int range){
    return -1;
}
long sys_rotlock_write(int degree, int range){
    return -1;
}
long sys_rotunlock_read(int degree, int range){
    return -1;
}
long sys_rotunlock_write(int degree, int range){
    return -1;
}
int lock_queue[500];
int size;
int front=0, rear=0, out;
int put(int val) {
    if( ( rear+1 ) % size == front ) {
        printk( KERN_ERR "Stack is full" );
        return -1;
    }
    lock_queue[rear] = val;
    rear = ( rear + 1 ) % size;
    return val;
}
int get(void) {
    if( rear == front ) {
        printk( KERN_ERR "Stack is empty" );
        return -1;
    }
    out = lock_queue[front];
    front = ( front + 1 ) % size;
    return out;
}
