#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/prinfo.h>

#define sys_ptree 398


struct stack {
    struct prinfo data[100];
    int top;
};

struct prinfo empty = {
    .pid = -1
};

//define stack, stack function
void init(struct stack *st);
struct prinfo pop(struct stack *st);
void push(struct stack *st, struct prinfo p);
struct prinfo peek(struct stack st);

//define print function
void print(struct prinfo p, int top);

int main(int argc, char *argv[]) {

    int result;

    struct prinfo *buf;
    int *nr;
    
    struct prinfo p;
    int i;
    int err;

    if (argc < 2) {
        printf("nr needed\n");
        return -1;
    }
    
    nr = (int*)malloc(sizeof(int));

    if (nr != NULL) {
        *nr = atoi(argv[1]);
    }
    else {
        perror("Malloc is failed");
        return -1;
    }

    
    buf = (struct prinfo *)malloc(sizeof(struct prinfo)*(*nr));

    if (!buf) {
        perror("Malloc is failed");
        return -1;
    }

    result = syscall(sys_ptree, buf, nr);
    err = errno;

    if (result == -1) {
        if (err == EINVAL){
            perror("Invalid argument");
            return -1;
        }

        else if (err == EFAULT) {
            perror("Bad address");
            return -1;
        }

        else {
            printf("errno is %d\n", err);
            perror("Undefined error happened!");
            return -1;
        }
    
    }

	//print ptree
	struct stack st;
	init(&st);
    
    if (nr > 0) {
	    p = buf[0];
	    push(&st, p);
	    print(p, st.top);
    }
    
	for (i = 1; i < *nr; i++) {
		
		p = buf[i];
		
        while (peek(st).pid != p.parent_pid) pop(&st);
        if (p.first_child_pid != 0) {
            push(&st,p);
            print(p,st.top);
        }
        else if (peek(st).pid == p.parent_pid){
            print(p,st.top+1);
        }
        else {
            print(p,st.top+1);
        }
	}
    
    printf("system call returns %d\nnr is %d\n", result, *nr);

    free(nr);
    free(buf);

}

void init(struct stack *st) {
    st->top = -1;
}


void push(struct stack *st, struct prinfo p) {
    if (st->top >= 99) return;  // stack is full!
    st->top++;
    st->data[st->top] = p;
}


struct prinfo pop(struct stack *st) {
    if (st->top < 0) return empty;  // stack is empty!
    st->top--;
    return st->data[st->top + 1];
}

struct prinfo peek(struct stack st) {
    if (st.top < 0) return empty;  // stack is empty!
    return st.data[st.top];
}

void print(struct prinfo p,int t){
    for(; t > 0; t--)
        printf("    ");
    printf("%s,%d,%lld,%d,%d,%d,%lld\n", p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
}
