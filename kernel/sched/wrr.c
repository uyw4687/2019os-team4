/*
 * Weighted Round Robin (WRR) Class (SCHED_WRR)
 */

#include "sched.h"

/*
 * Timeslice unit is 10 msecs (used only for SCHED_WRR tasks).
 * (Timeslices get refilled after they expire.)
 *
 * (ref)include/linux/sched/rt.h 65
 */
#define WRR_TIMESLICE (10 * HZ / 1000)

int sched_wrr_timeslice = WRR_TIMESLICE;

const struct sched_class wrr_sched_class;

static inline bool task_is_wrr(struct task_struct *tsk)
{
    int policy = tsk->policy;

    if(policy == SCHED_WRR)
        return true;

    return false;
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
/*
 * load balancing : 2000ms
 * refer to other schedulers about load balancing if materials exists
 * 
 * Make sure that it only works when more than one CPU is active
 * CPU hotplug
 * for_each_online_cpu(cpu)
 * 
 * RQ_MIN
 * RQ_MAX
 *
 * pick a task(largest weight/not running/moving to RQ_MIN is possible)
 *
 */
}

static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *task)
{
	if (task->policy == SCHED_WRR)
		return task->wrr.weight * sched_wrr_timeslice;
	else
		return 0;
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    // TODO fair.c 4879L / rt.c 1321L
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    // TODO fair.c 4935L / rt.c 1334L
}

static void yield_task_wrr(struct rq *rq)
{
    // TODO fair.c 6396L / rt.c 1373L
}

static struct task_struct *pick_next_task_wrr(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
    // TODO fair.c 6252L / rt.c 1530L & 1511L
    
    return NULL;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev)
{
    // TODO fair.c 6380L / rt.c 1577L
}

static int select_task_rq_wrr(struct task_struct *p, int prev_cpu, int sd_flag, int wake_flags)
{
    // TODO fair.c 5942L / rt.c 1382L

    return 0;
}

static void migrate_task_rq_wrr(struct task_struct *p)
{
    // TODO fair.c 6035L
}

static void rq_online_wrr(struct rq *rq)
{
    /*
    update_sysctl();

    update_runtime_enabled(rq);
    */

    // TODO fair.c 9025L / rt.c 2153L
}

static void rq_offline_wrr(struct rq *rq)
{
    // TODO fair.c 9032L / rt.c 2164L
}

static void set_curr_task_wrr(struct rq *rq)
{
    // TODO fair.c 9254L / rt.c 2328L
}

static void task_fork_wrr(struct task_struct *p)
{
    // TODO fair.c 9064L
}

static void switched_from_wrr(struct rq *rq, struct task_struct *p)
{
    // TODO fair.c 9227L / rt.c 2178L
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
    if (task_on_rq_queued(p)) {
        if (rq->curr == p) {
            // TODO
        } else {
            // TODO
        }
    }

    // TODO fair.c 9232L / rt.c 2209L
}

static void update_curr_wrr(struct rq *rq)
{
    // TODO fair.c 862L & 827L / rt.c 951L
}

const struct sched_class wrr_sched_class = {
    .next = &fair_sched_class,
    .enqueue_task = enqueue_task_wrr,
    .dequeue_task = dequeue_task_wrr,
    .yield_task = yield_task_wrr,
    //.yield_to_task = yield_to_task_wrr,
    //.check_preempt = check_preempt_...,
    .pick_next_task = pick_next_task_wrr,
    .put_prev_task = put_prev_task_wrr,

#ifdef CONFIG_SMP
    .select_task_rq = select_task_rq_wrr,
    .migrate_task_rq = migrate_task_rq_wrr,
    .set_cpus_allowed = set_cpus_allowed_common,
    .rq_online = rq_online_wrr,
    .rq_offline = rq_offline_wrr,
    //.task_dead = task_dead_wrr,
    //.task_woken = task_woken_wrr,
    //.switched_from = switched_from_wrr,
#endif

    .set_curr_task = set_curr_task_wrr,
    .task_tick = task_tick_wrr,
    .task_fork = task_fork_wrr,
    .get_rr_interval = get_rr_interval_wrr,
    .switched_from = switched_from_wrr,
    .switched_to = switched_to_wrr,
    .update_curr = update_curr_wrr,
};
