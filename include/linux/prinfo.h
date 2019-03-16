#include <uapi/asm-generic/errno-base.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/list.h>
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
