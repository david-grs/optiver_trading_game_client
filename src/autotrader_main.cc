#include "autotrader.h"
#include "tsc_clock.h"

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>

extern "C"
{
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
}

///////////////////////
// NOTE: do NOT edit or move those lines!
TSCTimestamp timestampIn;
std::string Username = "USERNAME";
///////////////////////

int main()
{
	std::signal(SIGINT, [](int /*sig*/) { std::exit(1); });

	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(0, &set);

	if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
	{
		std::cerr << "sched_setaffinity failed, " << strerror(errno) << std::endl;
		return 1;
	}

	TSCClock::Initialise();

	Autotrader autotrader;
	autotrader.Run();

	return 0;
}
