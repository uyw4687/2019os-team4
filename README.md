1. 점수 : 35/70 (보너스 문제 포함: 45/80)
2. 감점 사유:
* Run queue 하나가 empty 상태로 유지되지 않음 (-25)
* 새 task가 smallest total weight를 가진 run queue로 가지 않음 (-5)
* WRR인 task에 weight을 정상적으로 set할 수 없음 (sched_setweight 결함) (-5)
3. 통계 (괄호 안의 숫자는 보너스 문제 포함한 경우):
* 평균 : 43.11 (50.89)
* 표준편차 : 23.48 (27.47)
* Q1 (하위 25%) : 36.25 (46.25)
* Q2 (중간값) : 55 (65)
* Q3 (상위 25%) : 58.75 (68.75)

# os-team4
OS Spring Team4
## Project 3

### How to build our kernel
* project 기본 build 방법으로 하시면 됩니다.
* 또는 `./onetime.sh`를 실행해도 됩니다. 이렇게 하면 build 후에 image 파일이 생성되고 압축되는 과정까지 완료됩니다.

### High-level design and implementation
#### System call registration
* `kernel/sched/core.c`에 두 system call `sched_setweight`, `sched_getweight`를 정의하고, 시스템 콜 등록 과정을 따라 등록하였습니다.

* `sched_setweight`
  * 인자로 프로세스 `pid`와 바꿀 `weight`를 받습니다. 
  * 주어진 `weight`는 1 이상 20 이하여야 합니다. 그렇지 않으면 `-1`을 반환합니다.
  * 만약 주어진 `pid`가 0이면 이 시스템 콜을 호출한 프로세스(task)를 다룹니다.
  * 주어진 `pid`의 task가 존재하지 않으면 `-EINVAL`을 반환합니다.
  * 해당 task의 scheduling policy가 WRR이 아니면 `-1`을 반환합니다.
  * root 유저 또는 해당 프로세스를 소유한 유저만 호출할 수 있습니다. 그렇지 않으면 `-EPERM`을 반환합니다.
  * root가 아닌 유저는 `weight`를 올릴 수 없으며, 올리려고 하면 `-EPERM`을 반환합니다.
  * 실행에 문제가 없으면 해당 task의 `weight`를 변경합니다. 만약 해당 task가 현재 돌아가는 task가 아니라면 해당 task에 할당되는 `time_slice` 길이도 변경합니다.
  * 실행에 문제가 있어 중간에 반환할 때, lock을 잡고 있는 상태이면 lock을 놓습니다.

* `sched_getweight`
  * 만약 주어진 `pid`가 0이면 이 시스템 콜을 호출한 프로세스(task)를 다룹니다.
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
  * RT scheduler보다 우선순위가 낮은 바로 다음 scheduler가 WRR scheduler가 되도록 설정

* `kernel/sched/sched.h`
  * WRR도 `valid_policy` 중 하나로 인정되도록 함
  * `wrr_rq` 구조체 정의
  * `rq` 구조체 안에 `struct wrr_rq` 타입의 멤버 변수 정의

* `kernel/sched/wrr.c`
  * `struct sched_class` 타입의 `wrr_sched_class` 정의
    * WRR scheduler보다 우선순위가 낮은 바로 다음 scheduler가 fair(CFS) scheduler가 되도록 설정
    * 각종 함수 포인터의 값 설정 및 해당 함수 구현
  * run queue를 초기화하는 `init_wrr_rq` 함수 정의
  * `init_sched_wrr_class` 함수 정의

#### Implemented functions in WRR
> `kernel/sched/wrr.c`에 구현

* `enqueue_task_wrr`
  * 해당 task를 run queue에서 제거한 후, 이 task를 run queue 맨 뒤에 추가
  * 해당 task가 fork된 task이면 부모의 weight를 물려받음

* `dequeue_task_wrr`
  * 해당 task를 run queue에서 제거

* `pick_next_task_wrr`
  * run queue의 다음 task를 반환

* `task_tick_wrr`
  * *round robin을 수행하는 함수*
  * 인자로 주어진 task의 policy가 WRR가 아니면 반환
  * 이 함수가 호출될 때마다 해당 task의 `time_slice`를 1씩 감소시킴
  * 만약 해당 task의 `time_slice`가 0이면
    * `time_slice`를 `weight * 10ms`로 설정
    * 해당 task가 run queue에 혼자 들어 있으면 round robin을 수행할 필요가 없으므로 반환
    * 해당 task가 run queue에 혼자 들어 있지 않으면 이 task를 run queue의 맨 뒤로 옮기고 해당 run queue의 맨 앞의 task를 수행하도록 함

* `select_task_rq_wrr`
  * online CPU 중에서 3번 CPU를 제외하고, 가장 weight 합이 적은 run queue가 있는 CPU를 반환

