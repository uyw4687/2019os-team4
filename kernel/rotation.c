#include "../arch/arm64/include/asm/unistd.h"
#include <linux/kernel.h>
#include <linux/sched.h>
#include <sys/types.h>

int rotation;
long sys_set_rotation(int degree) {
    if( degree < 0 || degree > 360) {
        printk( KERN_ERR "Out of range" );
        return -1;
    }
	//write_lock
    rotation = degree;
	//write_unlock
    return 0;
}
long sys_rotlock_read(int degree, int range){

	//put in waiting list
	//wait till getting the lock

    return -1;
}
long sys_rotlock_write(int degree, int range){

	//put in waiting list
	//wait till getting the lock

    return -1;
}
long sys_rotunlock_read(int degree, int range){

	//find in current list using list_for_each_entry_safe
	//then delete
    return -1;
}
long sys_rotunlock_write(int degree, int range){

	//find in current list using list_for_each_entry_safe
	//then delete
    return -1;
}
void exit_rotlock(struct task_struct *tsk){

	pid_t pid = tsk->pid;

	//remove_invalid_locks_and_requests(pid);
	//called with every thread exiting? or every process exiting?
}
/*
**********pid or tgid what to use in lock struct
void put_in_list
void remove_invalid_locks_requests(pid_t pid){
	
	//find in current list using list_for_each_entry_safe
	//then delete
	//find and delete all matching entries throughout the loop

	//do it also in waiting list
}
*/
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
