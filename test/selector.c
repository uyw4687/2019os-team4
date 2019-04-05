#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int notFinished = 1;

void term(int signum)
{
	notFinished = 0;
}

int main(int argc, char **argv)
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = term;
	if(sigaction(SIGTERM, &action, NULL) == -1)
		exit(-1);

	if(argc != 2)
	{
		printf("starting number as the only argument");
		exit(1);
	}
	
	char *end;
	int value;
	
	value = strtol(argv[1], &end, 10);
	
	FILE *fp;

	//writing integer input
	//starting lock
	rotlock_write(90, 90);

	fp = fopen("integer", "w");
	fprintf(fp, "%d", value);
	
	fclose(fp);

	rotunlock_write(90, 90);
	//lock ended

	//writing 
	printf("selector: %d\n", value);
	return 0;
}
