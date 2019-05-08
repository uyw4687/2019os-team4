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
#define WRR_LB_TIMESLICE 2 * HZ

int sched_wrr_timeslice = WRR_TIMESLICE;

const struct sched_class wrr_sched_class;

static void update_curr_wrr(struct rq *rq);

static void load_balance_wrr(struct rq *rq);

static inline bool task_is_wrr(struct task_struct *tsk)
{
    int policy = tsk->policy;

    if(policy == SCHED_WRR)
        return true;

    return false;
}

#define for_each_wrr_rq(wrr_rq, iter, rq) \
    for((void) iter, wrr_rq = &rq->wrr; wrr_rq; wrr_rq = NULL)

#define for_each_sched_wrr_entity(wrr_se) \
    for(; wrr_se; wrr_se = NULL)

/*
 * Change wrr_se->run_list location unless SAVE && !MOVE
 *
 * assumes ENQUEUE/DEQUEUE flags match
 */
static inline bool move_entity(unsigned int flags)
{
	if ((flags & (DEQUEUE_SAVE | DEQUEUE_MOVE)) == DEQUEUE_SAVE)
		return false;

	return true;
}

static inline
void inc_wrr_tasks(struct sched_wrr_entity *wrr_se, struct wrr_rq *wrr_rq)
{
	//int prio = rt_se_prio(rt_se);

	//WARN_ON(!rt_prio(prio));
	wrr_rq->wrr_nr_running += 1; // wrr_se_nr_running(wrr_se);
	//wrr_rq->rr_nr_running += rt_se_rr_nr_running(rt_se);

	//inc_rt_prio(rt_rq, prio);
	//inc_rt_migration(rt_se, rt_rq);
	//inc_rt_group(rt_se, rt_rq);
}

void init_wrr_rq(struct wrr_rq *wrr_rq)
{
#ifdef CONFIG_SMP
    wrr_rq->load.weight = 0;
#endif
    INIT_LIST_HEAD(&wrr_rq->queue);
    wrr_rq->curr = wrr_rq->next = wrr_rq->last = wrr_rq->skip = NULL;
	raw_spin_lock_init(&wrr_rq->wrr_runtime_lock);
    wrr_rq->next_load_balance = WRR_LB_TIMESLICE;
}

static inline struct task_struct *wrr_task_of(struct sched_wrr_entity *wrr_se)
{
    return container_of(wrr_se, struct task_struct, wrr);
}

static inline struct rq *rq_of_wrr_rq(struct wrr_rq *wrr_rq)
{
    return container_of(wrr_rq, struct rq, wrr);
}

static inline struct rq *rq_of_wrr_se(struct sched_wrr_entity *wrr_se)
{
    struct task_struct *p = wrr_task_of(wrr_se);

    return task_rq(p);
}

static inline struct wrr_rq *wrr_rq_of_se(struct sched_wrr_entity *wrr_se)
{
    struct rq *rq = rq_of_wrr_se(wrr_se);

    return &rq->wrr;
}

static inline int on_wrr_rq(struct sched_wrr_entity *wrr_se)
{
    return wrr_se->on_rq;
}

static void requeue_wrr_entity(struct wrr_rq *wrr_rq, struct sched_wrr_entity *wrr_se, int head)
{
    if(on_wrr_rq(wrr_se)) {
        struct list_head *queue = &wrr_rq->queue;

        if(head)
            list_move(&wrr_se->run_list, queue);
        else
            list_move_tail(&wrr_se->run_list, queue);
    }
}

static void requeue_task_wrr(struct rq *rq, struct task_struct *p, int head)
{
    struct sched_wrr_entity *wrr_se = &p->wrr;
    struct wrr_rq *wrr_rq;

    for_each_sched_wrr_entity(wrr_se) {
        wrr_rq = wrr_rq_of_se(wrr_se);
        requeue_wrr_entity(wrr_rq, wrr_se, head);
    }
}

static inline u64 sched_wrr_runtime(struct wrr_rq *wrr_rq)
{
	return wrr_rq->wrr_runtime;
}

static void enqueue_wrr_entity(struct sched_wrr_entity *wrr_se, unsigned int flags)
{
	struct wrr_rq *wrr_rq = wrr_rq_of_se(wrr_se);
	//struct rt_prio_array *array = &rt_rq->active;
	//struct rt_rq *group_rq = group_rt_rq(rt_se);
	struct list_head *queue = &wrr_rq->queue; //array->queue + rt_se_prio(rt_se);

	/*
	 * Don't enqueue the group if its throttled, or when empty.
	 * The latter is a consequence of the former when a child group
	 * get throttled and the current group doesn't have any other
	 * active members.
	 */
	/*
	if (group_rq && (rt_rq_throttled(group_rq) || !group_rq->rt_nr_running)) {
		if (rt_se->on_list)
			__delist_rt_entity(rt_se, array);
		return;
	}
	 */

	if (move_entity(flags)) {
		//WARN_ON_ONCE(rt_se->on_list);
		if (flags & ENQUEUE_HEAD)
			list_add(&wrr_se->run_list, queue);
		else
			list_add_tail(&wrr_se->run_list, queue);

		//__set_bit(rt_se_prio(rt_se), array->bitmap);
		//rt_se->on_list = 1;
	}
	wrr_se->on_rq = 1;

	inc_wrr_tasks(wrr_se, wrr_rq);
}

