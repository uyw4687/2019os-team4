#include "../arch/arm64/include/asm/unistd.h"
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>

/* range descriptor */
struct rd {
    pid_t pid;      /* process id of requested process */
    int range[2];   /* acquire lock only if range[0] <= rotation <= range[1] */
    int state;      /* read: 0, write: 1 */
    struct list_head list;
};

int rotation;
struct list_head lock_queue;
struct list_head wait_queue;
int front = 0, rear = 0;
int is_initialized = 0; // if initialize() is called, set to 1

void initialize_list(void) {
    // TODO concurrency, call timing
    if (is_initialized == 0) {
        is_initialized = 1;
        INIT_LIST_HEAD(&lock_queue);
        INIT_LIST_HEAD(&wait_queue);
    }
}

long sys_set_rotation(int degree) {
    if (degree < 0 || degree > 360) {
        printk(KERN_ERR "Out of range");
        return -1;
    }
    initialize_list();
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

int push(struct list_head queue, struct rd* val) {
    if (val->pid == -1) {
        printk(KERN_ERR "invalid rd");
        return -1;
    }
    list_add_tail(&(val->list), &queue);
    return 0;
}

struct rd* pop(struct list_head queue) {

    struct rd* out;
    if (queue.next == &queue) {
        printk(KERN_ERR "queue is empty");

        out = (struct rd*)kmalloc(sizeof(struct rd), GFP_KERNEL);
        out->pid = -1;  // empty rd
        INIT_LIST_HEAD(&(out->list));
        return out;
    }
    out = list_entry(queue.next, struct rd, list);
    list_del_init(queue.next);
    return out;
    // TODO must call kfree(out);
}
