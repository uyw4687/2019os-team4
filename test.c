#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>

#define SYS_ptree 398
#define NR 500

int main(int argc, char *argv[]) {
	
	struct prinfo *buf;
	int *nr;
		
	nr = (int*)malloc(sizeof(int));
	*nr = NR;	

	buf = (struct prinfo *)malloc(sizeof(struct prinfo)*(*nr));
	
	pid_t tid;
	tid = syscall(SYS_ptree, buf, nr);
	printf("%s,%d,%lld,%d,%d,%d,%lld\n", p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);

}