static void enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    // TODO fair.c 4879L / rt.c 1321L
	struct sched_wrr_entity *wrr_se = &p->wrr;

	if (flags & ENQUEUE_WAKEUP)
		wrr_se->timeout = 0;

    for_each_sched_wrr_entity(wrr_se)
	    enqueue_wrr_entity(wrr_se, flags);

	//if (!task_current(rq, p) && p->nr_cpus_allowed > 1)
	//	enqueue_pushable_task(rq, p);
}

static inline
void dec_wrr_tasks(struct sched_wrr_entity *wrr_se, struct wrr_rq *wrr_rq)
{
	//WARN_ON(!rt_prio(rt_se_prio(rt_se)));
	WARN_ON(!wrr_rq->wrr_nr_running);
	wrr_rq->wrr_nr_running -= 1;//rt_se_nr_running(rt_se);
	//rt_rq->rr_nr_running -= rt_se_rr_nr_running(rt_se);

	//dec_rt_prio(rt_rq, rt_se_prio(rt_se));
	//dec_rt_migration(rt_se, rt_rq);
	//dec_rt_group(rt_se, rt_rq);
}

static void __delist_wrr_entity(struct sched_wrr_entity *wrr_se)//, struct rt_prio_array *array)
{
	list_del_init(&wrr_se->run_list);
/*
	if (list_empty(array->queue + rt_se_prio(rt_se)))
		__clear_bit(rt_se_prio(rt_se), array->bitmap);
*/
	wrr_se->on_rq = 0;
}

static void __dequeue_wrr_entity(struct sched_wrr_entity *wrr_se, unsigned int flags)
{
	struct wrr_rq *wrr_rq = wrr_rq_of_se(wrr_se);
	//struct rt_prio_array *array = &rt_rq->active;

	if (move_entity(flags)) {
	//	WARN_ON_ONCE(!rt_se->on_list);
		__delist_wrr_entity(wrr_se);//, array);
	}
	wrr_se->on_rq = 0;

	dec_wrr_tasks(wrr_se, wrr_rq);
}

/*
 * Because the prio of an upper entry depends on the lower
 * entries, we must remove entries top - down.
 */
static void dequeue_wrr_stack(struct sched_wrr_entity *wrr_se, unsigned int flags)
{
	struct sched_wrr_entity *back = NULL;

	for_each_sched_wrr_entity(wrr_se) {
		wrr_se->back = back;
		back = wrr_se;
	}

	//dequeue_top_rt_rq(rt_rq_of_se(back));

	for (wrr_se = back; wrr_se; wrr_se = wrr_se->back) {
		if (on_wrr_rq(wrr_se))
			__dequeue_wrr_entity(wrr_se, flags);
	}
}

static void dequeue_wrr_entity(struct sched_wrr_entity *wrr_se, unsigned int flags)
{
	struct rq *rq = rq_of_wrr_se(wrr_se);

	dequeue_wrr_stack(wrr_se, flags);
    /*
	for_each_sched_wrr_entity(wrr_se) {
		struct rt_rq *rt_rq = group_rt_rq(rt_se);

		if (rt_rq && rt_rq->rt_nr_running)
			__enqueue_rt_entity(rt_se, flags);
	}
	enqueue_top_rt_rq(&rq->rt);
    */
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
    // TODO fair.c 4935L / rt.c 1334L
	struct sched_wrr_entity *wrr_se = &p->wrr;

	update_curr_wrr(rq);
	dequeue_wrr_entity(wrr_se, flags);

	//dequeue_pushable_task(rq, p);
}

