#include "../arch/arm64/include/asm/unistd.h"
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/rwlock.h>
#include <linux/rwlock_types.h>

#define READ 0
#define WRITE 1
#define EMPTY 2

int rotation;

DEFINE_RWLOCK(rot_lock);
DEFINE_RWLOCK(held_lock);
DEFINE_RWLOCK(wait_lock);

INIT_LIST_HEAD(&(lock_queue.list));
INIT_LIST_HEAD(&(wait_queue.list));

//int is_initialized1 = 0; // if initialize() is called, set to 1
//int is_initialized2 = 0;

/* range descriptor */
struct rd {
    pid_t pid;      /* process id of requested process */
    int range[2];   /* acquire lock only if range[0] <= rotation <= range[1] */
    int type;      /* READ or WRITE */
    struct list_head list;
};

struct rd lock_queue = { .pid = -1 };
struct rd wait_queue = { .pid = -1 };

DECLARE_WAIT_QUEUE_HEAD(wait_queue_head);

int compare_rd(struct rd *rd1, struct rd *rd2) {

    if ((rd1->pid == rd2->pid) && ((rd1->range)[0] == (rd2->range)[0]) &&
            ((rd1->range)[1] == (rd2->range)[1]) && (rd1->type == rd2->type))
        return 1; //true : same
    else
        return 0; //false : different
}

void set_lower_upper(int degree, int range, int *lower, int *upper) {
    
    *lower= degree-range;
    *upper = degree+range;

    if (*lower < 0)
        *lower = *lower + 360;
    
    if (*upper >= 360)
        *upper = *upper - 360;
    
}

int check_range(int rot, struct rd* rd1){

    int lower = (rd1->range)[0];
    int upper = (rd1->range)[1];
    if(lower <= upper && lower <= rot && rot <= upper)
        return 1;   //Range include rotation
    else if(lower >= upper && (lower <= rot || rot <= upper))
        return 1;   //Range include rotation
    else 
        return 0;   //Range don't include rotation
    
}

// if rd1 is in wait_queue, return 1; else return 0.
int check_waiting(struct rd* rd1) {
    struct list_head *head_wait;
    struct list_head *next_head_wait;
    
    struct rd *wait_entry;
    
    read_lock(&wait_lock);

    list_for_each_safe(head_wait, next_head_wait, &(wait_queue.list)) {

        wait_entry = list_entry(head_wait, struct rd, list);

        if (compare_rd(rd1, wait_entry) == 1) {
            read_unlock(&wait_lock);
            return 1;   // rd1 is in wait_queue
        }
    }

    read_unlock(&wait_lock);
    return 0;           // rd1 is not in wait_queue
}

int my_enqueue(struct list_head *queue, struct rd* val) {
    if (val->pid == -1) {
        printk(KERN_ERR "invalid rd");
        return -1;
    }
    list_add_tail(&(val->list), queue);
    return 0;
}

int my_dequeue(struct list_head *queue, struct rd *target) {
    struct list_head *head;
    struct list_head *next_head;
    struct rd *lock_entry;
    if (queue->next == queue) {
        printk(KERN_ERR "queue is empty");
        return 0;   //dequeue fail.
    }
    list_for_each_safe(head, next_head, queue) {
        lock_entry = list_entry(head, struct rd, list);
        if (compare_rd(target, lock_entry)){
            list_del_init(head);
            return 1;   //dequeue success, can use target.
            // please call kfree(target); after return
        }
    }
    printk(KERN_ERR "can't find target");
    return 0;   //dequeue fail.
}

int delete_lock(struct list_head *queue, int degree, int range, int type) {
    struct list_head *head;
    struct list_head *next_head;

    struct rd *lock_entry;

    int lower, upper;
    struct rd compare;

    set_lower_upper(degree, range, &lower, &upper);

    compare.pid = task_pid_nr(current);
    compare.range[0] = lower;
    compare.range[1] = upper;
    compare.type = type;
    
    list_for_each_safe(head, next_head, queue) {

        lock_entry = list_entry(head, struct rd, list);

        if (compare_rd(lock_entry, &compare)){

            list_del(head);
            kfree(lock_entry);
            return 1;   // success to delete 
        }
    }
    return 0;   // fail to delete
}

