#include "types.h"
#include "stat.h"
#include "user.h"
#include "signal.h"

#define REPEATS	1000000 // 1 million

void dummy(void)
{
	printf(1, "TEST FAILED: this should never execute.\n");
}

void handle_signal(siginfo_t info)
{
	static int counter;

	counter++;

	if(counter >= REPEATS){
	    int* ptr = &(info.signum) + 4;
	    (*ptr) += 4;
	}
}

int main(int argc, char *argv[])
{
	// We could use REPEATS here but we hard-coded this for readability
	printf(1, "Timing of average cost is in microticks (see readme)\n");
	printf(1, "Running handler 1,000,000 times...\n\n");

	int start = uptime();

	// SIGFPE code
	int x = 5;
	int y = 0;
	signal(SIGFPE, handle_signal);
	x = x / y;

	int finish = uptime();
	int duration = finish - start;
	
	printf(1, "Traps Performed: 1,000,000\n");	// see comment @line 27
	printf(1, "Total Elapsed Time: %d\n", duration);
	printf(1, "Average Time Per Trap: %d\n", duration);
	// printf(1, "(a microtick is defined as one one-millionth of a tick)\n");
	// printf(1, "\nAssuming 1 tick ~ 1 ms, trap cost is about 0.%d ms.\n", duration);

	exit();
}