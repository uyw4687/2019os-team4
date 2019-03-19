#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/prinfo.h> // change this before use

#define sys_ptree 398

int main(int argc, char *argv[]) {

	int result;

	struct prinfo *buf;
	int *nr;
	
	struct prinfo p;
	
	int i;

	if (argc < 2) {
		printf("nr needed");
		return -1;
	}
	
	nr = (int*)malloc(sizeof(int));

	if(nr != NULL)
		*nr = atoi(argv[1]);	
	else
		return 1;
	
	buf = (struct prinfo *)malloc(sizeof(struct prinfo)*(*nr));

	if(!buf)
		return 1;		

	errno = 0;

	result = syscall(sys_ptree, buf, nr);

	printf("%d", result);

	if(result == -EINVAL) {

		perror("Invalid argument");
        return -1;

	} else if (result == -EFAULT) {

 		perror("Bad address");
		return -1;
	}
	
	for(i = 0 ; i < *nr ; i++) {
		
		p = buf[i];	
		
		printf("%s,%d,%lld,%d,%d,%d,%lld\n", p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);

	}

	free(nr);
	free(buf);

}