void remove_all(struct list_head *queue, pid_t pid) {
    
    struct list_head *head;
    struct list_head *next_head;

    struct rd *lock_entry;

	list_for_each_safe(head, next_head, queue) {

		lock_entry = list_entry(head, struct rd, list);

		if (lock_entry->pid == pid) {
			list_del(head);
			kfree(lock_entry);
		}
	}
}

int check_input(int degree, int range) {

    if(degree < 0 || degree >= 360 || range <= 0 || range >=180) {
        printk(KERN_ERR "Out of range");
        return -1;
    }
    return 0;
}

void change_queue(struct rd* input){
    struct list_head *head;
    struct list_head *next_head;
    struct rd *lock_entry;

    list_for_each_safe(head, next_head, &(wait_queue.list)) {
        lock_entry = list_entry(head, struct rd, list);

        if (compare_rd(lock_entry, input)) {

            if (my_dequeue(&(wait_queue.list), input)) {
                my_enqueue(&(lock_queue.list), input);
                break;
            }
        }
    }
}
/*
void initialize_list(void) {
    if (is_initialized1 == 0) {
        write_lock(&rot_lock);
        if (is_initialized2 == 0) {
            is_initialized1 = 1;
            is_initialized2 = 1;
        }
        write_unlock(&rot_lock);
    }
}
*/
void set_lock(struct rd* newlock, int degree, int range, int type) {

    int lower, upper; 

    set_lower_upper(degree, range, &lower, &upper);
    
    newlock->pid = task_pid_nr(current);
    newlock->range[0]= lower;
    newlock->range[1]= upper;
    newlock->type = type;
}

// if locks in wait_queue can be acquired, acquire the locks.
int check_and_acquire_lock(void) {
    
    struct list_head *head_wait;
    struct list_head *next_head_wait;

    struct list_head *head_lock;
    struct list_head *next_head_lock;
    
    struct rd *wait_entry;
    struct rd *lock_entry;
    
    int num_awoken_processes = 0;
    int held_lock_type = EMPTY;
    
    read_lock(&rot_lock);

    write_lock(&wait_lock);
    write_lock(&held_lock);
   
    // set held_lock_type.
    list_for_each_safe(head_lock, next_head_lock, &(lock_queue.list)) {

        lock_entry = list_entry(head_lock, struct rd, list);
        if (lock_entry->pid == -1) continue;

        if (check_range(rotation, lock_entry) == 0) {
            continue;
        }
        if (lock_entry->type == READ && held_lock_type != WRITE) {
            held_lock_type = READ;
        } else if (lock_entry->type == READ) {
            held_lock_type = WRITE;
            printk(KERN_ERR "Both write lock and read lock are held!");
        } else if (lock_entry->type == WRITE && held_lock_type != READ) {
            held_lock_type = WRITE;
        } else {
            held_lock_type = WRITE;
            printk(KERN_ERR "Both write lock and read lock are held!");
        }

    }

    // check if each wait_entry can acquire a lock.
    list_for_each_safe(head_wait, next_head_wait, &(wait_queue.list)) {

        wait_entry = list_entry(head_wait, struct rd, list);
        if (wait_entry->pid == -1) continue;

        if (check_range(rotation, wait_entry) == 0) {
            continue;
        }

        if (held_lock_type == EMPTY) {

            // awake a process and acquire lock for wait_entry
            change_queue(wait_entry);
            wake_up(&wait_queue_head);

            held_lock_type = wait_entry->type;
            num_awoken_processes++;

        } else if (held_lock_type == READ && wait_entry->type == READ) {

            // awake a process and acquire lock for wait_entry
            change_queue(wait_entry);
            wake_up(&wait_queue_head);

            num_awoken_processes++;

        } else {
            // if a wait_entry cannot acquire a lock,
            // also entries after the entry in wait_queue cannot acquire a lock.
            break;
        }

    }

    write_unlock(&held_lock);
    write_unlock(&wait_lock);

    read_unlock(&rot_lock);

    return num_awoken_processes;
}

long sys_set_rotation(int degree) {

    if (degree < 0 || degree > 360) {
        printk(KERN_ERR "Out of range");
        return -1;
    }

    //initialize_list();

    write_lock(&rot_lock);

    rotation = degree;

    write_unlock(&rot_lock);

    return check_and_acquire_lock();
}

