# os-team4
## Project 3

### Improve the WRR scheduler
* aging 도입
  * weight가 작은 상태로 오래 돌아가는 task가 있다면, 이 task의 weight를 조금씩 높여서 이 task가 적절하게 CPU를 사용할 수 있도록 합니다.
  * 이를 구현하기 위해 각 task가 WRR policy에서 CPU를 사용한 시간을 추적해야 합니다.
  * 이것을 구현하면 fairness를 향상시킬 수 있습니다.

* WRR보다 우선순위가 낮은 CFS로 돌아가는 task가, WRR task가 돌아가고 있는 CPU에서도 머지않은 미래에 실행될 수 있음을 보장하도록 구현합니다.
  * 현재는 이것이 구현되어 있지 않기 때문에 임시로 하나의 CPU(3번 CPU)에서 WRR task가 돌아가지 않도록 하였습니다. 만약 모든 CPU에서 WRR task가 돌아간다면 모든 CFS task는 돌아가지 못하게 되고, 심각한 starvation을 겪게 됩니다. 특히 kernel thread 중에 CFS로 돌아가는 task가 많이 있기 때문에, 이들이 starvation을 겪으면 시스템 전체가 다운될 수 있습니다.
  * 이를 해소하기 위한 구현 방법을 3가지 제시합니다.
    * WRR task가 돌아가고 있는 CPU에서 대기하고 있는 CFS task가 있다면, 이 CFS task의 policy를 적절한 시간 안에 WRR로 바꿔줍니다.
    * 위와 마찬가지로 대기하고 있는 CFS task가 있다면, 잠깐 동안 CFS policy가 WRR policy보다 높은 우선순위를 가질 수 있도록 구현합니다.
    * 위와 마찬가지로 대기하고 있는 CFS task가 있다면, `pick_next_task_wrr` 함수에서 WRR task를 반환하지 않고 NULL을 반환하게 합니다.

* 우리의 구현에서 round robin을 할 때와 load balancing을 할 때 거는 lock을 조금 더 fine-grained하게 변경합니다.
  * 현재는 `enqueue_task_wrr`, `dequeue_task_wrr`, `task_tick_wrr`, `load_balance_wrr` 등 여러 함수에서 `wrr_lock`이라는 하나의 lock을 잡습니다. 이들 중에는 매 tick마다 호출되는 함수도 있습니다.
  * 또한 현재는 두 개의 run queue만을 변경하는 상황에서도 모든 run queue에 대해 lock을 잡는 경우가 있습니다.
  * 불필요하게 lock을 잡고 있는 시간을 줄이면 lock contention이 해소되어 더 빠르게 동작할 수 있을 것입니다.
