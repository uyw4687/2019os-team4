# os-team4
OS Spring Team4
## Project 3

### How to build our kernel
* project 기본 build 방법으로 하시면 됩니다.
* 또는 `./onetime.sh`를 실행해도 됩니다.

### High-level design and implementation
#### System call registration
* `kernel/sched/core.c`에 두 system call `sched_setweight`, `sched_getweight`를 정의하고, 시스템 콜 등록 과정을 따라 등록하였습니다.

##### `sched_setweight`
* 인자로 프로세스 `pid`와 바꿀 `weight`를 받습니다. 
* 주어진 `weight`는 1 이상 20 이하여야 합니다. 그렇지 않으면 `-1`을 반환합니다.
* 만약 주어진 `pid`가 0이면 이 시스템 콜을 호출한 프로세스의 weight를 조정합니다.
* 주어진 `pid`의 task가 존재하지 않으면 `-EINVAL`을 반환합니다.
* 해당 task의 scheduling policy가 WRR이 아니면 `-1`을 반환합니다.
* root 유저 또는 해당 프로세스를 소유한 유저만 호출할 수 있습니다. 그렇지 않으면 `-EPERM`을 반환합니다.
* root가 아닌 유저는 `weight`를 올릴 수 없으며, 올리려고 하면 `-EPERM`을 반환합니다.
* 실행에 문제가 없으면 해당 task의 `weight`를 변경합니다. 만약 해당 task가 현재 돌아가는 task가 아니라면 해당 task에 할당되는 `time_slice` 길이도 변경합니다.
* 실행에 문제가 있어 중간에 반환할 때, lock을 잡고 있는 상태이면 lock을 놓습니다.

#### `sched_getweight`
* 만약 주어진 `pid`가 0이면 이 시스템 콜을 호출한 프로세스의 weight를 조정합니다.
* 주어진 `pid`의 task가 존재하지 않으면 `-EINVAL`을 반환합니다.
* 해당 task의 scheduling policy가 WRR이 아니면 `-1`을 반환합니다.
* 실행에 문제가 없으면 해당 task의 `weight`를 반환합니다.
* 실행에 문제가 있어 중간에 반환할 때, lock을 잡고 있는 상태이면 lock을 놓습니다.

#### Add a new scheduling policy
> Weighted Round Robin(WRR)

* `include/uapi/linux/sched.h`
  * `SCHED_WRR`의 값을 7로 하여 정의

* `tools/include/uapi/linux/sched.h`
  * `SCHED_WRR`의 값을 7로 하여 정의

* `include/linux/sched.h`
  * `sched_wrr_entity` 구조체 정의
  * `task_struct` 안에 `struct sched_wrr_entity` 타입의 멤버 변수 정의

* `kernel/sched/core.c`
  * task의 `sched_wrr_entity` 초기화 (`__sched_fork` 함수에서)
  * task의 policy가 `SCHED_WRR`이면 `wrr_sched_class`를 사용하도록 함 (`sched_fork` 함수에서)
  * 설정할 scheduler의 policy가 `SCHED_WRR`이면 `wrr_sched_class`를 사용하도록 함 (`__setscheduler` 함수에서)
  * `sched_init_smp` 함수에서 `init_sched_wrr_class` 함수를 호출하도록 함
  * `sched_init` 함수에서 `init_wrr_rq` 함수를 호출하도록 함

* `kernel/sched/rt.c`
  * RT scheduler보다 우선순위가 낮은 바로 다음 scheduler가 WRR이 되도록 설정

* `kernel/sched/sched.h`
  * WRR도 `valid_policy` 중 하나로 인정되도록 함
  * `wrr_rq` 구조체 정의
  * `rq` 구조체 안에 `struct wrr_rq` 타입의 멤버 변수 정의

##### `kernel/sched/wrr.c`
* 

##### Load balancing
* `kernel/sched/core.c`
  * `scheduler_tick` 함수에서 WRR의 load balancing을 수행하도록 함

* TODO `include/linux/sched/sysctl.h` 안에 있는 `extern int sched_wrr_timeslice`

You should provide a complete set of results that show all your tests. If there are any results that do not yield execution time proportional to weights, explain why. Your results and any explanations should be put in the README.md file in the project branch of your team's repository. Your plot should be named plot.pdf and should be put next to the README.md file.
