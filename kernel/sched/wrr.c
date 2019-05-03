/*
 * Weighted Round Robin (WRR) Class (SCHED_WRR)
 */


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
const struct sched_class wrr_sched_class = {

    .task_tick = task_tick_wrr,
}
