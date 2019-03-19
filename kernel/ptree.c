#include "../arch/arm64/include/asm/unistd.h"
#include <linux/ptree.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/string.h>
#include <linux/cred.h>

int sys_ptree(struct prinfo *buf, int *nr) {
	
    int errno;
    
    if (buf == NULL || nr == NULL) {
        errno = EINVAL;
        return -errno;
    }

    if (!access_ok(VERIFY_WRITE, nr, sizeof(int))) {
        errno = EFAULT;
        return -errno;
    }

    int n;  // the number of entries that actually copied into buf
    copy_from_user(&n, nr, sizeof(int));

    if (n < 1) {
        errno = EINVAL;
        return errno;
    }

    if (!access_ok(VERIFY_READ, buf, sizeof(struct prinfo) * n)) {
        errno = EFAULT;
        return -errno;
    }

	struct task_struct *task;
	int count=0;	

    struct prinfo *buf2;

    struct list_head *list;

    buf2 = (struct prinfo *)kmalloc(sizeof(struct prinfo) * n, GFP_KERNEL);

    read_lock(&tasklist_lock);

    /* 
     * TODO iterate over processes with DFS algorithm, update buf2, update n
     * discourage to use recursive function
     * DO NOT USE sleep, kmalloc, copy_to_user, copy_from_user!
     */

	for_each_process(task){
		if(count >= n)break;
		strcpy(buf2[count].comm, task->comm);
		buf2[count].state = (int64_t)(task->state);
		buf2[count].pid = (pid_t)task->pid;
		buf2[count].parent_pid = (pid_t)task->parent->pid;
		//buf2[count].first_child_pid = (pid_t)(list_entry(list_for_each(task->children.next, struct task_struct, children).pid);
		//buf2[count].next_sibling_pid = (pid_t)task->&p_osptr;
		buf2[count].uid = (int64_t)(task->cred->uid.val);
		count++;
	}
	if(count<n)n=count;

    /*
    LIST_HEAD(TODO);
    list_for_each_entry(pos, head, member) {
        TODO
    }
    */

    read_unlock(&tasklist_lock);
    
    copy_to_user(nr, &n, sizeof(int));
    copy_to_user(buf, buf2, sizeof(struct prinfo) * n);
    kfree(buf2);

    return 0;
}
