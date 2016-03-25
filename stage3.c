#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

#define REPEATS	10

void dummy(void)
{
	printf(1, "TEST FAILED: this should never execute.\n");
}

int handle_signal(int signum)
{
	static int counter;

	counter++;
	printf(1, "%d\n", counter);

	if(counter >= REPEATS){
		// Skip the instruction causing SIGFPE
	}

	return -1;
}

int main(int argc, char *argv[])
{
	// int start = uptime();

	int x = 5;
	int y = 0;

	signal(SIGFPE, handle_signal);

	while(1);
		x = x / y;
	
	printf(1, "Traps Performed: XXXX\n");
	printf(1, "Total Elapsed Time: XXXX\n");
	printf(1, "Average Time Per Trap: XXXXX\n");

	exit();
}