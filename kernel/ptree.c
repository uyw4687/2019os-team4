#include "../arch/arm64/include/asm/unistd.h"
#include <linux/ptree.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/string.h>
#include <linux/cred.h>
#include <linux/list.h>

/*
struct stack {
    struct prinfo data[100];
    int top;
};

struct prinfo empty = {
    .pid = -1
};

void init(struct stack *st);
struct prinfo pop(struct stack *st);
void push(struct stack *st, struct prinfo p);
struct prinfo peek(struct stack st);
*/

void assign_value(struct task_struct *task, int *count, struct prinfo *buf2) {

    strncpy(buf2[*count].comm, task->comm, 64);
    buf2[*count].state = (int64_t)(task->state);
    buf2[*count].pid = (pid_t)task->pid;
    buf2[*count].parent_pid = (pid_t)task->parent->pid;
    
    if ((task->children).next == &(task->children)) {
        buf2[*count].first_child_pid = 0;
    } else {
        buf2[*count].first_child_pid = (pid_t)(list_entry((task->children).next, struct task_struct, sibling)->pid);
    }

    if ((task->parent->children).next == (task->sibling).next) {
        buf2[*count].next_sibling_pid = 0;
    } else {
        buf2[*count].next_sibling_pid = (pid_t)(list_entry((task->sibling).next, struct task_struct, sibling)->pid);
    }

    buf2[*count].uid = (int64_t)(task->cred->uid.val);
}

/* recursive version */
void get_value(struct task_struct *task, int *count, struct prinfo *buf2, int n) {
    
    struct list_head *list;
    struct task_struct *task2;
    
    if (*count < n)
        assign_value(task, count, buf2); 
    
    *count = *count + 1;
    
    if ((task->children).next == &(task->children))
        return;

    list_for_each(list, &task->children) {
        task2 = list_entry(list, struct task_struct, sibling);
        get_value(task2, count, buf2, n);
    }
    return;
}

long sys_ptree(struct prinfo *buf, int *nr) {
    
    int err, errno;
    int n;  // the number of entries that actually copied into buf
    struct task_struct *task;
    int count = 0;  // the total number of entries
    struct prinfo *buf2;

    if (buf == NULL || nr == NULL) {
        errno = EINVAL;
        return -EINVAL;
    }

    if (!access_ok(VERIFY_WRITE, nr, sizeof(int))) {
        errno = EFAULT;
        return -EFAULT;
    }

    err = copy_from_user(&n, nr, sizeof(int));
    if (err != 0) {
        errno = EINVAL;
        return -EINVAL;
    }

    if (n < 1) {
        errno = EINVAL;
        return -EINVAL;
    }

    if (!access_ok(VERIFY_WRITE, buf, sizeof(struct prinfo) * n)) {
        errno = EFAULT;
        return -EFAULT;
    }

    task = &init_task;

    buf2 = (struct prinfo *)kmalloc(sizeof(struct prinfo) * n, GFP_KERNEL);

    if (!buf2) {
        errno = EFAULT;
        return -EFAULT;
    }

    // find swapper process
    while (1) {
        if (task->pid == 0)
            break;
        else
            task = task->parent;
    }

    read_lock(&tasklist_lock);

    // DO NOT USE sleep, kmalloc, copy_to_user, copy_from_user!
    get_value(task, &count, buf2, n);

    read_unlock(&tasklist_lock);
    
    if (count < n)
        n = count;

    err = copy_to_user(nr, &n, sizeof(int));
    if (err != 0) {
        errno = EINVAL;
        return -EINVAL;
    }

    err = copy_to_user(buf, buf2, sizeof(struct prinfo) * n);
    if (err != 0) {
        errno = EINVAL;
        return -EINVAL;
    }

    kfree(buf2);

    return count;
}

/*
void init(struct stack *st) {
    st->top = -1;
}

void push(struct stack *st, struct prinfo p) {
    if (st->top >= 99) return;  // stack is full!
    st->top++;
    st->data[st->top] = p;
}

struct prinfo pop(struct stack *st) {
    if (st->top < 0) return empty;  // stack is empty!
    st->top--;
    return st->data[st->top + 1];
}

struct prinfo peek(struct stack st) {
    if (st.top < 0) return empty;  // stack is empty!
    return st.data[st.top];
}
*/
