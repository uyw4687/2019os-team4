#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_ROTLOCK_WRITE 400
#define SYS_ROTUNLOCK_WRITE 402

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("a starting number as the only argument\n");
		exit(1);
	}
	
	char *end;
	int value;
	
	value = strtol(argv[1], &end, 10);
	
	FILE *fp;

	//writing integer input
	//starting lock
	syscall(SYS_ROTLOCK_WRITE, 90, 90);

	fp = fopen("integer", "w");
	fprintf(fp, "%d", value++);
	
	fclose(fp);

	syscall(SYS_ROTUNLOCK_WRITE, 90, 90);
	//lock ended

	while(1) {
		//writing +1
		//starting lock
		syscall(SYS_ROTLOCK_WRITE, 90, 90);
	
		fp = fopen("integer", "w");
		fprintf(fp, "%d", value++);
		
		fclose(fp);
	
		printf("selector: %d\n", value-1);

		syscall(SYS_ROTUNLOCK_WRITE, 90, 90);
		//lock ended
	}

	return 0;
}
