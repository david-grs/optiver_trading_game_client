#include "autotrader.h"
#include "tsc_clock.h"

#include <csignal>
#include <cstdlib>

int main()
{
	std::signal(SIGINT, [](int /*sig*/) { std::exit(1); });

	TSCClock::Initialise();

	Autotrader autotrader;
	autotrader.Run();

	return 0;
}
