#include "../arch/arm64/include/asm/unistd.h"
#include <linux/kernel.h>

#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/rwlock.h>
#include <linux/rwlock_types.h>


int rotation;
DEFINE_RWLOCK(rot_lock);

long sys_set_rotation(int degree) {
    if( degree < 0 || degree > 360) {
        printk( KERN_ERR "Out of range" );
        return -1;
    }
    write_lock(&rot_lock);
    rotation = degree;
    write_unlock(&rot_lock);
    return 0;
}//TODO set queue stack that include rotation range.

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