* `update_curr_wrr`
  * 현재 run queue 안에서 수행되고 있는 task의 수행 시간 등 통계량 업데이트

* `get_rr_interval_wrr`
  * 인자로 주어진 `task`의 policy가 WRR이면 이 `task`에 할당된 timeslice 길이를 반환

#### Load balancing
* `kernel/sched/core.c`
  * `scheduler_tick` 함수에서 `load_balance_wrr` 함수를 호출하도록 함

* `kernel/sched/wrr.c`
  * `load_balance_wrr` 함수
    * 다음 load balance가 일어나는 시간이 되지 않았으면 반환
    * load balance를 수행하기 전에, 네 CPU의 run queue에 대해 다음 load balance가 일어나는 시간(현재로부터 2초 후)을 계산
    * online 상태인 CPU들의 각 run queue 안에 들어 있는 `sched_wrr_entity`들의 weight 합을 계산해, 가장 weight 합이 큰 run queue와 가장 weight 합이 작은 run queue를 찾음
    * 만약 위에서 찾은 두 run queue의 weight 합 차이(`diff`)가 2보다 크면, weight 합이 가장 큰 run queue 안에 들어 있는, 현재 실행 중이지 않고 `weight`가 `(diff+1)/2`보다 작은 task들 중에서 가장 `weight`가 큰 task를 찾음
    * 만약 위에서 적절한 task를 찾으면, 해당 task를 weight 합이 가장 작은 run queue로 migrate함

#### Synchronization
* `task_struct`를 읽어야 할 때 `rcu_read_lock`과 `rcu_read_unlock` 사용
* `sched_setweight` 시스템 콜에서 `task_struct`에 값을 쓸 때 `write_lock(&tasklist_lock)`과 unlock 사용
* `weight`가 변경될 때, load balance가 수행될 때, round robin이 수행될 때, enqueue가 수행될 때, 그리고 dequeue가 수행될 때 `raw_spin_lock(&wrr_lock)`과 unlock 사용
* deadlock이 발생하지 않도록 신중하게 lock을 사용하여 구현함

### Investigation
#### Test program
* [./test/ok/loopandfactor](./test/ok/loopandfactor) 로 테스트를 진행하였습니다.
  * 실행하면 이 프로세스를 WRR로 돌립니다. (WRR이 구현된 환경에서만 돌릴 수 있습니다.)
  * 그리고 1부터 17 사이의 숫자를 입력하여 원하는 명령을 수행할 수 있습니다.
    * 1을 입력하면 sleep할 시간을 추가로 입력받습니다. 그리고 이 시간만큼 sleep합니다.
    * 2를 입력하면 반복문을 돌릴 횟수를 추가로 입력받습니다. 그리고 아무 일도 하지 않고 정해진 횟수 동안 반복문을 돕니다.
    * 3을 입력하면 반복문을 돌릴 횟수와 출력 주기를 입력받습니다. 그리고 반복문을 돈 횟수가 출력 주기로 나누어 떨어질 때마다 상태를 출력하면서 정해진 횟수 동안 반복문을 돕니다.
    * 4를 입력하면 weight를 변경할 프로세스의 pid와 변경할 weight를 입력받습니다. 그리고 `sched_setweight` 시스템 콜을 호출하여 해당 프로세스의 weight를 변경합니다.
    * 5를 입력하면 weight를 알고 싶은 프로세스의 pid를 입력받습니다. 그리고 `sched_getweight` 시스템 콜을 호출하여 해당 프로세스의 weight 값을 출력합니다.
    * 6을 입력하면 이 프로세스를 한 번 fork한 후, 부모 프로세스와 자식 프로세스의 weight 값을 각각 출력하고 자식 프로세스를 종료합니다.
    * 7을 입력하면 이 프로세스가 0번, 1번, 2번 CPU에서만 돌아갈 수 있도록 합니다.
    * 8을 입력하면 어떤 scheduler로 돌아가는지 알고 싶은 프로세스의 pid를 입력받습니다. 그리고 `sched_getscheduler` 시스템 콜을 호출하여 해당 프로세스가 돌아가는 scheduler policy 번호를 출력합니다. (WRR의 경우 7)
    * 9를 입력하면 3을 소인수분해하는 작업을 1번 수행하고 이 프로세스를 종료합니다.
    * 10을 입력하면 이 프로세스를 종료합니다.
    * 11을 입력하면 줄 바꿈 문자를 여러 번 출력하여 화면을 깔끔하게 비웁니다.
    * 12를 입력하면 0 이상 3 이하의 값을 추가로 입력받습니다. 이 프로세스는 방금 입력받은 번호의 CPU에서만 돌아가게 됩니다.
    * 13을 입력하면 0 이상 3 이하의 값을 추가로 입력받습니다. 이 프로세스는 방금 입력받은 번호의 CPU에서도 돌아갈 수 있게 됩니다.
    * 14를 입력하면 3을 소인수분해하면서 소인수분해가 완료될 때마다 소인수분해에 걸린 시간을 출력하는 작업을 무한히 반복 수행합니다.
    * 15를 입력하면 소인수분해할 수를 추가로 입력받습니다. 그리고 방금 입력받은 수를 소인수분해하면서 소인수분해가 완료될 때마다 소인수분해에 걸린 시간을 출력하는 작업을 무한히 반복 수행합니다.
    * 16을 입력하면 반복문을 돌릴 횟수를 추가로 입력받습니다. 그리고 방금 입력받은 수만큼 반복문을 돌리면서 반복문을 완료할 때마다 반복에 걸린 시간을 출력하는 작업을 무한히 반복 수행합니다.
    * 17을 입력하면 소인수분해할 수를 추가로 입력받습니다. 그리고 이 프로세스의 weight를 1부터 20까지 차례로 바꾸면서 각각으로 방금 입력받은 수를 소인수분해하고 소인수분해에 걸린 시간을 출력하는 작업을 무한히 반복 수행합니다.
  * Demo 영상에서는 다음의 명령들을 입력한 프로세스들을 동시에 돌렸습니다.
    * 프로세스 1: `12 0 4 0 20 16 200000000` -> 0번 CPU에서 weight를 20으로 하여 200000000번 아무 일도 하지 않는 반복문을 돌고 수행 시간을 출력하는 작업을 무한히 반복 수행
    * 프로세스 2: `12 0 4 0 20 16 200000000` -> 프로세스 1과 같은 작업을 무한히 반복 수행
    * 프로세스 3: `12 0 17 10000019` -> 0번 CPU에서 weight를 1부터 20까지 바꾸면서 각각으로 10000019를 소인수분해하고 수행 시간을 출력하는 작업을 무한히 반복 수행