static void yield_task_wrr(struct rq *rq)
{
    // TODO fair.c 6396L / rt.c 1373L
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
    struct sched_wrr_entity *wrr_se = &p->wrr;

    //TODO make update_curr_wrr
	update_curr_wrr(rq);

    if(p->policy != SCHED_WRR)
        return;

    if(--p->wrr.time_slice)
        return;

    p->wrr.time_slice = p->wrr.weight * sched_wrr_timeslice;

	/*
	 * Requeue to the end of queue if we (and all of our ancestors) are not
	 * the only element on the queue
	 */
    for_each_sched_wrr_entity(wrr_se) {
        if(wrr_se->run_list.prev != wrr_se->run_list.next) {
            requeue_task_wrr(rq, p, 0);
            resched_curr(rq);
            return;
        }
    }


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

/*
 * Update the current task's runtime statistics. Skip current tasks that
 * are not in our scheduling class.
 */
static void update_curr_wrr(struct rq *rq)
{
    // TODO fair.c 862L & 827L / rt.c 951L
	struct task_struct *curr = rq->curr;
	struct sched_wrr_entity *wrr_se = &curr->wrr;
	u64 delta_exec;

	if (curr->sched_class != &wrr_sched_class)
		return;

	delta_exec = rq_clock_task(rq) - curr->se.exec_start;
	if (unlikely((s64)delta_exec <= 0))
		return;

	/* Kick cpufreq (see the comment in kernel/sched/sched.h). */
	cpufreq_update_util(rq, SCHED_CPUFREQ_RT);

	schedstat_set(curr->se.statistics.exec_max,
		      max(curr->se.statistics.exec_max, delta_exec));

	curr->se.sum_exec_runtime += delta_exec;
	account_group_exec_runtime(curr, delta_exec);

	curr->se.exec_start = rq_clock_task(rq);
	cpuacct_charge(curr, delta_exec);

	//sched_wrr_avg_update(rq, delta_exec);

	//if (!rt_bandwidth_enabled())
	//	return;

	for_each_sched_wrr_entity(wrr_se) {
		struct wrr_rq *wrr_rq = wrr_rq_of_se(wrr_se);

		if (sched_wrr_runtime(wrr_rq) != RUNTIME_INF) {
			raw_spin_lock(&wrr_rq->wrr_runtime_lock);
			wrr_rq->wrr_time += delta_exec;
			//if (sched_wrr_runtime_exceeded(rt_rq))
			//	resched_curr(rq);
			raw_spin_unlock(&wrr_rq->wrr_runtime_lock);
		}
	}
}

/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags)
{

}

#ifdef CONFIG_SMP

static DEFINE_PER_CPU(cpumask_var_t, local_cpu_mask);

void __init init_sched_wrr_class(void)
{
	unsigned int i;

	for_each_possible_cpu(i) {
		zalloc_cpumask_var_node(&per_cpu(local_cpu_mask, i),
					GFP_KERNEL, cpu_to_node(i));
	}
}
#endif /* CONFIG_SMP */

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

static void find_busiest_freest_queue_wrr(struct rq *max_rq, struct rq *min_rq, int *max_weight, int *min_weight)
{
    int weight, cpu;
    int max = 0, min = 0;
    struct rq *rq;
    struct sched_wrr_entity *wrr_se;

    rcu_read_lock();

    for_each_online_cpu(cpu) {
        weight = 0;
        rq = cpu_rq(cpu);
        
        list_for_each_entry(wrr_se, &rq->wrr.queue, run_list) {
            weight += wrr_se->weight;
        }

        if(max < weight) {
            max = weight;
            max_rq = rq;
        }

        if(min == 0 || min > weight) {
            min = weight;
            min_rq = rq;
        }
    }

    rcu_read_unlock();

    *max_weight = max;
    *min_weight = min;
}

static void reset_lb_timeslice(void) {
    int cpu;
    struct rq *rq;

    for_each_online_cpu(cpu) {
        rq = cpu_rq(cpu);
        rq->wrr.next_load_balance = WRR_LB_TIMESLICE;
    }
}

static void load_balance_wrr(struct rq *rq)
{
    struct rq *busiest;
    struct rq *freest;
    struct sched_wrr_entity *wrr_se;
    struct task_struct *task;
    int max_weight;
    int min_weight;
    int diff;

    if(--rq->wrr.next_load_balance)
        return;
    


    else {
        reset_lb_timeslice();
        raw_spin_unlock(&rq->wrr.lb_lock);
    }

    find_busiest_freest_queue_wrr(busiest, freest, &max_weight, &min_weight);

    if(max_weight == min_weight)
        return;

    diff = max_weight - min_weight;

    double_rq_lock(busiest, freest);

    list_for_each_entry(wrr_se, &busiest->wrr.queue, run_list) {
        if(wrr_se->weight <= diff/2 && !task_on_rq_queued(task)) {
            task = wrr_task_of(wrr_se);
            break;
        }
    }

    dequeue_task_wrr(busiest, task, 1);
    enqueue_task_wrr(freest, task, 1);

    double_rq_unlock(busiest, freest);
}

const struct sched_class wrr_sched_class = {
    .next = &fair_sched_class,
    .enqueue_task = enqueue_task_wrr,
    .dequeue_task = dequeue_task_wrr,
    .yield_task = yield_task_wrr,
    //.yield_to_task = yield_to_task_wrr,
    .check_preempt_curr = check_preempt_curr_wrr,

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
    .switched_to = switched_to_wrr,
    .switched_from = switched_from_wrr,
    .update_curr = update_curr_wrr,
};
