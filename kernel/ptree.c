#include "../arch/arm64/include/asm/unistd.h"
#include <linux/ptree.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/string.h>
#include <linux/cred.h>
#include <linux/list.h>

struct stack {
    struct prinfo data[100];
    int top;
};

void init(struct stack *st);
struct prinfo pop(struct stack *st);
void push(struct stack *st, struct prinfo p);
struct prinfo peek(struct stack st);

void get_value(struct task_struct *task, int *count, struct prinfo *buf2, int &n){
    assign_value(task, count, buf2);
    LIST_HEAD(p);
    p = task->children;
    list_for_each_entry(p, struct task_struct, children);
    (task -> children) //TODO
    while(count < n){
        list_entry(
        count++;
    }
    if(count<n)n=count; //TODO
}
void assign_value(struct task_struct *task, int *count, struct prinfo *buf2) {
    strncpy(buf2[count].comm, 64);
    buf2[count].state = (int64_t)(task->state);
    buf2[count].pid = (pid_t)task->pid;
    buf2[count].parent_pid = (pid_t)task->parent->pid;
    //buf2[count].first_child_pid = (pid_t)(list_entry(list_for_each(task->children.next, struct task_struct, children).pid);
    //buf2[count].next_sibling_pid = (pid_t)task->&p_osptr;
    buf2[count].uid = (int64_t)(task->cred->uid.val);
    count++;
    
}

long sys_ptree(struct prinfo *buf, int *nr) {
    
    int errno;
    int n;  // the number of entries that actually copied into buf
    struct task_struct *task;
    int count=0;
    struct prinfo *buf2;
    struct list_head *list;
    struct stack st;

    if (buf == NULL || nr == NULL) {
        errno = EINVAL;
        return -errno;
    }

    if (!access_ok(VERIFY_WRITE, nr, sizeof(int))) {
        errno = EFAULT;
        return -errno;
    }

    copy_from_user(&n, nr, sizeof(int));

    if (n < 1) {
        errno = EINVAL;
        return errno;
    }

    if (!access_ok(VERIFY_READ, buf, sizeof(struct prinfo) * n)) {
        errno = EFAULT;
        return -errno;
    }

    task = &init_task;

    buf2 = (struct prinfo *)kmalloc(sizeof(struct prinfo) * n, GFP_KERNEL);

    while(1) {
    if(task.pid == 0)
        break;
    else
        task = task -> parent;
    }

    init(&st);

    read_lock(&tasklist_lock);

   //  * DO NOT USE sleep, kmalloc, copy_to_user, copy_from_user!
    get_value(task, &count, &n, buf2);

    read_unlock(&tasklist_lock);
    
    copy_to_user(nr, &n, sizeof(int));
    copy_to_user(buf, buf2, sizeof(struct prinfo) * n);
    kfree(buf2);

    return 0;
}

void init(struct stack *st) {
    st->top = -1;
}

void push(struct stack *st, struct prinfo p) {
    if (st->top >= 99) return;  // stack is full!
    st->top++;
    st->data[st->top] = p;
}

struct prinfo pop(struct stack *st) {
    if (st->top < 0) return NULL;  // stack is empty!
    st->top--;
    return st->data[st->top + 1];
}

struct prinfo peek(struct stack st) {
    if (st.top < 0) return NULL;  // stack is empty!
    return st.data[st.top];
}