#### Analysis & Result

* Prior Analysis:

     ![](./data.png)

* Result: 끝에 non increasing order에서 벗어나는 부분이 있는데 이 부분은 이 cpu에서 real time process 등 더 우선순위가 높은 것이 실행되었을 때 발생할 수 있는 상황입니다.

    ![](./result.png)

* Plot: [plot.pdf](./plot.pdf) 를 참조하십시오.

### Lessons learned
* 기존에 kernel에서 돌아가던 scheduler인 RT(real-time)와 fair(CFS)의 코드를 읽고, 새 scheduler인 WRR을 구현하기 위해 어떤 함수가 반드시 구현되어야 하는지, 구현하지 않아도 되는 함수는 무엇인지 고민해 보았습니다.
* 새 scheduler를 추가하려면 무엇을 해야 하는지 알게 되었습니다.
* round robin이 어떻게 동작하는지 이해하고 직접 구현해 보았습니다.
* load balance가 어떻게 동작하는지 이해하고 직접 구현해 보았습니다.
* round robin이나 load balance가 수행되는 도중에 해당 task를 수정하는 경우가 없도록 잘 synchronize하기 위해 많은 고민을 하였습니다.
* WRR로 돌아가던 task가 fork를 수행할 경우, 자식 task도 WRR로 돌아가도록 구현하였습니다.
* 최소 하나 이상의 CPU에서 WRR로 돌아가는 task가 없도록 해야 하는 이유를 알았습니다. WRR이 fair(CFS)보다 높은 우선순위를 가지기 때문에 WRR로 돌아가는 task가 하나라도 있으면 그 CPU에서는 fair로 돌아가야 할 task가 절대로 수행되지 않습니다. 이 때문에 모든 CPU에서 WRR로 돌아가는 task가 있는 경우, kernel thread 중 fair로 돌아가야 할 thread들이 starvation을 겪게 되고 시스템이 다운되는 것입니다.
* WRR이 CFS보다 높은 우선순위를 가지고 돌아가야 하는 이유를 생각해 보았습니다. 대부분의 task가 CFS에서 돌아가기 때문에 WRR의 우선순위가 CFS보다 낮을 경우 CPU가 CFS task를 우선적으로 수행합니다. 그러면 우리가 만든 WRR scheduler가 제대로 동작하는지 계속해서 관찰하는 것이 어렵습니다. 따라서 WRR의 우선순위를 CFS보다 높게 준 것입니다.
* 유저에 따라 실행 권한을 다르게 부여하는 방법을 알았습니다.
* `pr_err`를 이용하여 상태를 출력하게 한 덕분에 디버깅이 훨씬 수월해졌습니다.
* QEMU를 설치하고 어떻게 사용해야 하는지 익혔습니다. QEMU를 사용하지 않고 직접 기기에 kernel을 올려서 테스트했다면 시간이 매우 오래 걸렸을 것입니다.

