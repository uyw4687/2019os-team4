#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/prinfo.h> // change this before use

#define sys_ptree 398


struct stack {
	struct prinfo data[100];
	int top;
};

void Init_stack(struct stack *st);
void pop(struct stack *st);
void push(struct stack *st, struct prinfo p);
struct prinfo pick(struct stack st);

int main(int argc, char *argv[]) {

	int result;

	struct prinfo *buf;
	int *nr;
	
	struct prinfo p;
	
	
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


	//print ptree
	struct stack st;
	Init_stack(&st);
	
	for(int i = 0 ; i < *nr ; i++) {
		
		p = buf[i];
		
		if(pick(st).pid == p.pid) pop(&st);
		else if(p.first_child_pid != 0) push(&st, p);
		
		for(int i=0 ; i < st.top ; i++) printf("	");
		printf("%s,%d,%lld,%d,%d,%d,%lld\n", p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);

	}

	free(nr);
	free(buf);

}

void Init_stack(struct stack *st){
	struct prinfo p;
	p.pid = 0;
	p.parent_pid = 0;
	p.first_child_pid = 0;
	p.next_sibling_pid = 0;
	st->data[0] = p;
	st->top = -1;
}

void push(struct stack *st, struct prinfo p){
	st->top++;
	st->data[st->top] = p;
}

void pop(struct stack *st){
	st->top--;
}

struct prinfo pick(struct stack st){
	return st.data[st.top];
}
