#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

void dummy(void)
{
	printf(1, "TEST FAILED: this should never execute.\n");
}

void handle_signal(int signum)
{
	static int counter;
	counter++;
	printf(1, "%d\n", counter);
}

int main(int argc, char *argv[])
{
	int x = 5;
	int y = 0;

	signal(SIGFPE, handle_signal);

	int i = 0;
	for(i = 0; i < 10; i++)
		x = x / y;
	
	printf(1, "Traps Performed: XXXX\n");
	printf(1, "Total Elapsed Time: XXXX\n");
	printf(1, "Average Time Per Trap: XXXXX\n");

	exit();
}