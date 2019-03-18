#include <arch/arm64/include/asm/unistd.h>
#include <include/linux/ptree.h>
#include <include/linux/kernel.h>

int sys_ptree(struct prinfo *buf, int *nr) {
    
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
