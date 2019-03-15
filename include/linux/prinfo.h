#include <include/uapi/asm-generic/errno-base.h>
#include <include/asm/uaccess.h>
#include <include/linux/sched.h>
#include <include/linux/list.h>
#include <linux/sched/task.h>
#include <linux/slab.h>

struct prinfo {
    int64_t state;
    pid_t   pid;
    pid_t   parent_pid;
    pid_t   first_child_pid;
    pid_t   next_sibling_pid;
    int64_t uid;
    char    comm[64];
};

int sys_ptree(struct prinfo *buf, int *nr) {
    
    if (buf == NULL || nr == NULL) {
        errno = -EINVAL;
        return -1;
    }

    if (!access_ok(VERIFY_WRITE, nr, sizeof(int))) {
        errno = -EFAULT;
        return -1;
    }

    int n;  // the number of entries that actually copied into buf
    copy_from_user(&n, nr, sizeof(int));

    if (n < 1) {
        errno = -EINVAL;
        return -1;
    }

    if (!access_ok(VERIFY_READ, buf, sizeof(struct prinfo) * n)) {
        errno = -EFAULT;
        return -1;
    }

    struct prinfo *buf2;
    buf2 = (struct prinfo *)kmalloc(sizeof(struct prinfo) * n, GFP_KERNEL);

    read_lock(&tasklist_lock);

    /* 
     * TODO iterate over processes with DFS algorithm, update buf2, update n
     * discourage to use recursive function
     * DO NOT USE sleep, kmalloc, copy_to_user, copy_from_user!
     */

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
