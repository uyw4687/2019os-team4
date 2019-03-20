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

struct prinfo empty = {
    .pid = -1
};

void init(struct stack *st);
struct prinfo pop(struct stack *st);
void push(struct stack *st, struct prinfo p);
struct prinfo peek(struct stack st);

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

    if (result == -EINVAL) {

        perror("Invalid argument");
        return -1;

    } else if (result == -EFAULT) {

         perror("Bad address");
        return -1;
    }


    //print ptree
    struct stack st;
    
    for (i = 0; i < *nr; i++) {
        
        p = buf[i];
        
        if (peek(st).pid == p.next_sibling_pid) pop(&st);
        else if (p.first_child_pid != 0) push(&st, p);

        int j;
        for (j = 0; j < st.top; j++) printf("\t");
        printf("%s,%d,%lld,%d,%d,%d,%lld\n", p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);

    }

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