long sys_rotlock_read(int degree, int range){

    struct rd* newlock;
	DEFINE_WAIT(wait);

    if (check_input(degree, range) < 0)
        return -1;

    newlock = (struct rd*)kmalloc(sizeof(struct rd), GFP_KERNEL);
	
	if(!newlock) {
		return -1;
	}

    set_lock(newlock, degree, range, READ);

    write_lock(&wait_lock);

    my_enqueue(&(wait_queue.list), newlock);

    write_unlock(&wait_lock);

    check_and_acquire_lock();   // TODO is this right execution order?

	add_wait_queue(&wait_queue_head, &wait);

	while (check_waiting(newlock)) {

		prepare_to_wait(&wait_queue_head, &wait, TASK_INTERRUPTIBLE);

		if (check_waiting(newlock))
			schedule();
	}

	finish_wait(&wait_queue_head, &wait);

    return 0;
}

long sys_rotlock_write(int degree, int range){

    struct rd* newlock; 
    DEFINE_WAIT(wait);

    if (check_input(degree, range) < 0)
        return -1;
    
    newlock = (struct rd*)kmalloc(sizeof(struct rd), GFP_KERNEL);
	
	if(!newlock) {
		return -1;
	}

    set_lock(newlock, degree, range, WRITE);
    
    write_lock(&wait_lock);

    my_enqueue(&(wait_queue.list), newlock);

    write_unlock(&wait_lock);

    check_and_acquire_lock();   // TODO is this right execution order?

	add_wait_queue(&wait_queue_head, &wait);

	while (check_waiting(newlock)) {

		prepare_to_wait(&wait_queue_head, &wait, TASK_INTERRUPTIBLE);

		if (check_waiting(newlock))
			schedule();
	}

	finish_wait(&wait_queue_head, &wait);

    return 0;
}

long sys_rotunlock_read(int degree, int range){

    int success;

    if (check_input(degree, range) < 0)
        return -1;

    write_lock(&held_lock);

	success = delete_lock(&(lock_queue.list), degree, range, READ);	

    write_unlock(&held_lock);

    if (!success) {
        printk(KERN_ERR "cannot unlock");
        return -1;
    }

	// check if other locks can come
    check_and_acquire_lock();

    return 0;
}

long sys_rotunlock_write(int degree, int range){

    int success;

    if (check_input(degree, range) < 0)
        return -1;

	write_lock(&held_lock);
	
	success = delete_lock(&(lock_queue.list), degree, range, WRITE);	

    write_unlock(&held_lock);

    if (!success) {
        printk(KERN_ERR "cannot unlock");
        return -1;
    }

    check_and_acquire_lock();
    
    return 0;
}

void rechange_range(struct rd* target, int* outrange, int* outdegree){

    int range1 = target->range[0];
    int range2 = target->range[1];
    
    if(range1 <= range2) {
        *outdegree = (range1 + range2)/2;
        *outrange = range2 - *outdegree;
    }
    else {
        range2 += 360;
        *outdegree = ((range1 + range2)/2)%360;
        *outrange = range2 - *outdegree;
    }
}

void exit_rotlock(struct task_struct *tsk){
    
    pid_t pid = tsk->pid;
    int range, degree;
    struct rd* lock_entry;
    struct list_head *head;
    struct list_head *next_head;

    write_lock(&wait_lock);
    write_lock(&held_lock);

    remove_all(&(wait_queue.list), pid);   // remove wait queue
    
    list_for_each_safe(head, next_head, &(lock_queue.list)) {

        lock_entry = list_entry(head, struct rd, list);

        if (lock_entry->pid != pid) {
            continue;
        }
        else if (lock_entry->type == READ) {
            rechange_range(lock_entry, &range, &degree);
            sys_rotunlock_read(degree, range);
        }
        else if (lock_entry->type == WRITE) {
            rechange_range(lock_entry, &range, &degree);
            sys_rotunlock_write(degree, range);
        }
    }   // if processes held locks, unlock them

    write_unlock(&held_lock);
    write_unlock(&wait_lock);

    // called with every thread exiting? or every process exiting?
}
