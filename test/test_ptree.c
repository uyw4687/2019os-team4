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
struct prinfo pop(struct stack *st);
void push(struct stack *st, struct prinfo p);
struct prinfo peek(struct stack st);
//define stack, stack function
void print(struct prinfo p, int top);
//define print function

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


	//print ptree
	struct stack st;
	st.top = -1;

	for(i = 0 ; i < *nr ; i++) {
		
		p = buf[i];
		
        if(st.top == -1) {
            push(&st,p);
            print(p,st.top);
        }
        else if(p.first_child_pid != 0) {
            push(&st,p);
            print(p,st.top);
        }
        else if(peek(st).pid == p.parent_pid){
            print(p,st.top+1);
        }//case sibling
        else{
            while(peek(st).pid != p.parent_pid) pop(&st);
            print(p,st.top+1);
        }
	}

	free(nr);
	free(buf);

}

void push(struct stack *st, struct prinfo p){
    st->top++;
    st->data[st->top] = p;
}

struct prinfo pop(struct stack *st){
    return st->data[st->top];
    st->top--;
}

struct prinfo peek(struct stack st){
	return st.data[st.top];
}

void print(struct prinfo p,int t){
    for(t ; t > 0; t--)printf("    ");
    printf("%s,%d,%lld,%d,%d,%d,%lld\n", p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
}
