#include <signal.h>

int main()
{
	kill(-1, SIGUSR2);
	return 0;
}
