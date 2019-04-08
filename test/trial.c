#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_ROTLOCK_READ 399
#define SYS_ROTUNLOCK_READ 401

void factorize(int value)
{
	int divisor;
	int dividend = value;
	int end;

	for(divisor=2;divisor<=value;divisor++)
	{
		while(dividend % divisor == 0)
		{
			printf("%d", divisor);

			dividend /= divisor;
			
			if(dividend != 1)
				printf(" * ");
			else
				end = 1;
		}
		if(end == 1)
		{
			printf("\n");
			break;
		}
	}
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("an identifier integer as the only argument\n");
		exit(1);
	}
	
	//for strtol
	char *end;
	int identifier;
	int value;
	
	identifier = strtol(argv[1], &end, 10);
	
	FILE *fp;

	while(1) {
		//read integer & get value
		//starting lock
		syscall(SYS_ROTLOCK_READ, 90, 90);
	
		fp = fopen("integer", "r");
		if(!fp)
		{
			printf("null file pointer\n");
			return 1;
		}
	
		fscanf(fp, "%d", &value);
		
		printf("trial-%d: %d = ", identifier, value);
fflush(stdout);
		factorize(value);

		fclose(fp);

		syscall(SYS_ROTUNLOCK_READ, 90, 90);
		//lock ended
	}

	return 0;
}
