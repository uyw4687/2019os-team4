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
	int success; 

	//writing integer input
	//starting lock
	success = syscall(SYS_ROTLOCK_WRITE, 90, 90);

	if(success < 0) {
		printf("rotlock_write failed\n");
		exit(1);
	}

	fp = fopen("integer", "w");

	if(!fp) {
		printf("fopen failed\n");
		exit(1);
	}

	fprintf(fp, "%d", value++);
	
	fclose(fp);

	success = syscall(SYS_ROTUNLOCK_WRITE, 90, 90);
	//lock ended

	if(success < 0) {
		printf("rotunlock_write failed\n");
		exit(1);
	}

	while(1) {
		//writing +1
		//starting lock
		success = syscall(SYS_ROTLOCK_WRITE, 90, 90);

		if(success < 0) {
			printf("rotlock_write failed\n");
			exit(1);
		}
	
		fp = fopen("integer", "w");

		if(!fp) {
			printf("fopen failed");
			exit(1);
		}

		fprintf(fp, "%d", value++);
		
		fclose(fp);
	
		printf("selector: %d\n", value-1);

		success = syscall(SYS_ROTUNLOCK_WRITE, 90, 90);
		//lock ended

		if(success < 0) {
			printf("rotunlock_write failed\n");
			exit(1);
		}
	
	}

	return 0;
}
