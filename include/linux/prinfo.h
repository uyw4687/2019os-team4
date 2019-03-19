#ifdef __KERNEL__
#include<linux/sched.h>
#include<asm/uaccess.h>
#include<linux/sched/task.h>
#include<linux/list.h>
#include<linux/slab.h>
#endif

struct prinfo {
    int64_t state;
    pid_t   pid;
    pid_t   parent_pid;
    pid_t   first_child_pid;
    pid_t   next_sibling_pid;
    int64_t uid;
    char    comm[64];
};
